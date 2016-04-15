/*
 * Copyright (C) 2015-2016 Wojciech Nawrocki
 *
 * This file is part of Psi Engine.
 *
 * Psi Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Psi Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Psi Engine. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "manager.hpp"

#include <unordered_map>
#include <vector>

#include <boost/optional.hpp>

#include "../util/assert.hpp"


namespace psi_sys {
class SystemManagerScene : public psi_scene::ISceneDirectAccess {
friend class SystemManager;
private:
	struct ComponentStorage {
		/// raw data of components already present
		std::vector<char> data;
		/// amount of stored components
		size_t stored_n = 0;
		/// indices of changed components
		/// added component ids do not have to be here to be synced
		std::vector<size_t> changed;
		/// raw data of added components
		std::vector<char> added;
		/// amount of added components
		size_t added_n = 0;
		/// indices of components marked for removal
		std::vector<size_t> to_remove;
	};

	// TODO store it like that ^
	// or store same data a in static storage in SystemManager (non-system-accessible)
	// + vector<ChangeEvent/ComponentChange/SceneChange/Whatever>?
	// changes would be be harder to get an overall picture of
	// but maybe easier to sync? have to build vector<Change> most likely anyway for syncing

	std::unordered_map<psi_scene::ComponentType, ComponentStorage> _scene;
	std::unordered_map<psi_scene::ComponentType, psi_scene::ComponentTypeInfo> _types;

public:
	boost::any const read_component(psi_scene::ComponentType t, size_t id) override {
		ASSERT(_scene.count(t));
		auto const& store = _scene[t];
		auto const& info = _types[t];
		ASSERT(id < (store.stored_n + store.added_n));

		if (id < store.stored_n) {
			return info.to_const_any_f(&store.data[id * info.size]);
		}
		else {
			return info.to_const_any_f(&store.added[(id - store.stored_n) * info.size]);
		}
	}

	boost::any write_component(psi_scene::ComponentType t, size_t id) override {
		ASSERT(_scene.count(t));
		auto& store = _scene[t];
		auto const& info = _types[t];
		ASSERT(id < (store.stored_n + store.added_n));

		if (id < store.stored_n) {
			// mark as changed
			if (std::find(store.changed.begin(), store.changed.end(), id) == store.changed.end())
				store.changed.push_back(id);

			return info.to_any_f(&store.data[id * info.size]);
		}
		else {
			// no need to mark as changed, since added components sync anyway
			return info.to_any_f(&store.added[(id - store.stored_n) * info.size]);
		}
	}

	size_t add_component(psi_scene::ComponentType t, boost::any comp) override {
		ASSERT(_scene.count(t));
		auto& store = _scene[t];

		try {
			auto p = _types[t].to_data_f(&comp);
			// TODO is this right?
			store.added.insert(store.added.end(), p, p + _types[t].size);
			++store.added_n;
		}
		catch (std::exception const& e) {
			ASSERT(false && "invalid component type");
		}

		return store.stored_n + store.added_n - 1;
	}

	size_t component_count(psi_scene::ComponentType t) override {
		ASSERT(_scene.count(t));
		auto& store = _scene[t];

		return store.stored_n + store.added_n;
	}

	void mark_component_remove(psi_scene::ComponentType t, size_t id) override {
		ASSERT(_scene.count(t));
		auto& store = _scene[t];
		ASSERT(id < (store.stored_n + store.added_n));

		if(std::find(store.to_remove.begin(), store.to_remove.end(), id) == store.to_remove.end())
			store.to_remove.push_back(id);
	}

	void cancel_component_removal(psi_scene::ComponentType t, size_t id) override {
		ASSERT(_scene.count(t));
		auto& store = _scene[t];
		ASSERT(id < (store.stored_n + store.added_n));

		auto it = std::find(store.to_remove.begin(), store.to_remove.end(), id);
		if (it != store.to_remove.end())
			store.to_remove.erase(it);
	}

	bool component_is_marked_remove(psi_scene::ComponentType t, size_t id) override {
		ASSERT(_scene.count(t));
		auto& store = _scene[t];
		ASSERT(id < (store.stored_n + store.added_n));

		return std::find(store.to_remove.begin(), store.to_remove.end(), id) != store.to_remove.end();
	}
};

SystemManager::SystemManager(psi_thread::TaskManager const& tasks)
	: _tasks(tasks) {}

void SystemManager::register_system(std::unique_ptr<ISystem> sys) {
	auto req = sys->required_components();
	_systems.push_back({std::move(sys), req});
}

void SystemManager::register_component_type(psi_scene::ComponentTypeInfo info) {
	ASSERT(!bool(_scene.count(info.id)) && !bool(_types.count(info.id)));

	_types[info.id] = info;
	_scene[info.id];
}

void SystemManager::load_scene(void*) {
	// TODO load actual scene resource/file and init storage

	std::vector<uint64_t> tasks;
	for (auto& sys : _systems) {
		SystemManagerScene sc;
		auto id = _tasks.submit_task([&, this]{
			for (auto& info : _types) {
				// component type is required by the system
				if (sys.required_components & info.first) {
					// copy type info
					sc._types[info.first] = info.second;
					// copy type data - long operation!
					auto& comp = sc._scene[info.first];
					comp.data = _scene[info.first].data;
					comp.stored_n = _scene[info.first].stored_n;
				}
			}

			sys.sys->on_scene_loaded(sc);
		});
		tasks.push_back(id);
	}
	// wait until all systems are done
	for (auto task : tasks) {
		_tasks.wait_for_task(task);
	}

	// TODO SYNC CHANGES
}

void SystemManager::update_scene() {
	std::vector<uint64_t> tasks;
	for (auto& sys : _systems) {
		SystemManagerScene sc;
		auto id = _tasks.submit_task([&, this]{
			for (auto& info : _types) {
				// component type is required by the system
				if (sys.required_components & info.first) {
					// copy type info
					sc._types[info.first] = info.second;
					// copy type data - long operation!
					auto& comp = sc._scene[info.first];
					comp.data = _scene[info.first].data;
					comp.stored_n = _scene[info.first].stored_n;
				}
			}

			sys.sys->on_scene_update(sc);
		});
		tasks.push_back(id);
	}
	// wait until all systems are done
	for (auto task : tasks) {
		_tasks.wait_for_task(task);
	}

	// TODO SYNC CHANGES
}

void SystemManager::save_scene() {
	std::vector<uint64_t> tasks;
	for (auto& sys : _systems) {
		SystemManagerScene sc;
		auto id = _tasks.submit_task([&, this]{
			for (auto& info : _types) {
				// component type is required by the system
				if (sys.required_components & info.first) {
					// copy type info
					sc._types[info.first] = info.second;
					// copy type data - long operation!
					auto& comp = sc._scene[info.first];
					comp.data = _scene[info.first].data;
					comp.stored_n = _scene[info.first].stored_n;
				}
			}

			sys.sys->on_scene_loaded(sc);
		});
		tasks.push_back(id);
	}
	// wait until all systems are done
	for (auto task : tasks) {
		_tasks.wait_for_task(task);
	}

	// TODO SYNC CHANGES
}

void SystemManager::shut_scene(void*) {}
} // namespace psi_sys
