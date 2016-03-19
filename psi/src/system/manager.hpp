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

#include "../thread/task.hpp"
#include "system.hpp"

#include "../scene/components.hpp"


namespace psi_sys {
class SystemManager {
public:
	explicit SystemManager(psi_thread::ITaskSubmitter*);

	void register_system(std::unique_ptr<ISystem>);

	void register_component_type(psi_scene::ComponentTypeInfo);

	void load_scene(void*);

	void update_scene();

	void save_scene();

	void shut_scene(void*);

private:
	struct SystemStorage {
		std::unique_ptr<ISystem> sys;
		psi_scene::ComponentTypeBitset required_components;
	};

	struct ComponentStorage {
		std::vector<char> data;
		size_t stored_n = 0;
	};

	std::vector<SystemStorage> m_systems;

	std::unordered_map<psi_scene::ComponentType, ComponentStorage> m_scene;
	std::unordered_map<psi_scene::ComponentType, psi_scene::ComponentTypeInfo> m_types;

	psi_thread::ITaskSubmitter* m_tasks;
};
} // namespace psi_sys
