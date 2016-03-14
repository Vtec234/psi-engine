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

#include "../resource.hpp"


namespace psi_serv {
/// A resource that a uniform in a shader may be attached to.
/// The value of this resource will be sent to the shader at runtime.
enum UniformMapping {
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
	MESH_MAX_POS
};

/// Matches a UniformMapping to a given string.
/// @throw if the string does not match any mapping
UniformMapping uniform_mapping_from_name(std::string const&);

/// A memory buffer that a uniform block in a shader may be attached to.
/// The memory will be sent to the shader at runtime.
enum UniformBlockMapping {
	LIGHT_DATA
};

/// Matches a UniformBlockMapping to a given string.
/// @throw if the string does not match any mapping
UniformBlockMapping uniform_block_mapping_from_name(std::string const&);
} // namespace gsg

// stupid std hash breaking my namespace apart
namespace std {
template <>
struct hash<psi_serv::UniformMapping> {
	size_t operator()(psi_serv::UniformMapping const& m) const {
		return static_cast<size_t>(m);
	}
};

template <>
struct hash<psi_serv::UniformBlockMapping> {
	size_t operator()(psi_serv::UniformBlockMapping const& m) const {
		return static_cast<size_t>(m);
	}
};
} // namespace std

namespace psi_serv {
/// A resource class representing the whole source (vertex, fragment, etc.) of a single GLSL shader.
class GLSLSource : public IResource {
public:
	/// Returns the string representation of the GLSL uniform type that a given UniformMapping has.
	/// @return the string or "" if invalid value is passed
	static std::string uniform_type(UniformMapping map);

	/// An enum representing where each type of eGLSL source should be in the array passed to construct_from_sources.
	enum SourceTypeInArray {
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
	static std::unique_ptr<IResource> construct_from_sources(std::array<std::vector<std::string>, 6> const& sources);

	~GLSLSource() = default;

	std::unique_ptr<IResource> clone() const override;

	std::string const& vertex_shader() const;
	std::string const& fragment_shader() const;

	boost::optional<std::string> const& geometry_shader() const;
	boost::optional<std::string> const& tess_control_shader() const;
	boost::optional<std::string> const& tess_eval_shader() const;
	boost::optional<std::string> const& compute_shader() const;

	std::unordered_multimap<UniformMapping, std::string> const& mapped_uniforms() const;
	std::unordered_multimap<UniformBlockMapping, std::string> const& mapped_uniform_blocks() const;

private:
	// the sources
	std::string m_vertex;
	std::string m_fragment;

	boost::optional<std::string> m_geometry;
	boost::optional<std::string> m_tess_ctrl;
	boost::optional<std::string> m_tess_eval;
	boost::optional<std::string> m_compute;

	// mappings of uniform and uniform block names
	std::unordered_multimap<UniformMapping, std::string> m_unifs;
	std::unordered_multimap<UniformBlockMapping, std::string> m_unif_blocks;
};
} // namespace psi_serv
