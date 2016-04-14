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

#include <eigen3/Eigen/Geometry>


namespace psi_rndr {
class IsometricTransform {
public:
	void translate_in_local(Eigen::Vector3f const&);
	void translate_in_world(Eigen::Vector3f const&);

	void rotate_in_local(Eigen::Vector3f const& axis, float angle_deg);
	void rotate_in_world(Eigen::Vector3f const& axis, float angle_deg);

	Eigen::Vector3f const& position();

	Eigen::Matrix4f const& world_to_local();
	Eigen::Matrix4f const& local_to_world();

private:
	Eigen::Vector3f _position = {0.0f, 0.0f, 0.0f};
	Eigen::Quaternionf _orientation = {1.0f, 0.0f, 0.0f, 0.0f};

	Eigen::Matrix4f _world_to_local;
	bool _world_to_local_is_dirty = true;

	Eigen::Matrix4f _local_to_world;
	bool _local_to_world_is_dirty = true;
};

class ClipMatrix {
public:
	ClipMatrix();
	~ClipMatrix() = default;

	void adjust_aspect_ratio(float);
	void adjust_fov(float);

	Eigen::Matrix4f const& to_clip();

private:
	Eigen::Matrix4f _the_matrix;
	float _view_frustum_scale;
};
} // namespace psi_rndr
