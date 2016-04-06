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

#version 430

smooth in vec3 pos_world_vec3;
smooth in vec2 uv_tan_vec2_out;
smooth in mat3 tan_to_world_mat3;

#map NORMAL_TEXTURE_SAMPLER
uniform sampler2D normal_sampler;

#map ALBEDO_TEXTURE_SAMPLER
uniform sampler2D albedo_sampler;

#map REFLECTIVENESS_ROUGHNESS_TEXTURE_SAMPLER
uniform sampler2D rr_sampler;

layout(location = 0) out vec3 pos_world_vec3_out;
layout(location = 1) out vec3 norm_world_vec3;
layout(location = 2) out vec4 albedo_vec4;
layout(location = 3) out vec4 rr_vec4;

void main() {
	pos_world_vec3_out = pos_world_vec3;
	norm_world_vec3 = normalize(tan_to_world_mat3 * (255.0/128.0 * texture(normal_sampler, uv_tan_vec2_out).rgb - 1.0));
	albedo_vec4 = texture(albedo_sampler, uv_tan_vec2_out).rgba;
	rr_vec4 = texture(rr_sampler, uv_tan_vec2_out).rgba;
}
