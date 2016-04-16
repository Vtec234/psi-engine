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
#include "../scene/components.hpp"

#include "../marker/thread_safety.hpp"


namespace psi_sys {
class ISystem : psi_mark::NonThreadsafe {
public:
	/// Bitmask of SceneComponentType. Only the required types will be provided to the system.
	virtual psi_scene::ComponentTypeIdBitset required_components() const = 0;

	/// Functions called at various moments of a scene's lifetime.
	virtual void on_scene_loaded(psi_scene::ISceneDirectAccess&) = 0;
	virtual void on_scene_update(psi_scene::ISceneDirectAccess&) = 0;
	virtual void on_scene_save(psi_scene::ISceneDirectAccess&, void* replace_with_save_file) = 0;
	virtual void on_scene_shutdown(psi_scene::ISceneDirectAccess&) = 0;
};
} // namespace psi_sys
