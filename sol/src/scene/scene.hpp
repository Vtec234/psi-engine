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

constexpr int64_t NO_COMPONENT = -1;

/// The basic scene component, representing a single entity.
/// Its fields are indices of other components which this entity comprises of,
/// where a value of NO_COMPONENT indicates that the entity contains no such
/// component. This component is special in the sense that removing it will
/// also remove all components which it references, unless these components
/// are also referenced by other entities.
struct SceneComponentEntity {
	int64_t transform = NO_COMPONENT;
	int64_t model = NO_COMPONENT;

	bool experiences_causality = false;
};

/// A component representing the position, scale, and orientation of an entity
/// in world space. This component is required for all entities which physically
/// exist in the world.
struct SceneComponentTransform {
	int64_t parent = NO_COMPONENT;

	std::array<float, 3> pos;
	std::array<float, 3> scale;
	std::array<float, 4> orientation;
};

/// A component representing the resources needed to render the entity.
struct SceneComponentModel {
	typedef char32_t UChar32;
	/// Constant size 128-byte arrays containing UTF-32 resource names.
	std::array<UChar32, 128> mesh_name;
	std::array<UChar32, 128> mat_name;
	std::array<UChar32, 128> shader_name;
};

/// Provides direct access to the scene. The scene contains only the components required by the system.
class ISceneDirectAccess {
public:
	/// Obtains a read-only constant reference to a component of the requested type.
	/// @param[in] t  component type
	/// @param[in] id component id
	/// @return pointer to component
	/// @warning Fails on invalid (out of range) component ID or if the component type is not required by the accessing system.
	virtual boost::any const read_component(SceneComponentType t, size_t id) = 0;

	/// Obtains a writeable reference to a component of the requested type.
	/// Marks the component as changed.
	/// @param[in] t  component type
	/// @param[in] id component id
	/// @return pointer to component
	/// @warning Fails on invalid (out of range) component ID or if the component type is not required by the accessing system.
	virtual boost::any write_component(SceneComponentType t, size_t id) = 0;

	/// Adds the given component to the memory of the given type.
	/// Components present at frame beginning are guaranteed to be contiguous in memory, added ones are not.
	/// @param[in] t    component type
	/// @param[in] comp component data : boost::any<T>
	/// @return the id of the added component
	/// @warning Fails if the component type is not required by the accessing system or if the component data is not of the specified type.
	virtual size_t add_component(SceneComponentType t, boost::any comp) = 0;

	/// Returns the total number of components of given type available currently.
	/// Includes added components and ones marked for removal.
	/// Last valid id is component_count() - 1.
	/// @param[in] t component type
	/// @return number of components
	/// @warning Fails if the component type is not required by the accessing system.
	virtual size_t component_count(SceneComponentType t) = 0;

	/// Marks component as one to be removed. Does nothing if already marked.
	/// Does not actually delete anything until frame ends, even if the marked
	/// component was added in the same frame.
	/// @param[in] t  component type
	/// @param[in] id component id
	/// @warning Fails on invalid (out of range) component ID or if the component type is not required by the accessing system.
	virtual void mark_component_remove(SceneComponentType t, size_t id) = 0;

	/// Cancels the removal mark on a given component. Does nothing if not marked.
	/// @param[in] t  component type
	/// @param[in] id component id
	/// @warning Fails on invalid (out of range) component ID or if the component type is not required by the accessing system.
	virtual void cancel_component_removal(SceneComponentType t, size_t id) = 0;

	/// Checks if a given component was marked to be removed.
	/// @param[in] t  component type
	/// @param[in] id component id
	/// @return whether component was marked for removal
	/// @warning Fails on invalid (out of range) component ID or if the component type is not required by the accessing system.
	virtual bool component_is_marked_remove(SceneComponentType t, size_t id) = 0;
};

// TODO implement SceneComponentIterator and SceneComponentConstIterator for convenience
// derive from boost::iterator_facade
class SceneComponentConstIterator{};

class SceneComponentIterator{};
} // namespace sol
