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

#include <string>
#include <cstdint>
#include <functional>
#include <memory>

#include <boost/optional.hpp>


namespace sol {
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

	virtual std::unique_ptr<IResource> copy() const = 0;
};

/// An abstract class representing a read-lock on a resource.
class IResourceLock {
public:
    virtual ~IResourceLock() {};

    virtual IResource const* resource() const = 0;
};

/// A thread-safe service which loads and manages resources.
class IResourceService {
public:
    /// Hashes the given string with the same algorithm with which resource names are hashed.
	/// @return The hash value.
    virtual size_t hash_string(std::string const&) const = 0;
    /// Requests a resource to be loaded.
	/// @return Loading if loading began. Unavailable if file with given name does not exist.
    virtual ResourceState request_resource(std::string const&, std::function<std::unique_ptr<IResource>(std::string const)>) const = 0;
    /// Waits until the resource finishes loading.
	/// @return A read-lock on the resource or empty optional if resource is not currently Loading/Available.
	virtual boost::optional<std::unique_ptr<IResourceLock>> wait_for_resource(size_t) const = 0;
	/// Queries the state of the resource.
	/// @return The queried state.
    virtual ResourceState resource_state(size_t) const = 0;
	/// Frees the resource if it is Available. Stops loading and frees it if it is Loading.
	/// @return Whether the free was successful.
    virtual bool free_resource(size_t) const = 0;
};
} // namespace sol
