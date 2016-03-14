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

#include "../../scene/scene.hpp"
#include "../system.hpp"


namespace psi_sys {
class SystemGLRenderer : public ISystem {
public:
	uint64_t required_components() const override;

	void on_scene_loaded(std::unique_ptr<psi_scene::ISceneDirectAccess>) override;
	void on_scene_update(std::unique_ptr<psi_scene::ISceneDirectAccess>) override;
	void on_scene_save(std::unique_ptr<psi_scene::ISceneDirectAccess>, void* replace_with_save_file) override;
	void on_scene_shutdown(std::unique_ptr<psi_scene::ISceneDirectAccess>) override;
};
} // namespace psi_sys
