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

#include <unordered_map>

#include <boost/optional.hpp>

#include "gl.hpp"
#include "../resource.hpp"
#include "../../../util/enum.hpp"
#include "../../../util/assert.hpp"


namespace psi_gl {
class MeshBuffer {
public:
	explicit MeshBuffer(psi_rndr::MeshData const&);
	~MeshBuffer();

	void draw(GLenum primitives);

private:
	GLuint _vbo;
	GLuint _ebo;
	GLuint _vao;

	uint32_t _index_count;
};

struct FramebufferRenderTargetCreationInfo {
	// true for texture
	// false for renderbuffer
	bool texture_or_not_renderbuffer;
	// true for color
	// false for depth
	bool color_or_not_depth;
	GLenum internal_format;
};

class MultipleRenderTargetFramebuffer {
public:
	MultipleRenderTargetFramebuffer(std::vector<FramebufferRenderTargetCreationInfo>, uint32_t width, uint32_t height);
	~MultipleRenderTargetFramebuffer();

	MultipleRenderTargetFramebuffer(MultipleRenderTargetFramebuffer const&) = delete;
	MultipleRenderTargetFramebuffer& operator=(MultipleRenderTargetFramebuffer const&) = delete;

	MultipleRenderTargetFramebuffer(MultipleRenderTargetFramebuffer&&) = delete;
	MultipleRenderTargetFramebuffer& operator=(MultipleRenderTargetFramebuffer&&) = delete;

	void bind();
	void unbind();
	GLuint texture_target_handle(size_t index);

private:
	GLuint              _framebuffer;
	std::vector<GLuint> _textures;
	std::vector<GLuint> _renderbuffers;
};

struct SamplerSettings {
	GLint mag_filter;
	GLint min_filter;
	GLint wrap_s;
	GLint wrap_t;

	GLfloat max_aniso;
};

void create_sampler_at_texture_unit(SamplerSettings, GLuint);

// Vertex attributes passed into every mesh shader.
enum class ShaderVertexAttrib {
	POS = 0,
	NORM = 1,
	TAN = 2,
	UV = 3,
};

/// A resource that a uniform in a shader may be attached to.
/// The value of this resource will be sent to the shader at runtime.
HASHABLE_ENUM_CLASS_IN_NAMESPACE(UniformMapping, uint32_t, psi_gl) {
	LOCAL_TO_WORLD,
	WORLD_TO_CAMERA,
	LOCAL_TO_CLIP,
	WORLD_TO_CLIP,
	CAMERA_TO_CLIP,
	ALBEDO_TEXTURE_SAMPLER,
	NORMAL_TEXTURE_SAMPLER,
	REFLECTIVENESS_ROUGHNESS_TEXTURE_SAMPLER,
	CUBEMAP_TEXTURE_SAMPLER,
	POSITION_FRAME_TEXTURE_SAMPLER,
	NORMAL_FRAME_TEXTURE_SAMPLER,
	ALBEDO_FRAME_TEXTURE_SAMPLER,
	REFL_ROUGH_FRAME_TEXTURE_SAMPLER,
	CAMERA_POSITION_WORLD,
	ACTIVE_POINT_LIGHTS,
	ACTIVE_SPOT_LIGHTS,
	MESH_MIN_POS,
	MESH_MAX_POS,
};

/// A memory buffer that a uniform block in a shader may be attached to.
/// The memory will be sent to the shader at runtime.
HASHABLE_ENUM_CLASS_IN_NAMESPACE(UniformBlockMapping, uint32_t, psi_gl) {
	LIGHT_DATA,
};

// -- SHADERS --
struct GLSLSource {
	std::string vertex;
	std::string fragment;
	boost::optional<std::string> geometry;
	boost::optional<std::string> tess_ctrl;
	boost::optional<std::string> tess_eval;
	boost::optional<std::string> compute;

