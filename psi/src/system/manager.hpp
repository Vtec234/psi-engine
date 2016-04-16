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

#pragma once

#include <vector>
#include <unordered_map>
#include <cstdint>

#include "system.hpp"
#include "../thread/manager.hpp"
#include "../scene/components.hpp"
#include "../marker/thread_safety.hpp"


namespace psi_sys {
class SystemManager : psi_mark::NonThreadsafe {
public:
	explicit SystemManager(psi_thread::TaskManager const&);

	void register_system(std::unique_ptr<ISystem>);

	void register_component_type(psi_scene::ComponentTypeInfo);

	void load_scene(void*);

	void update_scene();

	void save_scene();

	void shut_scene(void*);

private:
	psi_thread::TaskManager const& _tasks;

	struct ComponentTypeStorage {
		std::vector<char> data;
		size_t stored_n = 0;
		psi_scene::ComponentTypeInfo info;
	};

	std::unordered_map<psi_scene::ComponentTypeId, ComponentTypeStorage> _scene;

	std::vector<std::unique_ptr<ISystem>> _systems;

	std::unique_ptr<psi_scene::ISceneDirectAccess> _construct_access(psi_scene::ComponentTypeIdBitset);
	void _sync_with_accesses(std::vector<std::unique_ptr<psi_scene::ISceneDirectAccess>>&);
};
} // namespace psi_sys
