/*
 * Copyright (C) 2016 Wojciech Nawrocki
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

#include <cstdlib>
#include <cassert>


// TODO:
// - dump stacktrace and additional info to file before aborting
// - to wrap or not to wrap? (in psi_util::)
#ifdef NDEBUG
	#define ASSERT(cond) \
		(cond) \
		? ((void)0) \
		: std::abort()
#else
	#define ASSERT(cond) \
		(cond) \
		? ((void)0) \
		: assert(false)
#endif
