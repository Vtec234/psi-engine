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

layout(location = 0) in vec3 pos_model_vec3;
layout(location = 1) in vec3 norm_model_vec3;
layout(location = 2) in vec3 tan_model_vec3;
layout(location = 3) in vec2 uv_tan_vec2;

#map LOCAL_TO_WORLD
uniform mat4 model_to_world_mat4;
#map LOCAL_TO_CLIP
uniform mat4 model_to_clip_mat4;

smooth out vec3 pos_world_vec3;
smooth out vec2 uv_tan_vec2_out;
smooth out mat3 tan_to_world_mat3;

void main() {
	pos_world_vec3 = vec3(model_to_world_mat4 * vec4(pos_model_vec3, 1.0));

	uv_tan_vec2_out = uv_tan_vec2;

	// rotate normal and tangent to world space
	mat3 model_to_world_mat3 = mat3(model_to_world_mat4);
	vec3 norm_world_vec3 = normalize(model_to_world_mat3 * norm_model_vec3);
	vec3 tan_world_vec3 = normalize(model_to_world_mat3 * tan_model_vec3);

	// use the Gramm-Schmidt process to reorthogonalize tangent with normal
	tan_world_vec3 = normalize(tan_world_vec3 - dot(tan_world_vec3, norm_world_vec3) * norm_world_vec3);

	vec3 bitan_world_vec3 = cross(tan_world_vec3, norm_world_vec3);

	// tangent space to world space transformation matrix
	tan_to_world_mat3 = mat3(tan_world_vec3, bitan_world_vec3, norm_world_vec3);

	gl_Position = model_to_clip_mat4 * vec4(pos_model_vec3, 1.0);
}
