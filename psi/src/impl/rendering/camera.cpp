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

#include <glm/detail/func_geometric.hpp>
#include <glm/detail/func_matrix.hpp>


const glm::vec3
	psi_rndr::Camera::UP_DIR_VEC = glm::vec3(0.0f, 1.0f, 0.0f);

const float
	psi_rndr::Camera::VIEW_FRUSTUM_SCALE = 1.0f / tan(FOV_DEG * (float(M_PI) * 2.0f / 360.0f) / 2.0f);

psi_rndr::Camera::Camera()
	: m_cameraPos(0.0f, 0.0f, 5.0f)
	, m_camPitchDeg(90.0f)
	, m_camYawDeg(0.0f)
	, m_camRollDeg(0.0f)
	, m_worldToCameraIsDirty(true) {
	// calculate the GLCameraToClipMatrix base with default aspect ratio of 1.0f
	m_cameraToClipMat4[0].x = VIEW_FRUSTUM_SCALE;
	m_cameraToClipMat4[1].y = VIEW_FRUSTUM_SCALE;
	m_cameraToClipMat4[2].z = (Z_FAR + Z_NEAR) / (Z_NEAR - Z_FAR);
	m_cameraToClipMat4[2].w = -1.0f;
	m_cameraToClipMat4[3].z = (2 * Z_FAR * Z_NEAR) / (Z_NEAR - Z_FAR);
}

psi_rndr::Camera::~Camera() {}

glm::mat4 const& psi_rndr::Camera::worldToCameraMat4() {
	if (m_worldToCameraIsDirty) {
		// -- CARTESIAN Camera SPACE VECTOR IN TERMS OF WORLD SPACE --
		const float degToRadMult = static_cast<float>(M_PI)* 2.0f / 360.0f;

		// the radian angles from degree pitch and yaw
		float theta = m_camPitchDeg * degToRadMult;
		float phi = m_camYawDeg * degToRadMult;

		// trigonometric functions
		float sinTheta = sinf(theta);
		float cosTheta = cosf(theta);
		float sinPhi = sinf(phi);
		float cosPhi = cosf(phi);

		// Cartesian from spherical -> x,y,z from r, pitch, yaw
		m_camLookDir = glm::normalize(glm::vec3(sinTheta * sinPhi, cosTheta, -sinTheta * cosPhi));

		// right and up direction
		m_camRightDir = glm::normalize(glm::cross(m_camLookDir, UP_DIR_VEC));
		m_camUpDir = glm::cross(m_camRightDir, m_camLookDir);

		// -- WORLD SPACE TO Camera SPACE PROJECTION MATRIX --
		// rotation matrix
		glm::mat4 rotationMat4(1.0f);
		rotationMat4[0] = glm::vec4(m_camRightDir, 0.0f);
		rotationMat4[1] = glm::vec4(m_camUpDir, 0.0f);
		rotationMat4[2] = glm::vec4(-m_camLookDir, 0.0f);
		rotationMat4 = glm::transpose(rotationMat4);

		// translation matrix
		glm::mat4 translationMat4(1.0f);
		translationMat4[3] = glm::vec4(-m_cameraPos, 1.0f);
		m_worldToCameraIsDirty = true;
		// projection matrix
		m_worldToCameraMat4 = rotationMat4 * translationMat4;

		m_worldToCameraIsDirty = false;
	}

	return m_worldToCameraMat4;
}

void psi_rndr::Camera::adjustAspectRatio(float aspectRatio) {
	m_cameraToClipMat4[0].x = VIEW_FRUSTUM_SCALE / aspectRatio;
}

void psi_rndr::Camera::moveX(float mult) {
	m_cameraPos = m_cameraPos + mult * m_camRightDir;

	m_worldToCameraIsDirty = true;
}

void psi_rndr::Camera::moveY(float mult) {
	m_cameraPos = m_cameraPos + mult * UP_DIR_VEC;

	m_worldToCameraIsDirty = true;
}

void psi_rndr::Camera::moveZ(float mult) {
	m_cameraPos = m_cameraPos + mult * m_camLookDir;

	m_worldToCameraIsDirty = true;
}

float psi_rndr::Camera::rotatePitch(float mult) {
	m_camPitchDeg += mult;

	m_camPitchDeg = glm::clamp(m_camPitchDeg, 1.0f, 179.0f);

	m_worldToCameraIsDirty = true;

	return m_camPitchDeg;
}

float psi_rndr::Camera::rotateYaw(float mult) {
	m_camYawDeg += mult;

	if (m_camYawDeg < 0.0f)
		m_camYawDeg = 359.0f;
	else if (m_camYawDeg > 359.0f)
		m_camYawDeg = 0.0f;

	m_worldToCameraIsDirty = true;

	return m_camYawDeg;
}
