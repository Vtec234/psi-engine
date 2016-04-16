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
/// A component representing the position, scale, and orientation of an entity
/// in world space. This component is required for all entities which physically
/// exist in the world.
struct ComponentTransform {
	static constexpr ComponentTypeId type = 0b10;

	ComponentHandle parent = NO_COMPONENT;

	std::array<float, 3> pos;
	std::array<float, 3> scale;
	std::array<float, 4> orientation;
};

static ComponentTypeInfo component_type_transform_info = {
	ComponentTransform::type,
	sizeof(ComponentTransform),
	{{
		{
			ComponentRelationship::Type::REFERENCE,
			ComponentTransform::type,
			0,
		},
	}},
	[] (char* p) -> boost::any {
		return reinterpret_cast<ComponentTransform*>(p);
	},
	[] (boost::any a) -> char* {
		return reinterpret_cast<char*>(boost::any_cast<ComponentTransform*>(a));
	},
};

/// A component representing the resources needed to render the entity.
struct ComponentModel {
	static constexpr ComponentTypeId type = 0b100;

	/// UTF-8
	std::array<char, 512> mesh_name;
	std::array<char, 512> albedo_tex;
	std::array<char, 512> reflectiveness_roughness_tex;
	std::array<char, 512> normal_tex;
};

static ComponentTypeInfo component_type_model_info = {
	ComponentModel::type,
	sizeof(ComponentModel),
	{{}},
	[] (char* p) -> boost::any {
		return reinterpret_cast<ComponentModel*>(p);
	},
	[] (boost::any a) -> char* {
		return reinterpret_cast<char*>(boost::any_cast<ComponentModel*>(a));
	},
};

/// The basic scene component, representing a single entity.
/// Its fields are indices of other components which this entity comprises of,
/// where a value of NO_COMPONENT indicates that the entity contains no such
/// component. This component is special in the sense that removing it will
/// also remove all components which it references, unless these components
/// are also referenced by other entities.
struct ComponentEntity {
	static constexpr ComponentTypeId type = 0b1;

	ComponentHandle transform = NO_COMPONENT;
	ComponentHandle model = NO_COMPONENT;

	bool experiences_causality = 0;
};

static ComponentTypeInfo component_type_entity_info = {
	ComponentEntity::type,
	sizeof(ComponentEntity),
	{{
		{
			ComponentRelationship::Type::OWNERSHIP,
			ComponentTransform::type,
			0,
		},
		{
			ComponentRelationship::Type::OWNERSHIP,
			ComponentModel::type,
			sizeof(ComponentHandle),
		},
	}},
	[] (char* p) -> boost::any {
		return reinterpret_cast<ComponentEntity*>(p);
	},
	[] (boost::any a) -> char* {
		return reinterpret_cast<char*>(boost::any_cast<ComponentEntity*>(a));
	},
};
} // namespace psi_scene
