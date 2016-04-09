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

#include "resource.hpp"

#include <FreeImage.h>

#include "../../util/file.hpp"
namespace fs = boost::filesystem;
#include "../../log/log.hpp"


psi_rndr::MeshData psi_rndr::load_mesh(fs::path const& file) {
	// might throw, pass exception if it does
	auto data = psi_util::load_binary(file);

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
	memcpy(mesh.vertices.data(), ptr, verts * sizeof(VertexData));
	ptr += verts * sizeof(VertexData);
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

psi_rndr::TextureData psi_rndr::load_texture(fs::path const& file) {
	// get image information
	FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(file.c_str());
	if (format == FIF_UNKNOWN)
		throw std::runtime_error("Invalid image file format.");

	// load bitmap
	auto bitmap = FreeImage_Load(format, file.c_str());
	if (!bitmap)
		throw std::runtime_error("Invalid or nonexistent image file.");

	// convert to RGBA8
	auto bitmap32 = FreeImage_ConvertTo32Bits(bitmap);
	FreeImage_Unload(bitmap);

	// get size information
	auto width = FreeImage_GetWidth(bitmap32);
	auto height = FreeImage_GetHeight(bitmap32);
	if (width == 0 || width & (width - 1) || height == 0 || height & (height - 1))
		throw std::runtime_error("Image dimensions not powers of two.");

	// create object to store bitmap in
	TextureData tex;
	tex.width = width;
	tex.height = height;
	tex.encoding = TextureData::Encoding::RGB8;

	// generate mipmaps
	bool lowest_level_passed = false;
	size_t level = 0;
	while (!lowest_level_passed) {
		auto prev = bitmap32;
		bitmap32 = FreeImage_Rescale(bitmap32, width, height, FILTER_CATMULLROM);
		//FreeImage_Unload(prev);

		if (width == 1 || height == 1) {
			lowest_level_passed = true;
		}

		unsigned char* bytes = FreeImage_GetBits(bitmap32);
		size_t len = height * FreeImage_GetLine(bitmap32);
		tex.data.emplace_back();
		tex.data[level].assign(bytes, bytes + len);

		width /= 2;
		height /= 2;
		level++;
	}

	return tex;
}
