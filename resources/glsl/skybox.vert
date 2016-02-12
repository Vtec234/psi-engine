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


#map CAMERA_TO_CLIP
uniform mat4 cam_to_clip_mat4;
#map WORLD_TO_CAMERA
uniform mat4 world_to_cam_mat4;

smooth out vec3 look_dir;

/// Use with glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
/// Idea of mapping from a screen space quad to camera space
/// thanks to people on <##OpenGL@chat.freenode.net>.
void main() {
	// 0 1 0 1
	gl_Position.x = 2.0 * float(gl_VertexID & 1) - 1.0;
	// 0 0 1 1
	gl_Position.y = 2.0 * float((gl_VertexID & 2) >> 1) - 1.0;
	// 1 1 1 1
	gl_Position.z = 1.0;
	gl_Position.w = 1.0;

	// map looking direction from clip space coordinates to world coordinates
	look_dir = vec3(inverse(cam_to_clip_mat4 * mat4(mat3(world_to_cam_mat4))) * gl_Position);
}
