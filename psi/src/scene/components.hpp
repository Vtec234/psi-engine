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

#include <cstdint>
#include <array>
#include <initializer_list>

#include <unicode/umachine.h>


namespace psi_scene {
/// The type of scene component.
enum class ComponentType : uint64_t {
	ENTITY = 0b1,
	TRANSFORM = 0b10,
	MODEL = 0b100,
};

/// A value indicating that no component is referenced.
constexpr int64_t NO_COMPONENT = -1;

/// Base struct which all components inherit from.
/// Used to indicate the component type.
template <enum ComponentType T>
struct Component { ComponentType type = T; };

/// The basic scene component, representing a single entity.
/// Its fields are indices of other components which this entity comprises of,
/// where a value of NO_COMPONENT indicates that the entity contains no such
/// component. This component is special in the sense that removing it will
/// also remove all components which it references, unless these components
/// are also referenced by other entities.
struct ComponentEntity /*: Component<ComponentType::ENTITY>*/ {
	int64_t transform = NO_COMPONENT;
	int64_t model = NO_COMPONENT;

	bool experiences_causality = false;
};


/// A component representing the position, scale, and orientation of an entity
/// in world space. This component is required for all entities which physically
/// exist in the world.
struct ComponentTransform /*: Component<ComponentType::TRANSFORM>*/ {
	int64_t parent = NO_COMPONENT;

	std::array<float, 3> pos;
	std::array<float, 3> scale;
	std::array<float, 4> orientation;
};

/// A component representing the resources needed to render the entity.
struct ComponentModel /*: Component<ComponentType::MODEL>*/ {
	// TODO ICU's typedef int32_t UChar32 is incompatible with U"string" from C++1z, as it returns char32_t[]
	typedef char32_t UChar32;
	/// Constant size 128-byte arrays containing UTF-32 resource names.
	std::array<UChar32, 128> mesh_name;
	std::array<UChar32, 128> mat_name;
	std::array<UChar32, 128> shader_name;
};
} // namespace psi_scene
