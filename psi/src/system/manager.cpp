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
private:
	template <typename T>
	struct ComponentTypeStorage {
		typedef T comp_type;

		/// raw data of components already present
		std::vector<T> data;
		/// indices of changed components
		/// added component ids do not have to be here to be synced
		std::vector<size_t> changed;
		/// raw data of added components
		std::vector<T> added;
		/// indices of components marked for removal
		std::vector<size_t> to_remove;
	};

	boost::optional<ComponentTypeStorage<psi_scene::SceneComponentEntity>> m_ents;
	boost::optional<ComponentTypeStorage<psi_scene::SceneComponentTransform>> m_transfs;
	boost::optional<ComponentTypeStorage<psi_scene::SceneComponentModel>> m_models;

	template <typename T>
	boost::any const read_comp_intrnl(T const& vec, size_t id) {
		ASSERT(vec && id < (vec->data.size() + vec->added.size()));

		if (id < vec->data.size()) {
			return &vec->data[id];
		}
		else
			return &vec->added[id - vec->data.size()];
	}

	template <typename T>
	boost::any write_comp_intrnl(T& vec, size_t id) {
		ASSERT(vec && id < (vec->data.size() + vec->added.size()));

		if (id < vec->data.size()) {
			// mark as changed
			if (std::find(vec->changed.begin(), vec->changed.end(), id) == vec->changed.end())
				vec->changed.push_back(id);

			return &vec->data[id];
		}
		else {
			// no need to mark as changed, since added components sync anyway
			return &vec->added[id - vec->data.size()];
		}
	}

	template <typename T, typename V = typename T::value_type::comp_type>
	size_t add_comp_intrnl(T& vec, boost::any comp) {
		ASSERT(vec);

		try {
			vec->added.push_back(boost::any_cast<V>(std::move(comp)));
		}
		catch(std::exception const& e) {
			ASSERT(false && "Invalid cast of component type");
		}

		return vec->data.size() + vec->added.size();
	}

	template <typename T>
	size_t count_intrnl(T const& vec) {
		ASSERT(vec);

		return vec->data.size() + vec->added.size();
	}

	template <typename T>
	void mark_rmv_intrnl(T& vec, size_t id) {
		ASSERT(vec && id < (vec->data.size() + vec->added.size()));

		if (std::find(vec->to_remove.begin(), vec->to_remove.end(), id) == vec->to_remove.end())
			vec->to_remove.push_back(id);
	}

	template <typename T>
	void cncl_rmv_intrnl(T& vec, size_t id) {
		ASSERT(vec && id < (vec->data.size() + vec->added.size()));

		auto it = std::find(vec->to_remove.begin(), vec->to_remove.end(), id);
		if (it != vec->to_remove.end())
			vec->to_remove.erase(it);
	}

	template <typename T>
	bool mrkd_rmv_intrnl(T const& vec, size_t id) {
		ASSERT(vec && id < (vec->data.size() + vec->added.size()));

		return std::find(vec->to_remove.begin(), vec->to_remove.end(), id) != vec->to_remove.end();
	}

public:
	boost::any const read_component(psi_scene::SceneComponentType t, size_t id) override {
		switch(t) {
			case psi_scene::ENTITY: return read_comp_intrnl(m_ents, id);
			case psi_scene::TRANSFORM: return read_comp_intrnl(m_transfs, id);
			case psi_scene::MODEL: return read_comp_intrnl(m_models, id);
		}
	}

	boost::any write_component(psi_scene::SceneComponentType t, size_t id) override {
		switch(t) {
			case psi_scene::ENTITY: return write_comp_intrnl(m_ents, id);
			case psi_scene::TRANSFORM: return write_comp_intrnl(m_transfs, id);
			case psi_scene::MODEL: return write_comp_intrnl(m_models, id);
		}
	}

	size_t add_component(psi_scene::SceneComponentType t, boost::any comp) override {
		switch(t) {
			case psi_scene::ENTITY: return add_comp_intrnl(m_ents, std::move(comp));
			case psi_scene::TRANSFORM: return add_comp_intrnl(m_transfs, std::move(comp));
			case psi_scene::MODEL: return add_comp_intrnl(m_models, std::move(comp));
		}
	}

	size_t component_count(psi_scene::SceneComponentType t) override {
		switch(t) {
			case psi_scene::ENTITY: return count_intrnl(m_ents);
			case psi_scene::TRANSFORM: return count_intrnl(m_transfs);
			case psi_scene::MODEL: return count_intrnl(m_models);
		}
	}

	void mark_component_remove(psi_scene::SceneComponentType t, size_t id) override {
		switch(t) {
			case psi_scene::ENTITY: return mark_rmv_intrnl(m_ents, id);
			case psi_scene::TRANSFORM: return mark_rmv_intrnl(m_transfs, id);
			case psi_scene::MODEL: return mark_rmv_intrnl(m_models, id);
		}
	}

	void cancel_component_removal(psi_scene::SceneComponentType t, size_t id) override {
		switch(t) {
			case psi_scene::ENTITY: return cncl_rmv_intrnl(m_ents, id);
			case psi_scene::TRANSFORM: return cncl_rmv_intrnl(m_transfs, id);
			case psi_scene::MODEL: return cncl_rmv_intrnl(m_models, id);
		}
	}

	bool component_is_marked_remove(psi_scene::SceneComponentType t, size_t id) override {
		switch(t) {
			case psi_scene::ENTITY: return mrkd_rmv_intrnl(m_ents, id);
			case psi_scene::TRANSFORM: return mrkd_rmv_intrnl(m_transfs, id);
			case psi_scene::MODEL: return mrkd_rmv_intrnl(m_models, id);
		}
	}
};

void SystemManager::register_system(std::unique_ptr<ISystem> sys) {
	m_systems.push_back({std::move(sys), sys->required_components(), 0});
}

void SystemManager::load_scene(void*) {
	// dispatch system starters
	// init world
	// dispatch system scene initializers

	// TEST
	m_scene_transforms.push_back({psi_scene::NO_COMPONENT, {{0.0f, 5.0f, 0.0f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 0.0f, 0.0f, 0.0f}}});
	m_scene_models.push_back({{U"mesh/default"}, {U"mat/default"}, {U"shader/default"}});
	m_scene_entities.push_back({0,0,false});
}

void SystemManager::update_scene() {
	// dispatch system scene updates
	// sync updates
	// repeat


	// submit ISceneDirectAccess implementation to each system
	// copy data, manage data, etc, etc

}

void SystemManager::save_scene() {
	// dispatch system scene savers
	// close
	// repeat


}

void SystemManager::shut_scene(void*) {




}
} // namespace psi_sys
