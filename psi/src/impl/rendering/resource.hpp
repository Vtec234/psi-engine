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

#include <array>
#include <cstdint>
#include <cfloat>
#include <vector>

#include <boost/filesystem.hpp>


namespace psi_rndr {
/// Describes a single vertex in a mesh.
struct VertexData {
	static constexpr size_t FLOATS_PER_VERTEX = 11;
	/// Position vector
	std::array<float, 3> pos;
	/// Normal vector
	std::array<float, 3> norm;
	/// Tangent vector
	std::array<float, 3> tan;
	/// Texture UV coordinates
	std::array<float, 2> uv;
};

/// Describes a physical mesh.
struct MeshData {
	/// Designates how primitives in a mesh are constructed from its vertices.
	enum class MeshPrimitiveMode {
		TRIANGLES,
		LINES,
	};

	/// Vertices making up the mesh
	std::vector<VertexData> vertices;
	/// Indices of vectors creating primitives
	std::vector<uint32_t> indices;
	MeshPrimitiveMode mode = MeshPrimitiveMode::TRIANGLES;

	/// Bounding Box maximum position
	std::array<float, 3> max_pos = {{FLT_MIN, FLT_MIN, FLT_MIN}};
	/// Bounding Box minimum position
	std::array<float, 3> min_pos = {{FLT_MAX, FLT_MAX, FLT_MAX}};
};

struct TextureData {
	enum class Encoding {
		RGB8,
		RGBA8,
		RGB16,
		RGBA16,
		RGB32,
		RGBA32,
		RGB16F,
	};

	/// Vector of mipmaps, each one containing data in the specified encoding.
	/// First element should be largest mipmap.
	std::vector<std::vector<unsigned char>> data;

	uint32_t width;
	uint32_t height;
	Encoding encoding;
};

/// Tries to load a mesh stored in the Psi Engine Mesh .msh format.
/// @throws if the file does not exist, is invalid, or otherwise occupied
/// @returns the mesh data
MeshData load_mesh(boost::filesystem::path const& file);

/// Tries to load a texture stored in the .png, .jpeg, or .tiff format.
/// Calculates mipmaps.
/// @throws if the file does not exist, is invalid, or otherwise occupied
/// @returns the mesh data
TextureData load_texture(boost::filesystem::path const& file);
} // namespace psi_util
