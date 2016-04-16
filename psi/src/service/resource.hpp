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
#include <boost/any.hpp>

#include "../marker/thread_safety.hpp"


namespace psi_serv {
/// State that a resource in ResourceService can be in.
enum class ResourceState {
	/// Resource is not loading or in storage.
	UNAVAILABLE,
	/// Resource is currently loading from the hard disk/other medium.
	LOADING,
	/// Resource is in storage and can be used.
	AVAILABLE,
};

/// An abstract class representing a read-lock on a resource.
/// Destroying this object releases the lock and allows potential modifications to the resource.
class IResourceLock : psi_mark::NonThreadsafe {
public:
	virtual ~IResourceLock() {};

	virtual boost::any const& resource() = 0;
};

/// A thread-safe service which loads and manages resources.
class IResourceService : psi_mark::ConstThreadsafe {
public:
	using ResourceLoaderId = uint64_t;

	/// Resource handles are global to the resource service, not per-resource-type.
	using ResourceHandle = size_t;

	/// Registers a loader function which can then be used to load resources.
	/// Replaces previous loader if this id was already registered.
	/// @param[in] id     resource loader id
	/// @param[in] loader a thread-safe loader function
	virtual void register_loader(ResourceLoaderId id, std::function<boost::any(std::string const&)> loader) = 0;

	/// Requests a resource to be loaded with the specified loader.
	/// @param[in] h     resource storage handle to load into
	/// @param[in] id    resource loader id
	/// @param[in] param UTF-8 string parameter passed to the loader
	/// @warning Assertion failure on unregistered type.
	virtual ResourceState request_resource(ResourceHandle h, ResourceLoaderId id, std::string param) const = 0;

	/// Requests a resource to be loaded using the given function.
	/// @param[in] h resource storage handle to load into
	/// @param[in] f a thread-safe loader function
	/// @throw When input is invalid or loading is otherwise prevented.
	/// @return Loading if loading began. Available if index is already in use.
	virtual ResourceState request_resource(ResourceHandle h, std::function<boost::any()> f) const = 0;

	/// Tries to retrieve the resource with the specified handle.
	/// Waits until the resource finishes loading if it is.
	/// @param[in] h storage handle of the requested resource
	/// @return A read-lock on the resource or empty optional if resource is not currently Loading/Available.
	virtual boost::optional<std::unique_ptr<IResourceLock>> retrieve_resource(ResourceHandle h) const = 0;

	/// Queries the state of the resource.
	/// @param[in] h storage handle of the queried resource
	/// @return The queried state.
	virtual ResourceState resource_state(ResourceHandle h) const = 0;

	/// Frees the resource if it is Available. Stops loading and frees it if it is Loading. Blocks.
	/// @param[in] h storage handle of the freed resource
	/// @return What the state was before freeing.
	virtual ResourceState free_resource(ResourceHandle h) const = 0;
};
} // namespace psi_serv
