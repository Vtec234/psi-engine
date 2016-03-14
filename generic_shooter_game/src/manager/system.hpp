/*
 * Copyright (C) 2015-2016 Wojciech Nawrocki
 *
 * This file is part of Generic Shooter Game.
 *
 * Generic Shooter Game is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Generic Shooter Game is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Generic Shooter Game. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <vector>
#include <cstdint>

#include <scene/system.hpp>
#include <scene/scene.hpp>


namespace gsg {
class SystemManager {
public:
	void register_system(std::unique_ptr<sol::ISystem>);

	void load_scene(void*);

	void update_scene();

	void save_scene();

	void shut_scene(void*);

	
private:
	struct SystemStorage {
		std::unique_ptr<sol::ISystem> sys;
		uint64_t required_components;
		size_t memory_requirements;
	};

	std::vector<SystemStorage> m_systems;

	std::vector<sol::SceneComponentEntity> m_scene_entities;
	std::vector<sol::SceneComponentTransform> m_scene_transforms;
	std::vector<sol::SceneComponentModel> m_scene_models;
};
} // namespace gsg
