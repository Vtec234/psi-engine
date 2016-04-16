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
#include <set>

#include <boost/optional.hpp>

#include "../util/assert.hpp"


namespace psi_sys {
class SystemManagerScene : public psi_scene::ISceneDirectAccess {
protected:
	boost::any _component(psi_scene::ComponentTypeId t, size_t id) override {
		ASSERT(_scene.count(t));
		auto& store = _scene[t];
		auto const& info = store.info;
		ASSERT(id < (store.stored_n + store.added_n));

		if (id < store.stored_n) {
			return info.to_any_f(&store.data[id * store.info.size]);
		}
		else {
			return info.to_any_f(&store.added[(id - store.stored_n) * store.info.size]);
		}
	}

	void _mark_component_changed(psi_scene::ComponentTypeId t, size_t id) override {
		ASSERT(_scene.count(t));
		auto& store = _scene[t];
		ASSERT(id < (store.stored_n + store.stored_n));

		if (id >= store.stored_n) {
			return;
		}

		if (std::find(store.changed.begin(), store.changed.end(), id) == store.changed.end()) {
			store.changed.push_back(id);
		}
	}

	size_t _add_component(psi_scene::ComponentTypeId t, boost::any comp) override {
		ASSERT(_scene.count(t));
		auto& store = _scene[t];
		auto const& info = store.info;

		try {
			size_t len = info.size;
			char* data = info.to_raw_f(comp);
			store.added.insert(store.added.end(), data, data + len);
			++store.added_n;
		}
		catch (std::exception const& e) {
			ASSERT(false && "invalid component type");
		}

		return store.stored_n + store.added_n - 1;
	}

	size_t _component_count(psi_scene::ComponentTypeId t) override {
		ASSERT(_scene.count(t));
		auto& store = _scene[t];

		return store.stored_n + store.added_n;
	}

	void _mark_component_remove(psi_scene::ComponentTypeId t, size_t id) override {
		ASSERT(_scene.count(t));
		auto& store = _scene[t];
		ASSERT(id < (store.stored_n + store.added_n));

		if(std::find(store.to_remove.begin(), store.to_remove.end(), id) == store.to_remove.end()) {
			store.to_remove.push_back(id);
		}
	}

	void _cancel_component_removal(psi_scene::ComponentTypeId t, size_t id) override {
		ASSERT(_scene.count(t));
		auto& store = _scene[t];
		ASSERT(id < (store.stored_n + store.added_n));

		auto it = std::find(store.to_remove.begin(), store.to_remove.end(), id);
		if (it != store.to_remove.end()) {
			store.to_remove.erase(it);
		}
	}

	bool _component_is_marked_remove(psi_scene::ComponentTypeId t, size_t id) override {
		ASSERT(_scene.count(t));
		auto& store = _scene[t];
		ASSERT(id < (store.stored_n + store.added_n));

		return std::find(store.to_remove.begin(), store.to_remove.end(), id) != store.to_remove.end();
	}

public:
	struct ComponentTypeStorage {
		std::vector<char> data;
		size_t stored_n = 0;

		std::vector<char> added;
		size_t added_n = 0;

		std::vector<size_t> changed;
		std::vector<size_t> to_remove;

		psi_scene::ComponentTypeInfo info;
	};
	// TODO store it like that ^
	// or store same data a in static storage in SystemManager (non-system-accessible)
	// + vector<ChangeEvent/ComponentChange/SceneChange/Whatever>?
	// changes would be be harder to get an overall picture of
	// but maybe easier to sync? have to build vector<Change> most likely anyway for syncing

