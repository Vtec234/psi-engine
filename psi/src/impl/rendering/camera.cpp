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

#include "camera.hpp"


void psi_rndr::IsometricTransform::translate_in_local(Eigen::Vector3f const& v) {
	m_position += m_orientation._transformVector(v);
	m_world_to_local_is_dirty = true;
	m_local_to_world_is_dirty = true;
}

void psi_rndr::IsometricTransform::translate_in_world(Eigen::Vector3f const& v) {
	m_position += v;
	m_world_to_local_is_dirty = true;
	m_local_to_world_is_dirty = true;
}

void psi_rndr::IsometricTransform::rotate_in_local(Eigen::Vector3f const& axis, float angle_deg) {
	float half_angle_rad = angle_deg * float(M_PI) / 360.0f;
	auto sax = m_orientation._transformVector(axis) * sinf(half_angle_rad);
	auto c = cosf(half_angle_rad);
	Eigen::Quaternionf rot(c, sax.x(), sax.y(), sax.z());
	m_orientation = rot * m_orientation;
	m_world_to_local_is_dirty = true;
	m_local_to_world_is_dirty = true;
}

void psi_rndr::IsometricTransform::rotate_in_world(Eigen::Vector3f const& axis, float angle_deg) {
	float half_angle_rad = angle_deg * float(M_PI) / 360.0f;
	auto sax = axis * sinf(half_angle_rad);
	auto c = cosf(half_angle_rad);
	Eigen::Quaternionf rot(c, sax.x(), sax.y(), sax.z());
	m_orientation = rot * m_orientation;
	m_world_to_local_is_dirty = true;
	m_local_to_world_is_dirty = true;
}

Eigen::Vector3f const& psi_rndr::IsometricTransform::position() {
	return m_position;
}

Eigen::Matrix4f const& psi_rndr::IsometricTransform::world_to_local() {
	if (m_world_to_local_is_dirty) {
		m_world_to_local = local_to_world().inverse();

		m_world_to_local_is_dirty = false;
	}

	return m_world_to_local;
}

Eigen::Matrix4f const& psi_rndr::IsometricTransform::local_to_world() {
	if (m_local_to_world_is_dirty) {
		Eigen::Matrix4f translation;
		translation << 1, 0, 0, 0,
					   0, 1, 0, 0,
					   0, 0, 1, 0,
					   0, 0, 0, 1;

		Eigen::Matrix4f rotation = translation;

		translation.col(3) = Eigen::Vector4f(m_position.x(), m_position.y(), m_position.z(), 1.0f);

		rotation.topLeftCorner<3,3>() = m_orientation.matrix();
		m_local_to_world = translation * rotation;

		m_local_to_world_is_dirty = false;
	}

	return m_local_to_world;
}

psi_rndr::ClipMatrix::ClipMatrix() {
	constexpr float
		Z_NEAR = 0.1f,
		Z_FAR = 1000.0f,
		DEFAULT_FOV_DEG = 75.0f;

	m_view_frustum_scale = 1.0f / tan(DEFAULT_FOV_DEG * float(M_PI) / 360.0f);

	m_the_matrix << m_view_frustum_scale, 0,                    0,                                   0,
					0,                    m_view_frustum_scale, 0,                                   0,
					0,                    0,                    (Z_FAR + Z_NEAR) / (Z_NEAR - Z_FAR), (2 * Z_FAR * Z_NEAR) / (Z_NEAR - Z_FAR),
					0,                    0,                    -1.0f,                               1;
}

void psi_rndr::ClipMatrix::adjust_aspect_ratio(float ratio) {
	m_the_matrix(0,0) = m_view_frustum_scale / ratio;
}

void psi_rndr::ClipMatrix::adjust_fov(float fov) {
	float ratio = m_view_frustum_scale / m_the_matrix(0,0);
	m_view_frustum_scale = 1.0f / tan(fov * float(M_PI) / 360.0f);
	m_the_matrix(0,0) = m_view_frustum_scale / ratio;
}

Eigen::Matrix4f const& psi_rndr::ClipMatrix::to_clip() {
	return m_the_matrix;
}
