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

#include "helper.hpp"

#include <regex>

#include "../../../util/assert.hpp"
#include "../../../log/log.hpp"


// -- MeshBuffer --
psi_gl::MeshBuffer::MeshBuffer(psi_rndr::MeshData const& mesh) {
	gl::GenVertexArrays(1, &m_VAO);
	gl::GenBuffers(1, &m_VBO);
	gl::GenBuffers(1, &m_EBO);

	// a m_vao stores the buffer bound to ELEMENT_ARRAY_BUFFER when that m_vao is active
	// and the m_vao seems to also store the ARRAY_BUFFER when VertexAttribPointer and EnableVertexAttribArray are called
	gl::BindVertexArray(m_VAO);
	gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, m_EBO);
	gl::BindBuffer(gl::ARRAY_BUFFER, m_VBO);

	// describe how vertices are laid out in vert_memory
	gl::VertexAttribPointer(GLuint(ShaderVertexAttrib::POS), 3, gl::FLOAT, false, psi_rndr::VertexData::FLOATS_PER_VERTEX * sizeof(float), nullptr);
	gl::EnableVertexAttribArray(GLuint(ShaderVertexAttrib::POS));

	gl::VertexAttribPointer(GLuint(ShaderVertexAttrib::NORM), 3, gl::FLOAT, false, psi_rndr::VertexData::FLOATS_PER_VERTEX * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
	gl::EnableVertexAttribArray(GLuint(ShaderVertexAttrib::NORM));

	gl::VertexAttribPointer(GLuint(ShaderVertexAttrib::TAN), 3, gl::FLOAT, false, psi_rndr::VertexData::FLOATS_PER_VERTEX * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));
	gl::EnableVertexAttribArray(GLuint(ShaderVertexAttrib::TAN));

	gl::VertexAttribPointer(GLuint(ShaderVertexAttrib::UV), 2, gl::FLOAT, false, psi_rndr::VertexData::FLOATS_PER_VERTEX * sizeof(float), reinterpret_cast<void*>(9 * sizeof(float)));
	gl::EnableVertexAttribArray(GLuint(ShaderVertexAttrib::UV));

	// buffer sizes
	size_t const VERTICES_SIZE = mesh.vertices.size() * psi_rndr::VertexData::FLOATS_PER_VERTEX * sizeof(float);
	size_t const INDICES_SIZE = mesh.indices.size() * sizeof(uint32_t);

	// create and initialize data storage
	// TODO is gl::DYNAMIC_STORAGE_BIT needed here?
	gl::BufferStorage(gl::ARRAY_BUFFER, VERTICES_SIZE, nullptr, gl::DYNAMIC_STORAGE_BIT | gl::MAP_WRITE_BIT | gl::MAP_PERSISTENT_BIT | gl::MAP_COHERENT_BIT);
	gl::BufferStorage(gl::ELEMENT_ARRAY_BUFFER, INDICES_SIZE, nullptr, gl::DYNAMIC_STORAGE_BIT | gl::MAP_WRITE_BIT | gl::MAP_PERSISTENT_BIT | gl::MAP_COHERENT_BIT);

	// upload vertices
	float* vert_mem = static_cast<float*>(gl::MapBufferRange(gl::ARRAY_BUFFER, 0, VERTICES_SIZE, gl::MAP_WRITE_BIT | gl::MAP_PERSISTENT_BIT | gl::MAP_COHERENT_BIT | gl::MAP_INVALIDATE_BUFFER_BIT));

	for (size_t i_vert = 0; i_vert < mesh.vertices.size(); ++i_vert) {
		auto const& vert = mesh.vertices[i_vert];
		size_t id = i_vert * psi_rndr::VertexData::FLOATS_PER_VERTEX;
		vert_mem[id] = vert.pos[0];
		vert_mem[id + 1] = vert.pos[1];
		vert_mem[id + 2] = vert.pos[2];
		vert_mem[id + 3] = vert.norm[0];
		vert_mem[id + 4] = vert.norm[1];
		vert_mem[id + 5] = vert.norm[2];
		vert_mem[id + 6] = vert.tan[0];
		vert_mem[id + 7] = vert.tan[1];
		vert_mem[id + 8] = vert.tan[2];
		vert_mem[id + 9] = vert.uv[0];
		vert_mem[id + 10] = vert.uv[1];
	}

	// upload indices
	uint32_t* index_mem = static_cast<uint32_t*>(gl::MapBufferRange(gl::ELEMENT_ARRAY_BUFFER, 0, INDICES_SIZE, gl::MAP_WRITE_BIT | gl::MAP_PERSISTENT_BIT | gl::MAP_COHERENT_BIT | gl::MAP_INVALIDATE_BUFFER_BIT));

	for (size_t i_ind = 0; i_ind < mesh.indices.size(); ++i_ind) {
		index_mem[i_ind] = mesh.indices[i_ind];
	}

	m_index_count = GLuint(mesh.indices.size());
}

