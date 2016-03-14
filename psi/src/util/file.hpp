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

#include <string>
#include <vector>


namespace psi_util {
	/// Tries to load a text file from the specified path.
	/// @throw if the file does not exist, is invalid, or otherwise occupied
	/// @return a vector of lines from this file
	std::vector<std::string> load_text(std::string const& file);

	/// Tries to load a binary file from the specified path.
	/// @throw if the file does not exist, is invalid, or otherwise occupied
	/// @return a vector containing the data from the file
	std::vector<uint8_t> load_binary(std::string const& file);
} // namespace psi_util
