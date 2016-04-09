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

#include <openssl/md5.h>

#include "../../util/file.hpp"
namespace fs = boost::filesystem;
#include "../../log/log.hpp"


psi_rndr::MeshData psi_rndr::load_mesh(fs::path const& file) {
	// might throw, pass exception if it does
	auto data = psi_util::load_binary(file);

	MeshData mesh;

	MD5_CTX md5;
	std::array<unsigned char, 16> md5_digest;

	// read from Psi Mesh format
	// calculate first 16 byte MD4 hash
	char const* ptr = data.data();
	MD5_Init(&md5);
	MD5_Update(&md5, ptr, 40 * sizeof(char));
	MD5_Final(md5_digest.data(), &md5);

	// read first 40 bytes
	size_t verts = *reinterpret_cast<uint64_t const*>(ptr);
	ptr += sizeof(uint64_t) / sizeof(char);
	size_t inds = *reinterpret_cast<uint64_t const*>(ptr);
	ptr += sizeof(uint64_t) / sizeof(char);
	auto box_ptr  = reinterpret_cast<float const*>(ptr);
	mesh.max_pos[0] = *(box_ptr++);
	mesh.max_pos[1] = *(box_ptr++);
	mesh.max_pos[2] = *(box_ptr++);
	mesh.min_pos[0] = *(box_ptr++);
	mesh.min_pos[1] = *(box_ptr++);
	mesh.min_pos[2] = *(box_ptr++);
	ptr = reinterpret_cast<char const*>(box_ptr);

	// check first 16 byte MD5 hash
	for (char const c : md5_digest) {
		if (c != *(ptr++)) {
			throw std::runtime_error("Mesh file MD5 Sum 1 is invalid.");
		}
	}

	// calculate second 16 byte MD5 hash
	MD5_Init(&md5);
	MD5_Update(&md5, ptr, verts * sizeof(psi_rndr::VertexData) + inds * sizeof(uint32_t));
	MD5_Final(md5_digest.data(), &md5);

	// read the rest of the bytes
	mesh.vertices.resize(verts);
	memcpy(mesh.vertices.data(), ptr, verts * sizeof(psi_rndr::VertexData));
	ptr += verts * sizeof(psi_rndr::VertexData);
	mesh.indices.resize(inds);
	memcpy(mesh.indices.data(), ptr, inds * sizeof(uint32_t));
	ptr += inds * sizeof(uint32_t);

	// check second 16 byte MD5 hash
	for (char const c : md5_digest) {
		if (c != *(ptr++)) {
			throw std::runtime_error("Mesh file MD5 Sum 2 is invalid.");
		}
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