psi_gl::MeshBuffer::~MeshBuffer() {
	//gl::DeleteBuffers(1, &m_VBO);
	//gl::DeleteBuffers(1, &m_EBO);
	//gl::DeleteVertexArrays(1, &m_VAO);
}

void psi_gl::MeshBuffer::draw(GLenum primitives) {
	gl::BindVertexArray(m_VAO);
	gl::DrawElements(primitives, m_index_count, gl::UNSIGNED_INT, nullptr);
}

// -- MultipleRenderTargetFramebuffer --
static inline GLenum internal_format_to_data_format(GLenum internal) {
	switch (internal) {
		case gl::RGB16F:
			return gl::RGB;

		case gl::RGBA:
			return gl::RGBA;

		case gl::DEPTH_COMPONENT:
			return gl::DEPTH_COMPONENT;

		default:
			ASSERT(false);
	}
}

static inline GLenum internal_format_to_type(GLenum internal) {
	switch (internal) {
		case gl::RGB16F:
			return gl::FLOAT;

		case gl::RGBA:
			return gl::UNSIGNED_BYTE;

		case gl::DEPTH_COMPONENT:
			return gl::FLOAT;

		default:
			ASSERT(false);
	}
}

psi_gl::MultipleRenderTargetFramebuffer::MultipleRenderTargetFramebuffer(std::vector<FramebufferRenderTargetCreationInfo> targets, uint32_t width, uint32_t height) {
	gl::GenFramebuffers(1, &m_framebuffer);
	gl::BindFramebuffer(gl::FRAMEBUFFER, m_framebuffer);

	GLint last_color = 0;
	bool depth_attached = false;
	std::vector<GLenum> attachments;

	for (auto const& info : targets) {
		GLuint handle;

		if (info.texture_or_not_renderbuffer) {
			gl::GenTextures(1, &handle);
			gl::BindTexture(gl::TEXTURE_2D, handle);
			gl::TexImage2D(gl::TEXTURE_2D, 0, info.internal_format, width, height, 0, internal_format_to_data_format(info.internal_format), internal_format_to_type(info.internal_format), 0);

			if (info.color_or_not_depth) {
				gl::FramebufferTexture2D(gl::FRAMEBUFFER, gl::COLOR_ATTACHMENT0 + last_color, gl::TEXTURE_2D, handle, 0);
				attachments.push_back(gl::COLOR_ATTACHMENT0 + last_color);
				++last_color;
			}
			else {
				ASSERT(!depth_attached);
				gl::FramebufferTexture2D(gl::FRAMEBUFFER, gl::DEPTH_ATTACHMENT, gl::TEXTURE_2D, handle, 0);
				depth_attached = true;
			}

			m_textures.push_back(handle);
		}
		else {
			gl::GenRenderbuffers(1, &handle);
			gl::BindRenderbuffer(gl::RENDERBUFFER, handle);
			gl::RenderbufferStorage(gl::RENDERBUFFER, info.internal_format, width, height);

			if (info.color_or_not_depth) {
				gl::FramebufferRenderbuffer(gl::FRAMEBUFFER, gl::COLOR_ATTACHMENT0 + last_color, gl::RENDERBUFFER, handle);
				attachments.push_back(gl::COLOR_ATTACHMENT0 + last_color);
				++last_color;
			}
			else {
				ASSERT(!depth_attached);
				gl::FramebufferRenderbuffer(gl::FRAMEBUFFER, gl::DEPTH_ATTACHMENT, gl::RENDERBUFFER, handle);
				depth_attached = true;
			}

			m_renderbuffers.push_back(handle);
		}
	}

	gl::DrawBuffers(attachments.size(), attachments.data());
}

