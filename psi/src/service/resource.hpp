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

#include <string>
#include <cstdint>
#include <functional>
#include <memory>

#include <boost/optional.hpp>


namespace psi_serv {
/// State that a resource in ResourceService can be in.
enum ResourceState {
    /// Resource is not loading or in storage.
    Unavailable,
    /// Resource is currently loading from the hard disk/other medium.
    Loading,
    /// Resource is in storage and can be used.
    Available,
};

/// An abstract class representing a resource.
class IResource {
public:
	virtual ~IResource() {};

	virtual std::unique_ptr<IResource> clone() const = 0;
};

/// An abstract class representing a read-lock on a resource.
/// Destroying this object releases the lock and allows potential modifications to the resource.
class IResourceLock {
public:
    virtual ~IResourceLock() {};

    virtual IResource const& resource() const = 0;
};

/// A thread-safe service which loads and manages resources.
class IResourceService {
public:
    /// Requests a resource to be loaded.
	/// @throw When input is invalid or loading is otherwise prevented.
	/// @return Loading if loading began. Available if index is already in use.
	virtual ResourceState request_resource(size_t, std::function<std::unique_ptr<IResource>()>) const = 0;

    /// Waits until the resource finishes loading.
	/// @return A read-lock on the resource or empty optional if resource is not currently Loading/Available.
	virtual boost::optional<std::unique_ptr<IResourceLock>> wait_for_resource(size_t) const = 0;

	/// Queries the state of the resource.
	/// @return The queried state.
    virtual ResourceState resource_state(size_t) const = 0;

	/// Frees the resource if it is Available. Stops loading and frees it if it is Loading. Blocks.
	/// @return What the state was before freeing.
    virtual ResourceState free_resource(size_t) const = 0;
};
} // namespace psi_serv
