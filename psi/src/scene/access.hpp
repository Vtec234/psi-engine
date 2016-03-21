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

#include <boost/any.hpp>

#include "../marker/thread_safety.hpp"
#include "components.hpp"


namespace psi_scene {
/// Provides direct access to the scene. The scene contains only the components required by the system.
class ISceneDirectAccess : psi_mark::NonThreadsafe {
public:
	/// Obtains a read-only constant reference to a component of the requested type.
	/// @param[in] t  component type
	/// @param[in] id component id
	/// @return pointer to component
	/// @warning Fails on invalid (out of range) component ID or if the component type is not required by the accessing system.
	virtual boost::any const read_component(ComponentType t, size_t id) = 0;

	/// Obtains a writeable reference to a component of the requested type.
	/// Marks the component as changed.
	/// @param[in] t  component type
	/// @param[in] id component id
	/// @return pointer to component
	/// @warning Fails on invalid (out of range) component ID or if the component type is not required by the accessing system.
	virtual boost::any write_component(ComponentType t, size_t id) = 0;

	/// Adds the given component to the memory of the given type.
	/// Components present at frame beginning are guaranteed to be contiguous in memory, added ones are not.
	/// @param[in] t    component type
	/// @param[in] comp component data : boost::any<T>
	/// @return the id of the added component
	/// @warning Fails if the component type is not required by the accessing system or if the component data is not of the specified type.
	virtual size_t add_component(ComponentType t, boost::any comp) = 0;

	/// Returns the total number of components of given type available currently.
	/// Includes added components and ones marked for removal.
	/// Last valid id is component_count() - 1.
	/// @param[in] t component type
	/// @return number of components
	/// @warning Fails if the component type is not required by the accessing system.
	virtual size_t component_count(ComponentType t) = 0;

	/// Marks component as one to be removed. Does nothing if already marked.
	/// Does not actually delete anything until frame ends, even if the marked
	/// component was added in the same frame.
	/// @param[in] t  component type
	/// @param[in] id component id
	/// @warning Fails on invalid (out of range) component ID or if the component type is not required by the accessing system.
	virtual void mark_component_remove(ComponentType t, size_t id) = 0;

	/// Cancels the removal mark on a given component. Does nothing if not marked.
	/// @param[in] t  component type
	/// @param[in] id component id
	/// @warning Fails on invalid (out of range) component ID or if the component type is not required by the accessing system.
	virtual void cancel_component_removal(ComponentType t, size_t id) = 0;

	/// Checks if a given component was marked to be removed.
	/// @param[in] t  component type
	/// @param[in] id component id
	/// @return whether component was marked for removal
	/// @warning Fails on invalid (out of range) component ID or if the component type is not required by the accessing system.
	virtual bool component_is_marked_remove(ComponentType t, size_t id) = 0;
};

// TODO implement ComponentIterator and ComponentConstIterator for convenience
// derive from boost::iterator_facade
class ComponentConstIterator{};

class ComponentIterator{};
} // namespace psi_scene
