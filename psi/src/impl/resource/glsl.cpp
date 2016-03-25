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

#include "glsl.hpp"

#include <regex>


// -- UniformMapping --
psi_util::UniformMapping psi_util::uniform_mapping_from_name(std::string const& name) {
	// TODO do dis wit templates?
	#define UMAP(s) if(name == #s) return UniformMapping::s;
	UMAP(LOCAL_TO_CLIP)
	UMAP(LOCAL_TO_WORLD)
	UMAP(WORLD_TO_CLIP)
	UMAP(CAMERA_TO_CLIP)
	UMAP(WORLD_TO_CAMERA)
	UMAP(DIFFUSE_TEXTURE_SAMPLER)
	UMAP(SPECULAR_TEXTURE_SAMPLER)
	UMAP(NORMAL_TEXTURE_SAMPLER)
	UMAP(CUBEMAP_TEXTURE_SAMPLER)
	UMAP(GAUSSIAN_SPECULAR_TERM)
	UMAP(CAMERA_POSITION_WORLD)
	UMAP(ACTIVE_POINT_LIGHTS)
	UMAP(ACTIVE_SPOT_LIGHTS)
	UMAP(MESH_MIN_POS)
	UMAP(MESH_MAX_POS)

	throw std::runtime_error("Tried to resolve invalid uniform mapping " + name + ".");
}

std::string psi_util::uniform_mapping_glsl_type(UniformMapping map) {
	switch (map) {
	case UniformMapping::LOCAL_TO_CLIP:
	case UniformMapping::LOCAL_TO_WORLD:
	case UniformMapping::WORLD_TO_CLIP:
	case UniformMapping::WORLD_TO_CAMERA:
	case UniformMapping::CAMERA_TO_CLIP:
		return "mat4";

	case UniformMapping::DIFFUSE_TEXTURE_SAMPLER:
	case UniformMapping::SPECULAR_TEXTURE_SAMPLER:
	case UniformMapping::NORMAL_TEXTURE_SAMPLER:
		return "sampler2D";

	case UniformMapping::CUBEMAP_TEXTURE_SAMPLER:
		return "samplerCube";

	case UniformMapping::GAUSSIAN_SPECULAR_TERM:
		return "float";

	case UniformMapping::MESH_MIN_POS:
	case UniformMapping::MESH_MAX_POS:
	case UniformMapping::CAMERA_POSITION_WORLD:
		return "vec3";

	case UniformMapping::ACTIVE_POINT_LIGHTS:
	case UniformMapping::ACTIVE_SPOT_LIGHTS:
		return "uint";

	default:
		return "";
	}
}

// -- UniformBlockMapping --
psi_util::UniformBlockMapping psi_util::uniform_block_mapping_from_name(std::string const& name) {
	#define UBMAP(s) if (name == #s) return UniformBlockMapping::s;
	UBMAP(LIGHT_DATA)

	throw std::runtime_error("Tried to resolve invalid uniform block mapping " + name + ".");
}

// -- GLSLSource --
psi_util::GLSLSource psi_util::GLSLSource::parse_glsl_sources(std::array<std::vector<std::string>, 6> const& sources) {
	GLSLSource obj;

	// idea was to require #type, but now it seems to rigid
	// ie one file can't be used as two types of shader
	//std::regex const type_rgx("\\s*#type\\s*((frag)|(vert)|(geom)|(tess_ctrl)|(tess_eval)|(comp))(?=\\s*\\n)");
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
						psi_util::UniformMapping map;
						try {
							map = psi_util::uniform_mapping_from_name(map_str);
						}
						catch (std::exception const& e) {
							throw std::runtime_error("line " + std::to_string(i_line + 1) + ":\n" + e.what());
						}

						// check if uniform type matches resource type
						if (uniform_mapping_glsl_type(map) != unif_type_str)
							throw std::runtime_error("line " + std::to_string(i_line + 2) + ":\nUniform type " + unif_type_str + " does not match mapped resource type " + uniform_mapping_glsl_type(map) + ".");

						// just insert. one mapping can map to many uniforms
						obj.m_unifs.insert(std::make_pair(map, unif_name_str));
					}
					// line below is a uniform block
					else if (regex_search(line_below, result, block_rgx)) {
						auto result_str = result.str();

						// remove whitespace
						result_str.erase(remove_if(result_str.begin(), result_str.end(), isspace), result_str.end());

						// remove the "uniform" part
						result_str = result_str.substr(7);

						// try to find uniform block mapping enum
						psi_util::UniformBlockMapping block_map;
						try {
							block_map = psi_util::uniform_block_mapping_from_name(result_str);
						}
						catch (std::exception const& e) {
							throw std::runtime_error("line " + std::to_string(i_line + 1) + ":\n" + e.what());
						}

						// TODO block isn't checked for compatiblity with resource
						// checking would require a complicated parser of the block contents

						// just insert block. one mapping can map to many blocks
						obj.m_unif_blocks.insert(std::make_pair(block_map, result_str));
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
				obj.m_vertex = src_concat;
				break;
			case SourceTypeInArray::FRAGMENT:
				obj.m_fragment = src_concat;
				break;
			case SourceTypeInArray::GEOMETRY:
				obj.m_geometry = src_concat;
				break;
			case SourceTypeInArray::TESSELATION_CONTROL:
				obj.m_tess_ctrl = src_concat;
				break;
			case SourceTypeInArray::TESSELATION_EVALUATION:
				obj.m_tess_eval = src_concat;
				break;
			case SourceTypeInArray::COMPUTE:
				obj.m_compute = src_concat;
				break;
			}
		}
	}

	// in case it tries copying or something
	return obj;
}

std::string const& psi_util::GLSLSource::vertex_shader() const {
	return m_vertex;
}

std::string const& psi_util::GLSLSource::fragment_shader() const {
	return m_fragment;
}

boost::optional<std::string> const& psi_util::GLSLSource::geometry_shader() const {
	return m_geometry;
}

boost::optional<std::string> const& psi_util::GLSLSource::tess_control_shader() const{
	return m_tess_ctrl;
}

boost::optional<std::string> const& psi_util::GLSLSource::tess_eval_shader() const {
	return m_tess_eval;
}

boost::optional<std::string> const& psi_util::GLSLSource::compute_shader() const {
	return m_compute;
}

std::unordered_multimap<psi_util::UniformMapping, std::string> const& psi_util::GLSLSource::mapped_uniforms() const {
	return m_unifs;
}

std::unordered_multimap<psi_util::UniformBlockMapping, std::string> const& psi_util::GLSLSource::mapped_uniform_blocks() const {
	return m_unif_blocks;
}
