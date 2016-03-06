/*
 * Copyright (C) 2015-2016 Wojciech Nawrocki
 *
 * This file is part of the Sol Library.
 *
 * The Sol Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Sol Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Sol Library. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <cstdint>

#include "scene.hpp"


namespace sol {
class ISystem {
public:
	/// Bitmask of SceneComponentType. Only the required types will be provided to the system.
    virtual uint64_t required_components() const = 0;

	/// Functions called at various moments of a scene's lifetime.
    virtual void on_scene_loaded(ISceneDirectAccess) = 0;
    virtual void on_scene_update(ISceneDirectAccess) = 0;
    virtual void on_scene_save(ISceneDirectAccess, void* replace_with_save_file) = 0;
    virtual void on_scene_shutdown(ISceneDirectAccess) = 0;
};
} // namespace sol