psi_gl::MultipleRenderTargetFramebuffer::~MultipleRenderTargetFramebuffer() {
	gl::DeleteTextures(m_textures.size(), m_textures.data());
	gl::DeleteRenderbuffers(m_renderbuffers.size(), m_renderbuffers.data());

	gl::DeleteFramebuffers(1, &m_framebuffer);
}

void psi_gl::MultipleRenderTargetFramebuffer::bind() {
	gl::BindFramebuffer(gl::FRAMEBUFFER, m_framebuffer);
}

void psi_gl::MultipleRenderTargetFramebuffer::unbind() {
	gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
}

GLuint psi_gl::MultipleRenderTargetFramebuffer::texture_target_handle(size_t index) {
	ASSERT(index < m_textures.size());
	return m_textures[index];
}

// -- SAMPLERS --
void psi_gl::create_sampler_at_texture_unit(psi_gl::SamplerSettings settings, GLuint tex_unit) {
	GLuint sampler;

	gl::GenSamplers(1, &sampler);

	gl::SamplerParameteri(sampler, gl::TEXTURE_MAG_FILTER, settings.mag_filter);
	gl::SamplerParameteri(sampler, gl::TEXTURE_MIN_FILTER, settings.min_filter);
	gl::SamplerParameteri(sampler, gl::TEXTURE_WRAP_S, settings.wrap_S);
	gl::SamplerParameteri(sampler, gl::TEXTURE_WRAP_T, settings.wrap_T);
	gl::SamplerParameterf(sampler, gl::TEXTURE_MAX_ANISOTROPY_EXT, settings.max_aniso);

	gl::BindSampler(tex_unit, sampler);
}

// -- UniformMapping --
/// Matches a UniformMapping to a given string.
/// @throw if the string does not match any mapping
inline static psi_gl::UniformMapping uniform_mapping_from_name(std::string const& name) {
	// TODO do dis wit templates?
	#define UMAP(s) if(name == #s) return psi_gl::UniformMapping::s;
	UMAP(LOCAL_TO_WORLD)
	UMAP(WORLD_TO_CAMERA)
	UMAP(LOCAL_TO_CLIP)
	UMAP(WORLD_TO_CLIP)
	UMAP(CAMERA_TO_CLIP)
	UMAP(ALBEDO_TEXTURE_SAMPLER)
	UMAP(NORMAL_TEXTURE_SAMPLER)
	UMAP(REFLECTIVENESS_ROUGHNESS_TEXTURE_SAMPLER)
	UMAP(CUBEMAP_TEXTURE_SAMPLER)
	UMAP(POSITION_FRAME_TEXTURE_SAMPLER)
	UMAP(NORMAL_FRAME_TEXTURE_SAMPLER)
	UMAP(ALBEDO_FRAME_TEXTURE_SAMPLER)
	UMAP(REFL_ROUGH_FRAME_TEXTURE_SAMPLER)
	UMAP(CAMERA_POSITION_WORLD)
	UMAP(ACTIVE_POINT_LIGHTS)
	UMAP(ACTIVE_SPOT_LIGHTS)
	UMAP(MESH_MIN_POS)
	UMAP(MESH_MAX_POS)

	throw std::runtime_error("Tried to resolve invalid uniform mapping " + name + ".");
}

