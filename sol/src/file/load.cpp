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

#include "load.hpp"

#include <fstream>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


std::vector<std::string> sol::file::load_text(std::string const& file) {
	// try to open input stream
	std::ifstream in_file(file);
	if (!in_file.good()) {
		in_file.close();
		throw std::runtime_error("Failed to open text file " + file + ".");
	}

	// read line by line
	std::string line;
	std::vector<std::string> contents;
	while (in_file.good()) {
		getline(in_file, line);
		contents.push_back(line + "\n");
	}

	// close file and return contents
	in_file.close();
	return contents;
}

std::vector<uint8_t> sol::file::load_binary(std::string const& file) {
	// might throw, pass exception if it does
	auto size = fs::file_size(file);

	// allocate memory of file size
	std::vector<uint8_t> bin(size);

	// try to open binary input stream
	std::ifstream in_file(file, std::ios::in | std::ios::binary);
	if (!in_file.good()) {
		in_file.close();
		throw std::runtime_error("Failed to open binary file " + file + ".");
	}

	// copy whole file into vector
	in_file.read(reinterpret_cast<char*>(bin.data()), size);

	// close file and return contents
	in_file.close();
	return bin;
}
