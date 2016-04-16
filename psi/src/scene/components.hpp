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
using ComponentTypeId = uint64_t;
using ComponentTypeIdBitset = uint64_t;

using ComponentHandle = int64_t;

/// A value indicating that no component is referenced.
constexpr ComponentHandle NO_COMPONENT = -1;

struct ComponentRelationship {
	enum class Type {
		/// This component owns the referenced component.
		/// The referenced component can only exist if this component exists.
		/// This component may also not own anything,
		/// in which case the reference is set to NO_COMPONENT.
		OWNERSHIP,
		/// This component references another component.
		/// This component can reference nothing,
		/// in which case the reference is set to NO_COMPONENT.
		/// The existence of this component or the referenced component
		/// does not depend on this reference.
		REFERENCE,
		/// This component references another component,
		/// which is necessary for this component to exist.
		/// This reference may never be set to NO_COMPONENT.
		NECESSARY_REFERENCE,
// if necessary and referenced comp deleted, delete me
// if not necessary and referenced comp deleted, zero the ref
	};
	Type type;

	/// The type of the referenced component.
	ComponentTypeId ref_comp_type;

	/// The byte offset into this component where the reference is contained.
	size_t offset;
};

/// Information about a component type.
struct ComponentTypeInfo {
	/// A unique power-of-2 type id.
	ComponentTypeId type;
	/// The size in bytes of a component of this type.
	size_t size;
	/// Relationship to other components.
	std::array<ComponentRelationship, 32> relations;
	/// A function which takes a pointer to the raw data of the component
	/// and returns a pointer to the component wrapped in boost::any.
	std::function<boost::any(char*)> to_any_f;
	/// A function which takes a pointer to the component component wrapped in boost::any
	/// and returns a pointer to the raw data of the component.
	std::function<char*(boost::any)> to_raw_f;
};
} // namespace psi_scene
