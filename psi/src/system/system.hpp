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

#include <cstdint>
#include <memory>

#include "../scene/access.hpp"


namespace psi_sys {
class ISystem {
public:
	/// Bitmask of SceneComponentType. Only the required types will be provided to the system.
	virtual uint64_t required_components() const = 0;

	/// Functions called at various moments of a scene's lifetime.
	virtual void on_scene_loaded(std::unique_ptr<psi_scene::ISceneDirectAccess>) = 0;
	virtual void on_scene_update(std::unique_ptr<psi_scene::ISceneDirectAccess>) = 0;
	virtual void on_scene_save(std::unique_ptr<psi_scene::ISceneDirectAccess>, void* replace_with_save_file) = 0;
	virtual void on_scene_shutdown(std::unique_ptr<psi_scene::ISceneDirectAccess>) = 0;
};
} // namespace psi_sys
