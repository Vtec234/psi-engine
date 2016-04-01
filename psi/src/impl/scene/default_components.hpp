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

#include "../../scene/components.hpp"


namespace psi_scene {
/// The basic scene component, representing a single entity.
/// Its fields are indices of other components which this entity comprises of,
/// where a value of NO_COMPONENT indicates that the entity contains no such
/// component. This component is special in the sense that removing it will
/// also remove all components which it references, unless these components
/// are also referenced by other entities.
struct ComponentEntity {
	ComponentId transform = NO_COMPONENT;
	ComponentId model = NO_COMPONENT;

	bool experiences_causality = false;
};

static ComponentTypeInfo component_type_entity_info = {
	0b1,
	sizeof(ComponentEntity),
	{{}},
	// TODO check this
	{{{0b10, 0}, {0b100, sizeof(ComponentId)}}},
	[](char* p)->boost::any{ return reinterpret_cast<ComponentEntity*>(p); },
	[](char const* p)->boost::any{ return reinterpret_cast<ComponentEntity const*>(p); },
	[](boost::any* a)->char* { return reinterpret_cast<char*>(boost::any_cast<ComponentEntity>(a)); },
};

/// A component representing the position, scale, and orientation of an entity
/// in world space. This component is required for all entities which physically
/// exist in the world.
struct ComponentTransform {
	ComponentId parent = NO_COMPONENT;

	std::array<float, 3> pos;
	std::array<float, 3> scale;
	std::array<float, 4> orientation;
};

static ComponentTypeInfo component_type_transform_info = {
	0b10,
	sizeof(ComponentTransform),
	{{{0b10, 0, false}}},
	{{}},
	[](char* p)->boost::any{ return reinterpret_cast<ComponentTransform*>(p); },
	[](char const* p)->boost::any{ return reinterpret_cast<ComponentTransform const*>(p); },
	[](boost::any* a)->char* { return reinterpret_cast<char*>(boost::any_cast<ComponentTransform>(a)); },
};

/// A component representing the resources needed to render the entity.
struct ComponentModel {
	/// UTF-8
	std::array<char, 512> mesh_name;
	std::array<char, 512> mat_name;
};

static ComponentTypeInfo component_type_model_info = {
	0b100,
	sizeof(ComponentModel),
	{{}},
	{{}},
	[](char* p)->boost::any{ return reinterpret_cast<ComponentModel*>(p); },
	[](char const* p)->boost::any{ return reinterpret_cast<ComponentModel const*>(p); },
	[](boost::any* a)->char* { return reinterpret_cast<char*>(boost::any_cast<ComponentModel>(a)); },
};
} // namespace psi_scene
