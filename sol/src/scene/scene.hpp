/*
 * Copyright (C) 2015-2016 Wojciech Nawrocki
 *
 * This file is part of the Sol Library.
 *
 * The Sol Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Sol Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Sol Library. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <cstdint>
#include <array>
#include <memory>

#include <boost/any.hpp>

#include <unicode/umachine.h>

#include "system.hpp"


namespace sol {
/// The type of struct which can be a scene component.
enum SceneComponentType {
	ENTITY = 0b1,
	TRANSFORM = 0b10,
	MODEL = 0b100,
};

/// The basic scene component, representing a single entity.
/// Its fields are indices of other components which this entity comprises of,
/// where a value of NO_COMPONENT indicates that the entity contains no such
/// component. This component is special in the sense that removing it will
/// also remove all components which it references, unless these components
/// are also referenced by other entities.
struct SceneComponentEntity {
	const int64_t NO_COMPONENT = -1;

	int64_t transform = NO_COMPONENT;
	int64_t model = NO_COMPONENT;

	bool experiences_causality = false;
};

/// A component representing the position, scale, and orientation of an entity
/// in world space. This component is required for all entities which physically
/// exist in the world.
struct SceneComponentTransform {
	int64_t parent;
	std::array<float, 3> pos;
	std::array<float, 3> scale;
	std::array<float, 4> orientation;
};

/// A component representing the resources needed to render the entity.
struct SceneComponentModel {
	/// Constant size 128-byte arrays containing UTF-32 resource names.
	std::array<UChar32, 128> mesh_name;
	std::array<UChar32, 128> mat_name;
	std::array<UChar32, 128> shader_name;
};

/// Provides direct access to the scene. The scene contains only the components required by the system.
class ISceneDirectAccess {
// TODO improve the docs
public:
	/// Returns a const& to a component of the requested type.
	/// Assertion failure on type that is not required by the system or out-of-range id.
	virtual boost::any const& read_component(SceneComponentType, size_t id) = 0;
	
	/// Returns a & to a component of the requested type and marks it as changed.
	/// Assertion failure on type that is not required by the system or out-of-range id.
	virtual boost::any& write_component(SceneComponentType, size_t id) = 0;

	/// Returns the total number of components which were present at frame beginning.
	/// Does not include added ones, but includes ones marked for removal.
	virtual size_t component_count(SceneComponentType) = 0;

	/// Adds the given component to the given type, may allocate memory.
	/// Assertion failure on type that is not required by the system.
	/// Returns the id of the added component.
	/// Components present at frame beginning are guaranteed to be contiguous in memory, added ones are not.
	virtual size_t add_component(SceneComponentType, boost::any comp) = 0;

	/// Returns a & to an added component of the requested type.
	/// Assertion failure on type that is not required by the system or out-of-range id.
	virtual boost::any& added_component(SceneComponentType, size_t) = 0;

	/// Returns the total number of components added after frame began.
	/// Assertion failure on type that is not required by the system.
	virtual size_t added_component_count(SceneComponentType) = 0;

	/// Marks component as one to be removed or cancels addition if the component was added after frame began.
	/// Assertion failure on type that is not required by the system or out-of-range id.
	virtual void mark_component_remove(SceneComponentType, size_t) = 0;

	/// Checks if component was marked to be removed.
	/// Assertion failure on type that is not required by the system or out-of-range id.
	virtual bool component_is_marked_remove(SceneComponentType, size_t id) = 0;
};

// TODO implement SceneComponentIterator and SceneComponentConstIterator for convenience
// derive from boost::iterator_facade
class SceneComponentConstIterator{};

class SceneComponentIterator{};
} // namespace sol
