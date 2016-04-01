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

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>


namespace psi_rndr {
// TODO redo with quaternions
class Camera {
public:
	Camera();
	~Camera();

	// move the camera on camera space X axis
	void moveX(float mult);
	// move the camera on camera space Y axis
	void moveY(float mult);
	// move the camera on camera space Z axis
	void moveZ(float mult);

	// rotate the pitch of this camera
	void rotatePitch(float mult);
	// rotate the yaw of this camera
	void rotateYaw(float mult);

	void adjustAspectRatio(float aspectRatio);

	// returns the projection matrix [world space -> this camera space]
	glm::mat4 const& worldToCameraMat4();

	glm::vec3 const& cameraPosition() const { return m_cameraPos; }

	glm::mat4 const& cameraToClipMat4() const { return m_cameraToClipMat4; }

private:
	// -- WORLD SPACE TO CAMERA SPACE --
	static constexpr float
		// near and far plane distances
		Z_NEAR = 0.1f,
		Z_FAR = 1000.0f,
		FOV_DEG = 70.0f;

	// no can do constexpr tan(..)
	// formula: 1 / tan(FOV_DEG * (PI * 2 / 360) / 2);
	static const float VIEW_FRUSTUM_SCALE;

	static const glm::vec3
		UP_DIR_VEC;

	// the camera position in Cartesian coordinates [world space]
	glm::vec3 m_cameraPos;

	// the pitch of the camera in degrees [1; 179]
	float m_camPitchDeg;
	// the yaw of the camera in degrees [0; 359]
	float m_camYawDeg;
	// the roll of the camera in degrees [?; ?]
	float m_camRollDeg;

	// three vectors representing the camera space
	glm::vec3 m_camLookDir;
	glm::vec3 m_camUpDir;
	glm::vec3 m_camRightDir;

	// a world space to camera space transformation matrix
	glm::mat4 m_worldToCameraMat4;
	bool m_worldToCameraIsDirty;

	// -- CAMERA SPACE TO CLIP SPACE --
	// a camera space to clip space transformation matrix
	glm::mat4 m_cameraToClipMat4;
};
} // namespace psi_rndr
