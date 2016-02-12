/*
 * Copyright (C) 2015-2016 Wojciech Nawrocki
 *
 * This file is part of Generic Shooter Game.
 *
 * Generic Shooter Game is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Generic Shooter Game is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Generic Shooter Game. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <mutex>
#include <atomic>
#include <condition_variable>

#include <tbb/concurrent_hash_map.h>


#include <task/submitter.hpp>
#include <service/resource.hpp>


namespace gsg {
/// Arguments required to start the ResourceLoader service.
struct ResourceLoaderArgs {
	sol::ITaskSubmitter* task_submitter;
};

class ResourceLoader : public sol::IResourceService {
	class ResourceStorage {
	public:
		ResourceStorage();
		~ResourceStorage();

		/// ResourceStorage must be CopyConstructible for concurrent_hash_map.
		ResourceStorage(ResourceStorage const&);

		/// Store the result of a successful load operation in a newly initialized ResourceStorage.
		void store_load(std::unique_ptr<sol::IResource>);
		/// Check if it is still loading == nothing was stored.
		bool is_loaded() const;
		/// Wait until something gets stored.
		void wait_for_load() const;
		/// Retrieve the state of this resource.
		sol::ResourceState state() const;
		/// Returns the stored resource if it was loaded. Assertion fails otherwise.
		sol::IResource const* resource() const;

	private:
		bool m_is_loaded;
		mutable std::mutex m_load_mut;
		mutable std::condition_variable m_load_cond;

		std::unique_ptr<sol::IResource> m_res;
	};

	typedef tbb::concurrent_hash_map<size_t, ResourceStorage>::accessor accessor;
	typedef tbb::concurrent_hash_map<size_t, ResourceStorage>::const_accessor const_accessor;

	class ResourceLock : public sol::IResourceLock {
	public:
		explicit ResourceLock(std::unique_ptr<const_accessor>&&);
		virtual ~ResourceLock();
		sol::IResource const& resource() const override;

	private:
		std::unique_ptr<const_accessor> m_access;
	};

public:
	static std::unique_ptr<sol::IResourceService> start_resource_loader(ResourceLoaderArgs);

	sol::ResourceState request_resource(size_t, std::function<std::unique_ptr<sol::IResource>()>) const override;

	boost::optional<std::unique_ptr<sol::IResourceLock>> wait_for_resource(size_t) const override;

	sol::ResourceState resource_state(size_t) const override;

	sol::ResourceState free_resource(size_t) const override;

private:
	ResourceLoader(sol::ITaskSubmitter*);

	mutable tbb::concurrent_hash_map<size_t, ResourceStorage> m_resources;
	sol::ITaskSubmitter const* m_task_submitter;
};
} // namespace gsg
