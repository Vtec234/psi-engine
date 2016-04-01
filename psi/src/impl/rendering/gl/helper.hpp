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


namespace psi_gl {
using Object = GLuint;

class MeshBuffer {
public:
	explicit MeshBuffer(psi_rndr::MeshData const&);
	~MeshBuffer();

	void draw(GLenum primitives);

private:
	Object m_VBO;
	Object m_EBO;
	Object m_VAO;

	uint32_t m_index_count;
};

enum class TextureUnit {
	DIFFUSE = 1,
	SPECULAR = 2,
	NORMAL = 3,
	CUBE = 4,
	GUI = 5,
	TEXT = 6,
	GLOSS = 7,
};

struct SamplerSettings {
	GLint mag_filter;
	GLint min_filter;
	GLint wrap_S;
	GLint wrap_T;
	GLfloat max_aniso;
};

void create_sampler_at_texture_unit(SamplerSettings, TextureUnit);

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
	LOCAL_TO_CLIP,
	LOCAL_TO_WORLD,
	WORLD_TO_CLIP,
	CAMERA_TO_CLIP,
	WORLD_TO_CAMERA,
	DIFFUSE_TEXTURE_SAMPLER,
	SPECULAR_TEXTURE_SAMPLER,
	NORMAL_TEXTURE_SAMPLER,
	CUBEMAP_TEXTURE_SAMPLER,
	GAUSSIAN_SPECULAR_TERM,
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

class Shader {
public:
	explicit Shader(GLSLSource const&);
	~Shader();

	GLuint uniform_handle(UniformMapping);
	GLuint uniform_block_buffer(UniformBlockMapping);

	/// this has to be called every frame on NVIDIA cards
	/// because NVIDIA forces the shader to recompile
	/// which messes it up
	/// goddamnit NVIDIA
	void bind();

private:
	GLuint m_handle;
	std::unordered_map<UniformMapping, GLuint> m_unifs;
	std::unordered_map<UniformBlockMapping, GLuint> m_unif_blocks;
};

// TODO how to load textures?
// maybe material first from resource loader, defined by entity
// and from material load textures with resource loader
// then store all as psi_gl::Material
class Texture2D {
public:
	explicit Texture2D(std::vector<char> const&);
	~Texture2D();

	void bind_to_tex_unit(TextureUnit) const;

private:
	GLuint m_handle;
};

// TODO PBR
struct Material {
	Texture2D diffuse;
	Texture2D specular;
	Texture2D normal;
	Texture2D gloss;
	float metalicness;
};
} // namespace psi_gl