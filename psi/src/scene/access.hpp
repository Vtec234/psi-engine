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
	template <typename T>
	T const& read_component(size_t id) {
		return *boost::any_cast<T*>(_component(T::type, id));
	}

	template <typename T>
	T& write_component(size_t id) {
		_mark_component_changed(T::type, id);
		return *boost::any_cast<T*>(_component(T::type, id));
	}

	template <typename T>
	size_t add_component(T comp) {
		return _add_component(T::type, comp);
	}

	template <typename T>
	size_t component_count() {
		return _component_count(T::type);
	}

	template <typename T>
	void mark_component_remove(size_t id) {
		_mark_component_remove(T::type, id);
	}

	template <typename T>
	void cancel_component_removal(size_t id) {
		_cancel_component_removal(T::type, id);
	}

	template <typename T>
	bool component_is_marked_remove(size_t id) {
		return _component_is_marked_remove(T::type, id);
	}

protected:
	/// Obtains a pointer to a component of the requested type.
	/// @param[in] t  component type
	/// @param[in] id component id
	/// @return boost::any containing T*, where T is the component type
	/// @warning Fails on invalid (out of range) component ID or if the component type is not required by the accessing system.
	virtual boost::any _component(ComponentTypeId t, size_t id) = 0;

	/// Marks the specified component as changed in order for it to be synced later.
	/// @param[in] t  component type
	/// @param[in] id component id
	/// @warning Fails on invalid (out of range) component ID or if the component type is not required by the accessing system.
	virtual void _mark_component_changed(ComponentTypeId t, size_t id) = 0;

	/// Adds the given component to the memory of the given type.
	/// Components present at frame beginning are guaranteed to be contiguous in memory, added ones are not.
	/// @param[in] t    component type
	/// @param[in] comp component data : boost::any<T>
	/// @return the id of the added component
	/// @warning Fails if the component type is not required by the accessing system or if the component data is not of the specified type.
	virtual size_t _add_component(ComponentTypeId t, boost::any comp) = 0;

	/// Returns the total number of components of given type available currently.
	/// Includes added components and ones marked for removal.
	/// Last valid id is component_count() - 1.
	/// @param[in] t component type
	/// @return number of components
	/// @warning Fails if the component type is not required by the accessing system.
	virtual size_t _component_count(ComponentTypeId t) = 0;

	/// Marks component as one to be removed. Does nothing if already marked.
	/// Does not actually delete anything until frame ends, even if the marked
	/// component was added in the same frame.
	/// @param[in] t  component type
	/// @param[in] id component id
	/// @warning Fails on invalid (out of range) component ID or if the component type is not required by the accessing system.
	virtual void _mark_component_remove(ComponentTypeId t, size_t id) = 0;

	/// Cancels the removal mark on a given component. Does nothing if not marked.
	/// @param[in] t  component type
	/// @param[in] id component id
	/// @warning Fails on invalid (out of range) component ID or if the component type is not required by the accessing system.
	virtual void _cancel_component_removal(ComponentTypeId t, size_t id) = 0;

	/// Checks if a given component was marked to be removed.
	/// @param[in] t  component type
	/// @param[in] id component id
	/// @return whether component was marked for removal
	/// @warning Fails on invalid (out of range) component ID or if the component type is not required by the accessing system.
	virtual bool _component_is_marked_remove(ComponentTypeId t, size_t id) = 0;
};

// TODO implement ComponentIterator and ComponentConstIterator for convenience
// derive from boost::iterator_facade
class ComponentConstIterator{};

class ComponentIterator{};
} // namespace psi_scene
