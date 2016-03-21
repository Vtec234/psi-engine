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


namespace psi_mark {
/// Marker struct designating the properties of an interface.
/// All functions in this interface are threadsafe like so:
/// 1. Non-const functions are safe with respect to const functions.
/// 2. Const functions are safe with respect to other const functions.
/// In other words, any number of const functions and exactly one non-const function may
/// be safely called at once. Calling more than one non-const function is not safe.
struct Threadsafe {};

/// Marker struct designating the properties of an interface.
/// Const functions in this interface are threadsafe like so:
/// 1. Const functions are safe with respect to other const functions.
/// In other words, any number of const functions may be safely called at once.
/// Non-const functions may only be called by themselves, that is without any
/// other const or non-const ones running at the same time
struct ConstThreadsafe {};

/// Marker struct designating the properties of an interface.
/// No function in this interface is threadsafe. It is meant
/// to be used by a single thread only.
struct NonThreadsafe {};
} // namespace psi_mark
