/*
 * Copyright (C) 2015-2016 Wojciech Nawrocki
 *
 * This file is part of Generic Shooter Game.
 *
 * Generic Shooter Game is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Generic Shooter Game is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Generic Shooter Game. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#version 430


/// THIS SHADER MAY OR MAY NOT BE COMPLETELY BROKEN
// -- VERTEX INPUT DATA --
layout(location = 0) in vec3 vertPos_model;
layout(location = 1) in vec3 vertNorm_model;
layout(location = 2) in vec3 vertTangent_model;
layout(location = 3) in vec2 vertTexCoords;

// -- TRANSFORMATION MATRICES --
#map LOCAL_TO_CLIP
uniform mat4 modelToClipMat4;
#map LOCAL_TO_WORLD
uniform mat4 modelToWorldMat4;

// -- VERTEX DATA OUTPUT TO RASTERIZER --
smooth out vec3 fragPos_world;
smooth out vec2 fragTexCoords;
smooth out mat3 tangentToWorldMat3;

void main() {
	// rotate, translate and scale to world space
	fragPos_world = vec3(modelToWorldMat4 * vec4(vertPos_model, 1.0));

	// pass texture coordinates
	fragTexCoords = vertTexCoords;

	// rotate to world space
	mat3 modelToWorldMat3 = mat3(modelToWorldMat4);
	vec3 vertNorm_world = normalize(modelToWorldMat3 * vertNorm_model);
	vec3 vertTangent_world = normalize(modelToWorldMat3 * vertTangent_model);

	// use the Gramm-Schmidt process to reorthogonalize tangent with normal
	vertTangent_world = normalize(vertTangent_world - dot(vertTangent_world, vertNorm_world) * vertNorm_world);

	vec3 vertBitangent_world = cross(vertTangent_world, vertNorm_world);

	// tangent space to world space transformation matrix
	tangentToWorldMat3 = mat3(vertTangent_world, vertBitangent_world, vertNorm_world);
	
	// rotate, translate and scale to clip space
	gl_Position = modelToClipMat4 * vec4(vertPos_model, 1.0);
}
