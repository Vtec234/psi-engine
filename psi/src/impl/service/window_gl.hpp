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

#include "../../service/window.hpp"


namespace psi_serv {
/// The arguments required to start an OpenGL window instance.
struct GLWindowServiceArgs {
	uint16_t width;
	uint16_t height;

	uint8_t samples;

	char const* title;

	bool resizable;
	bool always_on_top;

	bool debug;
};

/// Starts the service and spawns a window.
/// Might throw an exception or two.
/// Shouldn't probably be called more than once - might bork.
std::unique_ptr<IWindowService> start_gl_window_service(GLWindowServiceArgs);
} // namespace psi_serv
