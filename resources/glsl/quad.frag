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


smooth in vec3 coord_clip;

#map POSITION_FRAME_TEXTURE_SAMPLER
uniform sampler2D pos_tex;
#map NORMAL_FRAME_TEXTURE_SAMPLER
uniform sampler2D norm_tex;
#map ALBEDO_FRAME_TEXTURE_SAMPLER
uniform sampler2D albedo_tex;
#map REFL_ROUGH_FRAME_TEXTURE_SAMPLER
uniform sampler2D refl_rough_tex;

out vec4 fragColor;

void main() {
	if (coord_clip.x < 0.5) {
		if (coord_clip.y < 0.5) {
			fragColor = texture2D(pos_tex, coord_clip.xy * 2.0).rgba;
		}
		else {
			fragColor = texture2D(norm_tex, vec2(coord_clip.x * 2.0, (coord_clip.y - 0.5) * 2.0)).rgba;
		}
	}
	else {
		if (coord_clip.y < 0.5) {
			fragColor = texture2D(albedo_tex, vec2((coord_clip.x - 0.5) * 2.0, coord_clip.y * 2.0)).rgba;
		}
		else {
			fragColor = texture2D(refl_rough_tex, (coord_clip.xy - 0.5) * 2.0).rgba;
		}
	}
}