	std::unordered_map<psi_scene::ComponentTypeId, ComponentTypeStorage> _scene;
};

SystemManager::SystemManager(psi_thread::TaskManager const& tasks)
	: _tasks(tasks) {}

void SystemManager::register_system(std::unique_ptr<ISystem> sys) {
	_systems.push_back(std::move(sys));
}

void SystemManager::register_component_type(psi_scene::ComponentTypeInfo info) {
	ASSERT(!_scene.count(info.type));

	_scene[info.type].info = info;
}

void SystemManager::load_scene(void*) {
	// TODO load actual scene resource/file and init storage

	std::vector<std::pair<uint64_t, std::unique_ptr<psi_scene::ISceneDirectAccess>>> tasks;
	for (auto& sys : _systems) {
		tasks.emplace_back();
		auto index = tasks.size() - 1;

		tasks[index].first = _tasks.submit_task(
			[&, this] {
			tasks[index].second = _construct_access(sys->required_components());
			sys->on_scene_loaded(*tasks[index].second);
		}
		);
	}

	// wait until all systems are done
	std::vector<std::unique_ptr<psi_scene::ISceneDirectAccess>> accesses;
	for (auto& t : tasks) {
		_tasks.wait_for_task(t.first);
		accesses.push_back(std::move(t.second));
	}

	_sync_with_accesses(accesses);
}

void SystemManager::update_scene() {
	std::vector<std::pair<uint64_t, std::unique_ptr<psi_scene::ISceneDirectAccess>>> tasks;
	for (auto& sys : _systems) {
		tasks.emplace_back();
		auto index = tasks.size() - 1;

		tasks[index].first = _tasks.submit_task(
			[&, this] {
			tasks[index].second = _construct_access(sys->required_components());
			sys->on_scene_update(*tasks[index].second);
		}
		);
	}

	// wait until all systems are done
	std::vector<std::unique_ptr<psi_scene::ISceneDirectAccess>> accesses;
	for (auto& t : tasks) {
		_tasks.wait_for_task(t.first);
		accesses.push_back(std::move(t.second));
	}

	_sync_with_accesses(accesses);
}

void SystemManager::save_scene() {
	std::vector<std::pair<uint64_t, std::unique_ptr<psi_scene::ISceneDirectAccess>>> tasks;
	for (auto& sys : _systems) {
		tasks.emplace_back();
		auto index = tasks.size() - 1;

		tasks[index].first = _tasks.submit_task(
			[&, this] {
			tasks[index].second = _construct_access(sys->required_components());
			sys->on_scene_save(*tasks[index].second, nullptr);
		}
		);
	}

	// wait until all systems are done
	std::vector<std::unique_ptr<psi_scene::ISceneDirectAccess>> accesses;
	for (auto& t : tasks) {
		_tasks.wait_for_task(t.first);
		accesses.push_back(std::move(t.second));
	}

	_sync_with_accesses(accesses);
}

void SystemManager::shut_scene(void*) {}

std::unique_ptr<psi_scene::ISceneDirectAccess> SystemManager::_construct_access(psi_scene::ComponentTypeIdBitset types) {
	SystemManagerScene* access = new SystemManagerScene;
	for (auto const& map : _scene) {
		auto const& store = map.second;
		auto const& info = map.second.info;

		if (types & info.type) {
			auto& access_store = access->_scene[info.type];

			// copy type information
			access_store.info = info;
			// copy type data - EXPENSIVE!
			access_store.data= store.data;
			access_store.stored_n = store.stored_n;
		}
	}

	return std::unique_ptr<psi_scene::ISceneDirectAccess>(access);
}

void SystemManager::_sync_with_accesses(std::vector<std::unique_ptr<psi_scene::ISceneDirectAccess>>& accesses) {
	// one sync operation per component type
	for (auto const& type : _scene) {
		// make a list of all storages of accesses which requested the given type
		std::vector<SystemManagerScene::ComponentTypeStorage*> storage;
		for (auto& a : accesses) {
			SystemManagerScene* sc = static_cast<SystemManagerScene*>(a.get());
			if (sc->_scene.count(type.first)) {
				storage.push_back(&sc->_scene[type.first]);
			}
		}

		// aggregate indices of components which are to be removed
		std::set<size_t> to_remove;
		for (auto s : storage) {
			to_remove.insert(s->to_remove.begin(), s->to_remove.end());
		}


		// aggregate indices of components which underwent changes
		std::vector<size_t> changed;
		for (auto s : storage) {
			for (size_t id : s->changed) {
				// discard changes to components which will be removed
				if (std::find(to_remove.cbegin(), to_remove.cend(), id) != to_remove.cend())
					continue;

				if (std::find(changed.cbegin(), changed.cend(), id) != changed.cend()) {
					// TODO resolve conflicts
					// have to keep track of which system did which change to make proper resolutions
					// for now only merge first change
					continue;
				}
				else {
					changed.push_back(id);

					size_t type_size = type.second.info.size;
					char* data = &s->data[id];


					// do different things based on what kind of change happened
					// first handle owning references
					//
					// normal refs need only be handled in removal


					// TODO sync change here
				}
			}
		}



		// syncing additions is easiest
		// just copy data
		// and for types which hold references update the references
		for (auto s : storage) {
			for (size_t i_a = 0; i_a < s->added_n; ++i_a) {
				size_t len = type.second.info.size;
				char* data = &s->added[i_a * len];

				// have to update refs!



			}





		}
	}
}
} // namespace psi_sys