/// Returns the string representation of the GLSL uniform type that a given UniformMapping has.
/// @return the string or "" if invalid value is passed
inline static std::string uniform_mapping_glsl_type(psi_gl::UniformMapping map) {
	switch (map) {
	case psi_gl::UniformMapping::LOCAL_TO_WORLD:
	case psi_gl::UniformMapping::WORLD_TO_CAMERA:
	case psi_gl::UniformMapping::LOCAL_TO_CLIP:
	case psi_gl::UniformMapping::WORLD_TO_CLIP:
	case psi_gl::UniformMapping::CAMERA_TO_CLIP:
		return "mat4";

	case psi_gl::UniformMapping::ALBEDO_TEXTURE_SAMPLER:
	case psi_gl::UniformMapping::REFLECTIVENESS_ROUGHNESS_TEXTURE_SAMPLER:
	case psi_gl::UniformMapping::NORMAL_TEXTURE_SAMPLER:
	case psi_gl::UniformMapping::POSITION_FRAME_TEXTURE_SAMPLER:
	case psi_gl::UniformMapping::NORMAL_FRAME_TEXTURE_SAMPLER:
	case psi_gl::UniformMapping::ALBEDO_FRAME_TEXTURE_SAMPLER:
	case psi_gl::UniformMapping::REFL_ROUGH_FRAME_TEXTURE_SAMPLER:
		return "sampler2D";

	case psi_gl::UniformMapping::CUBEMAP_TEXTURE_SAMPLER:
		return "samplerCube";

	case psi_gl::UniformMapping::MESH_MIN_POS:
	case psi_gl::UniformMapping::MESH_MAX_POS:
	case psi_gl::UniformMapping::CAMERA_POSITION_WORLD:
		return "vec3";

	case psi_gl::UniformMapping::ACTIVE_POINT_LIGHTS:
	case psi_gl::UniformMapping::ACTIVE_SPOT_LIGHTS:
		return "uint";

	default:
		return "";
	}
}

// -- UniformBlockMapping --
/// Matches a UniformBlockMapping to a given string.
/// @throw if the string does not match any mapping
inline static psi_gl::UniformBlockMapping uniform_block_mapping_from_name(std::string const& name) {
	#define UBMAP(s) if (name == #s) return psi_gl::UniformBlockMapping::s;
	UBMAP(LIGHT_DATA)

	throw std::runtime_error("Tried to resolve invalid uniform block mapping " + name + ".");
}

