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
#include <functional>

#include <boost/any.hpp>


namespace psi_scene {
using ComponentType = uint64_t;
/// TypeA | TypeB | TypeC
using ComponentTypeBitset = uint64_t;
using ComponentId = int64_t;

/// A value indicating that no component is referenced.
constexpr ComponentId NO_COMPONENT = -1;

/// A component type info piece specifying that a component of this type represents some other component.
struct ComponentRef {
	/// The type of the referenced component.
	ComponentType type;
	/// The byte offset into the referencing component where the reference is contained.
	size_t offset;
	/// Whether a valid reference is necessary for the referencing component to exist.
	bool necessary;
};
// if necessary and referenced comp deleted, delete me
// if not necessary and referenced comp deleted, zero the ref

/// A component type info piece specifying that a component of this type owns another component.
struct ComponentOwnership {
	/// The type of the owned component.
	ComponentType type;
	/// The byte offset into the owner where the owned component id is contained.
	size_t offset;
};

/// Information about a component type.
struct ComponentTypeInfo {
	/// A unique power-of-2 id.
	ComponentId id;
	/// The byte size of a component of this type.
	size_t size;
	/// References to other components.
	std::array<ComponentRef, 16> refs;
	/// Ownership of other components.
	std::array<ComponentOwnership, 16> owns;
	/// A function which takes a byte array and returns a boost::any containing T*, where T is the component type.
	std::function<boost::any(char*)> to_any_f;
	/// A function which takes a const byte array and returns a boost::any const containing a T const*, where T is the component type.
	std::function<boost::any const(char const*)> to_const_any_f;
	/// A function which takes a boost::any and returns the contained data as a byte array if the contained type is T, where T is the component type.
	std::function<char*(boost::any*)> to_data_f;
};
} // namespace psi_scene
