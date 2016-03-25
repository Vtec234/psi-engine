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

#include "file.hpp"

#include <fstream>

namespace fs = boost::filesystem;


std::vector<std::string> psi_util::load_text(fs::path const& file) {
	// try to open input stream
	std::ifstream in_file(file.c_str());
	if (!in_file.good()) {
		in_file.close();
		throw std::runtime_error("Failed to open text file " + file.string() + ".");
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

std::vector<char> psi_util::load_binary(fs::path const& file) {
	// might throw, pass exception if it does
	auto size = fs::file_size(file);

	// allocate memory of file size
	std::vector<char> bin(size);

	// try to open binary input stream
	std::ifstream in_file(file.c_str(), std::ios::in | std::ios::binary);
	if (!in_file.good()) {
		in_file.close();
		throw std::runtime_error("Failed to open binary file " + file.string() + ".");
	}

	// copy whole file into vector
	in_file.read(bin.data(), size);

	// close file and return contents
	in_file.close();
	return bin;
}

psi_util::MeshData psi_util::load_mesh(boost::filesystem::path const& file) {
	// might throw, pass exception if it does
	auto data = load_binary(file);

	MeshData mesh;

	// format:
	// 8 bytes - vertex count
	// 8 bytes- index count
	// 24 bytes - bounding box
	// 1 byte null char
	// vertices (count * sizeof(VertexData))
	// 1 byte null char
	// indices (count * 4 bytes)
	// 1 byte null char
	char const* ptr = data.data();
	size_t verts = *reinterpret_cast<uint64_t const*>(ptr);
	ptr += sizeof(uint64_t) / sizeof(char);
	size_t inds = *reinterpret_cast<uint64_t const*>(ptr);
	ptr += sizeof(uint64_t) / sizeof(char);
	auto bbptr = reinterpret_cast<float const*>(ptr);
	mesh.max_pos[0] = *(bbptr++);
	mesh.max_pos[1] = *(bbptr++);
	mesh.max_pos[2] = *(bbptr++);
	mesh.min_pos[0] = *(bbptr++);
	mesh.min_pos[1] = *(bbptr++);
	mesh.min_pos[2] = *(bbptr++);
	ptr = reinterpret_cast<char const*>(bbptr);
	if (*ptr != '\0') {
		throw std::runtime_error("Invalid data in mesh file " + file.string() + ".");
	}
	ptr += sizeof(char);
	mesh.vertices.resize(verts);
	memcpy(mesh.vertices.data(), ptr, verts * sizeof(psi_util::VertexData));
	ptr += verts * sizeof(psi_util::VertexData);
	if (*ptr != '\0') {
		throw std::runtime_error("Invalid data in mesh file " + file.string() + ".");
	}
	ptr += sizeof(char);
	mesh.indices.resize(inds);
	memcpy(mesh.indices.data(), ptr, inds * sizeof(uint32_t));
	ptr += inds * sizeof(uint32_t);
	if (*ptr != '\0') {
		throw std::runtime_error("Invalid data in mesh file " + file.string() + ".");
	}

	return mesh;
}

// TODO this and store a single shader in one file instead of .vert, .frag, etc
psi_util::GLSLSource psi_util::load_glsl(boost::filesystem::path const& file) {
	std::array<std::vector<std::string>, 6> sources;
	sources[0] = load_text(file.string() + ".vert");
	sources[1] = load_text(file.string() + ".frag");

	return GLSLSource::construct_from_sources(sources);
}