// -- SHADERS --
psi_gl::GLSLSource psi_gl::parse_glsl_source(std::array<std::vector<std::string>, 6> const& sources) {
	GLSLSource obj;
	// TODO parse the directives better
	// now .e.g #map WOLOLO; <- mind the semicolon
	// will not raise an error and pass to GL where it fails to compile
	// maybe first find "#map" and then look at the rest of the line
	// and raise warnings when there are signs others than letters and underlines

	std::regex const map_rgx("\\s*#map\\s+\\w+(?=\\s*\\n)");
	std::regex const unif_rgx("uniform\\s+\\w+\\s+\\w+(?=\\s*;)");
	std::regex const block_rgx("uniform\\s+\\w+(?=\\s*(\\{|\\n))");

	for (size_t i_src = 0; i_src < sources.size(); ++i_src) {
		auto const& src = sources[i_src];
		// lines containing eGLSL preprocessor definitions
		std::vector<size_t> preproc_lines;

		if (src.size() != 0) {
			for (size_t i_line = 0; i_line < src.size() - 1; ++i_line) {
				auto const& line = src[i_line];
				auto const& line_below = src[i_line + 1];

				std::smatch result;

				// line is #map [RESOURCE]
				if (regex_search(line, result, map_rgx)) {
					auto map_str = result.str();

					// remember that this line contains a custom preprocessor directive
					preproc_lines.push_back(i_line);

					// remove all whitespace
					map_str.erase(remove_if(map_str.begin(), map_str.end(), isspace), map_str.end());

					// remove the "#map" part
					map_str = map_str.substr(4);

					// check line below
					// line below is a uniform
					if (regex_search(line_below, result, unif_rgx)) {
						auto result_str = result.str();

						// this matches the uniform type
						std::smatch unif;
						regex_search(result_str, unif, std::regex("\\s+\\w+"));
						auto unif_type_str = unif.str();

						// uniform name starts after the type match
						auto unif_name_str = result_str.substr(unif.position() + unif.length());

						// remove all whitespace
						unif_name_str.erase(remove_if(unif_name_str.begin(), unif_name_str.end(), isspace), unif_name_str.end());
						unif_type_str.erase(remove_if(unif_type_str.begin(), unif_type_str.end(), isspace), unif_type_str.end());

						// find uniform mapping enum
						psi_gl::UniformMapping map;
						try {
							map = uniform_mapping_from_name(map_str);
						}
						catch (std::exception const& e) {
							throw std::runtime_error("line " + std::to_string(i_line + 1) + ":\n" + e.what());
						}

						// check if uniform type matches resource type
						if (uniform_mapping_glsl_type(map) != unif_type_str)
							throw std::runtime_error("line " + std::to_string(i_line + 2) + ":\nUniform type " + unif_type_str + " does not match mapped resource type " + uniform_mapping_glsl_type(map) + ".");

						// just insert. one mapping can map to many uniforms
						obj.unifs.insert(std::make_pair(map, unif_name_str));
					}
					// line below is a uniform block
					else if (regex_search(line_below, result, block_rgx)) {
						auto result_str = result.str();

						// remember that this line contains a custom preprocessor directive
						preproc_lines.push_back(i_line);

						// remove whitespace
						result_str.erase(remove_if(result_str.begin(), result_str.end(), isspace), result_str.end());

						// remove the "uniform" part
						result_str = result_str.substr(7);

						// try to find uniform block mapping enum
						psi_gl::UniformBlockMapping block_map;
						try {
							block_map = uniform_block_mapping_from_name(result_str);
						}
						catch (std::exception const& e) {
							throw std::runtime_error("line " + std::to_string(i_line + 1) + ":\n" + e.what());
						}

						// TODO block isn't checked for compatiblity with resource
						// checking would require a complicated parser of the block contents

						// just insert block. one mapping can map to many blocks
						obj.unif_blocks.insert(std::make_pair(block_map, result_str));
					}
					// line below #map is neither a uniform nor a block
					else
						throw std::runtime_error("line " + std::to_string(i_line + 2 /*line numbers start at 1 and we are going below, hence + 2*/) + ":\n#map directive does not have a uniform or a uniform block declaration below it.");
				}
			}

			// erase lines containing eGLSL preprocessor directives in copy of source
			std::vector<std::string> src_cpy = src;
			for (size_t i_line = preproc_lines.size(); i_line != 0; --i_line)
				src_cpy.erase(src_cpy.begin() + preproc_lines[i_line - 1]);

			// concatentate source into one string
			std::string src_concat;
			for (auto const& line : src_cpy) {
				src_concat += line;
			}

			// sources are put into their respective string based upon their position in the array
			switch (static_cast<SourceTypeInArray>(i_src)) {
			case SourceTypeInArray::VERTEX:
				obj.vertex = src_concat;
				break;
			case SourceTypeInArray::FRAGMENT:
				obj.fragment = src_concat;
				break;
			case SourceTypeInArray::GEOMETRY:
				obj.geometry = src_concat;
				break;
			case SourceTypeInArray::TESSELATION_CONTROL:
				obj.tess_ctrl = src_concat;
				break;
			case SourceTypeInArray::TESSELATION_EVALUATION:
				obj.tess_eval = src_concat;
				break;
			case SourceTypeInArray::COMPUTE:
				obj.compute = src_concat;
				break;
			}
		}
	}

	// in case it tries copying or something
	return obj;
}

inline static GLuint _compile_shader_source(std::string const& source, GLenum type) {
	// create a shader
	GLuint ID = gl::CreateShader(type);

	// send shader source to OpenGL
	char const* c_src = source.c_str();

	gl::ShaderSource(ID, 1, &c_src, nullptr);

	// try to compile the shader
	gl::CompileShader(ID);

	// get compile status
	GLint status = 2;
	gl::GetShaderiv(ID, gl::COMPILE_STATUS, &status);

	if (status != 0 && status != 1)
		throw std::runtime_error("Couldn't retrieve shader compilation status.");

	// get info log length
	GLint info_log_length;
	gl::GetShaderiv(ID, gl::INFO_LOG_LENGTH, &info_log_length);

	// get the info log
	char* info_log = new char[info_log_length + 1];
	gl::GetShaderInfoLog(ID, info_log_length, nullptr, info_log);

	if (status == 0) {
		// print out the info log
		throw std::runtime_error("Failed to compile shader.\nOpenGL log:\n" + std::string(info_log));
	}

	if (info_log_length > 1) {
		// print out the info log if it's present
		psi_log::info("OpenGL Shader Compilation") << "Log:\n" + std::string(info_log) << "\n";
		delete[] info_log;
	}

	return ID;
}