	std::unordered_multimap<UniformMapping, std::string> unifs;
	std::unordered_multimap<UniformBlockMapping, std::string> unif_blocks;
};

/// An enum representing where each type of eGLSL source should be in the array passed to construct_from_sources.
enum class SourceTypeInArray : size_t {
	VERTEX,
	FRAGMENT,
	GEOMETRY,
	TESSELATION_CONTROL,
	TESSELATION_EVALUATION,
	COMPUTE,
};

/// Parses the given eGLSL (enhancedGLSL) sources to create a GLSLSource object.
/// @in sources:
///		[0] - vertex
/// 	[1] - fragment
///		[2] - geometry
///		[3] - tesselation control
///		[4] - tesselation evaluation
///		[5] - compute
/// @throw if anything is wrong with the sources
/// @return a parsed version of the source in a GLSLSource instance
GLSLSource parse_glsl_source(std::array<std::vector<std::string>, 6> const& sources);

struct Shader {
	GLuint handle;
	std::unordered_map<UniformMapping, GLuint> unifs;
	std::unordered_map<UniformBlockMapping, GLuint> unif_blocks;
};

Shader compile_glsl_source(GLSLSource const&);

// -- TEXTURES --
inline GLenum tex_internal_format(psi_rndr::TextureData::Encoding enc) {
	switch (enc) {
		case psi_rndr::TextureData::Encoding::RGB8:
			return gl::RGB8;

		case psi_rndr::TextureData::Encoding::RGBA8:
			return gl::RGBA8;

		case psi_rndr::TextureData::Encoding::RGB16:
			return gl::RGB16;

		case psi_rndr::TextureData::Encoding::RGBA16:
			return gl::RGBA16;

		case psi_rndr::TextureData::Encoding::RGB32:
			return gl::RGB32I;

		case psi_rndr::TextureData::Encoding::RGBA32:
			return gl::RGBA32I;

		case psi_rndr::TextureData::Encoding::RGB16F:
			return gl::RGB16F;

		default:
			ASSERT(false);
	}
}

inline GLenum tex_format(psi_rndr::TextureData::Encoding enc) {
	switch (enc) {
		case psi_rndr::TextureData::Encoding::RGB8:
		case psi_rndr::TextureData::Encoding::RGB16:
		case psi_rndr::TextureData::Encoding::RGB32:
		case psi_rndr::TextureData::Encoding::RGB16F:
			return gl::RGB;

		case psi_rndr::TextureData::Encoding::RGBA8:
		case psi_rndr::TextureData::Encoding::RGBA16:
		case psi_rndr::TextureData::Encoding::RGBA32:
			return gl::RGBA;

		default:
			ASSERT(false);
	}
}

inline GLenum tex_type(psi_rndr::TextureData::Encoding enc) {
	switch (enc) {
		case psi_rndr::TextureData::Encoding::RGB8:
		case psi_rndr::TextureData::Encoding::RGBA8:
			return gl::UNSIGNED_BYTE;

		case psi_rndr::TextureData::Encoding::RGB16:
		case psi_rndr::TextureData::Encoding::RGBA16:
			return gl::UNSIGNED_SHORT;

		case psi_rndr::TextureData::Encoding::RGB32:
		case psi_rndr::TextureData::Encoding::RGBA32:
			return gl::UNSIGNED_INT;

		case psi_rndr::TextureData::Encoding::RGB16F:
			return gl::FLOAT;

		default:
			ASSERT(false);
	}
}

inline GLuint upload_tex(psi_rndr::TextureData const& tex) {
	GLuint tex_handle;

	gl::GenTextures(1, &tex_handle);
	gl::BindTexture(gl::TEXTURE_2D, tex_handle);

	auto width = tex.width;
	auto height = tex.height;
	for (size_t i_lvl = 0; i_lvl <	tex.data.size(); ++i_lvl) {
		gl::TexImage2D(
			gl::TEXTURE_2D,
			i_lvl,
			tex_internal_format(tex.encoding),
			width,
			height,
			0,
			tex_format(tex.encoding),
			tex_type(tex.encoding),
			tex.data[i_lvl].data()
		);

		width /= 2;
		height /= 2;
	}

	return tex_handle;
}
} // namespace psi_gl
