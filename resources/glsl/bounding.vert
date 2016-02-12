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


#map LOCAL_TO_CLIP
uniform mat4 localToClipMat4;
#map MESH_MAX_POS
uniform vec3 maxPos;
#map MESH_MIN_POS
uniform vec3 minPos;

/// Draws a wireframe cube with specified min and max coordinates.
/// Use with GL_LINES.
/// Bitwise magic thanks to <https://github.com/rikusalminen>.
/// If you want to understand it, draw the shapes out on paper.
void main() {
	vec4 pos = vec4(0.0, 0.0, 0.0, 1.0);

	int axis = gl_VertexID >> 3;

	int crd0 = (axis + 0) % 3;
	int crd1 = (axis + 1) % 3;
	int crd2 = (axis + 2) % 3;

	int minMax0 = (gl_VertexID & 1) >> 0;
	int minMax1 = (gl_VertexID & 2) >> 1;
	int minMax2 = (gl_VertexID & 4) >> 2;

	pos[crd0] = minMax0 * maxPos[crd0] + (minMax0 ^ 1) * minPos[crd0];
	pos[crd1] = minMax1 * maxPos[crd1] + (minMax1 ^ 1) * minPos[crd1];
	pos[crd2] = minMax2 * maxPos[crd2] + (minMax2 ^ 1) * minPos[crd2];

	gl_Position = localToClipMat4 * pos;
}
