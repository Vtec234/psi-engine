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


namespace psi_util {
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

/// Designates how primitives in a mesh are constructed from its vertices.
enum class MeshPrimitiveMode {
	TRIANGLES,
	LINES,
};

/// Describes a physical mesh.
struct MeshData {
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
} // namespace psi_util
