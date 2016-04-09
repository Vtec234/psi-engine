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

smooth in vec2 uv;

#map POSITION_FRAME_TEXTURE_SAMPLER
uniform sampler2D pos_tex;
#map NORMAL_FRAME_TEXTURE_SAMPLER
uniform sampler2D norm_tex;
#map ALBEDO_FRAME_TEXTURE_SAMPLER
uniform sampler2D albedo_tex;
#map REFL_ROUGH_FRAME_TEXTURE_SAMPLER
uniform sampler2D refl_rough_tex;

out vec4 frag_color;

void main() {
	if (uv.x < 0.5) {
		if (uv.y < 0.5) {
			frag_color = texture2D(albedo_tex, uv * 2.0);
		}
		else {
			frag_color = texture2D(pos_tex, vec2(uv.x, uv.y - 0.5) * 2.0);
		}
	}
	else {
		if (uv.y < 0.5) {
			frag_color = texture2D(refl_rough_tex, vec2(uv.x - 0.5, uv.y) * 2.0);
		}
		else {
			frag_color = texture2D(norm_tex, (uv - 0.5) * 2.0);
		}
	}
}