inline static void _check_validation_link_error(GLuint handle, GLenum flag) {
	// make a variable to store status in, fill it with invalid value for when it can't be retrieved
	GLint status = 2;
	gl::GetProgramiv(handle, flag, &status);

	// print out status
	if (status != 0 && status != 1)
		throw std::runtime_error("Couldn't retrieve shader program " + std::to_string(handle) + " link/validate status.");

	// get info log length
	GLint info_log_length;
	gl::GetProgramiv(handle, gl::INFO_LOG_LENGTH, &info_log_length);

	// get info log
	GLchar* info_log = new GLchar[info_log_length + 1];
	gl::GetProgramInfoLog(handle, info_log_length, nullptr, info_log);

	// if failed to link/validate
	if (status == false) {
		std::string log = info_log;
		delete[] info_log;
		throw std::runtime_error("Failed to link/validate shader program " + std::to_string(handle) + ":\n" + log + "\n");
	}

	// if succeeded and there is an info log
	if (info_log_length > 1) {
		psi_log::info("OpenGL Shader Validation") << "Shader program " + std::to_string(handle) + " info log:\n" + info_log << "\n";
		delete[] info_log;
	}
}

psi_gl::Shader psi_gl::compile_glsl_source(GLSLSource const& src) {
	Shader shader;
	shader.handle = gl::CreateProgram();

	std::vector<GLuint> shader_handles;
	shader_handles.push_back(_compile_shader_source(src.vertex, gl::VERTEX_SHADER));
	shader_handles.push_back(_compile_shader_source(src.fragment, gl::FRAGMENT_SHADER));
	if (src.geometry)
		shader_handles.push_back(_compile_shader_source(*src.geometry, gl::GEOMETRY_SHADER));
	if (src.tess_ctrl)
		shader_handles.push_back(_compile_shader_source(*src.tess_ctrl, gl::TESS_CONTROL_SHADER));
	if (src.tess_eval)
		shader_handles.push_back(_compile_shader_source(*src.tess_eval, gl::TESS_EVALUATION_SHADER));
	if (src.compute)
		shader_handles.push_back(_compile_shader_source(*src.compute, gl::COMPUTE_SHADER));

	for (auto h : shader_handles)
		gl::AttachShader(shader.handle, h);

	gl::LinkProgram(shader.handle);
	_check_validation_link_error(shader.handle, gl::LINK_STATUS);

	for (auto const& unif : src.unifs) {
		GLint loc = gl::GetUniformLocation(shader.handle, unif.second.c_str());

		// -1 is magic value for invalid uniforms
		if (loc == -1)
			psi_log::warning("OpenGL Shader Compilation") << "Mapped uniform " + unif.second + " is not active in shader program.";
		else
			shader.unifs.emplace(unif.first, loc);
	}

	GLuint bind_point = 0;
	for (auto const& block : src.unif_blocks) {
		GLuint loc  = gl::GetUniformBlockIndex(shader.handle, block.second.c_str());

		if (loc == gl::INVALID_INDEX)
			psi_log::warning("OpenGL Shader Compilation") << "Mapped uniform block " + block.second + " is not active in shader program.";
		else {
			GLuint buf;
			gl::GenBuffers(1, &buf);

			if (shader.unif_blocks.size() != 0)
				++bind_point;

			gl::UniformBlockBinding(shader.handle, loc, bind_point);
			gl::BindBufferBase(gl::UNIFORM_BUFFER, bind_point, buf);

			shader.unif_blocks.emplace(block.first, buf);
		}
	}

	gl::ValidateProgram(shader.handle);
	_check_validation_link_error(shader.handle, gl::VALIDATE_STATUS);

	for (auto h : shader_handles) {
		gl::DetachShader(shader.handle, h);
		gl::DeleteShader(h);
	}

	return shader;
}
