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
#include <experimental/optional>


namespace sol {
class ResourceLock {
public:
    virtual ~ResourceLock() = 0;

    template<typename T>
    T const* resource() {
        return static_cast<T const*>(m_res);
    }

private:
    void const* m_res;
};

/// State that a resource in ResourceService can be in.
enum ResourceState {
    /// Resource is not loading or in storage.
    Unavailable,
    /// Resource is currently loading from the hard disk/other medium.
    Loading,
    /// Resource is in storage and can be used.
    Available,
};

class ResourceService {
public:
    /// Hashes the given string with the same algorithm with which resource names are hashed.
    virtual uint64_t hash_string(std::string const&) const = 0;
    /// Requests a resource to be loaded. Returns Loading if it began loading or Unavailable if the file is not present or invalid. Shouldn't block.
    virtual ResourceState request_resource(std::string const&) = 0;
    /// Blocks until the resource is Available or returns None if resource is not currently Loading/Available.
    virtual std::experimental::optional<ResourceLock> wait_for_resource(uint64_t) const = 0;
    /// Returns the state of the resource.
    virtual ResourceState resource_state(uint64_t) const = 0;
    /// Frees the memory occupied by a given resource. Returns true if it was Loading or Available and can be freed. Otherwise returns false.
    virtual bool free_resource(uint64_t) = 0;
};
} // namespace sol
