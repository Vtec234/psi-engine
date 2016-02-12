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


#map WORLD_TO_CAMERA
uniform mat4 world_to_cam_mat4;
#map CAMERA_TO_CLIP
uniform mat4 cam_to_clip_mat4;

smooth out vec3 str;

/// Outputs a box that is a 1000 units away from the camera.
/// Can be used for a skybox.
/// Use with glDrawArrays(GL_TRIANGLES, 0, 36).
void main() {
	// 3 axes: 0, 1, 2 -> x, y, z
	int axis = ((gl_VertexID - gl_VertexID % 6) / 6) % 3;
		
	// per face one coord stays constant
	// it is either 0 or 1
	int sign = int(gl_VertexID > 17);
	vec3 pos;
	pos[axis] = float(sign);
	
	// other two go like 1 0 0 0 1 1 to draw two triangles counter-clockwise
	//                   0 0 1 1 1 0
	// or for other side 0 0 1 1 1 0
	//                   1 0 0 0 1 1
	pos[(axis + 1 + (sign ^ 1)) % 3] = float((gl_VertexID + 1) % 6 > 2);
	pos[(axis + 1 + (sign ^ 0)) % 3] = float((gl_VertexID + 2) % 6 > 2);

	pos = 2.0 * pos - vec3(1.0);
	str = pos;
	pos *= 1000.0;
	gl_Position = cam_to_clip_mat4 * vec4(mat3(world_to_cam_mat4) * pos, 1.0);
	gl_Position = gl_Position.xyww;
}
