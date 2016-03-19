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

#include "resource.hpp"

#include <algorithm>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include <tbb/concurrent_hash_map.h>

#include "../../log/log.hpp"
#include "../../util/assert.hpp"


class ResourceStorage {
public:
	ResourceStorage()
		: m_is_loaded(false) {}

	/// ResourceStorage must be CopyConstructible for concurrent_hash_map.
	ResourceStorage(ResourceStorage const& cpy)
		: m_is_loaded(cpy.m_is_loaded) {
		// make deep copy of resource if it was loaded into cpy
		if (cpy.m_res) {
			m_res = cpy.m_res->clone();
		}
	}

	~ResourceStorage() {
		// simulate successful load and notify everybody we're done
		// TODO won't this segfault if waiters check the Pred after m_is_loaded was destroyed?
		std::unique_lock<std::mutex> lock(m_load_mut);
		m_is_loaded = true;
		lock.unlock();
		m_load_cond.notify_all();
	}

	/// Store the result of a successful load operation in a newly initialized ResourceStorage.
	void store_load(std::unique_ptr<psi_serv::IResource> res) {
		std::unique_lock<std::mutex> lock(m_load_mut);
		// is_loaded must be false and m_res must be empty
		ASSERT(!m_is_loaded && !m_res);
		m_res = std::move(res);
		m_is_loaded = true;
		lock.unlock();
		m_load_cond.notify_all();
	}

	/// Check if it is still loading == nothing was stored.
	bool is_loaded() const {
		std::unique_lock<std::mutex> lock(m_load_mut);
		return m_is_loaded;
	}

	/// Wait until something gets stored.
	void wait_for_load() const {
		std::unique_lock<std::mutex> lock(m_load_mut);
		if (m_is_loaded)
			return;

		m_load_cond.wait(lock, [this]{ return m_is_loaded; });
	}

	/// Retrieve the state of this resource.
	psi_serv::ResourceState state() const {
		if (is_loaded())
			return psi_serv::ResourceState::Available;
		else
			return psi_serv::ResourceState::Loading;
	}

	/// Returns the stored resource if it was loaded. Assertion fails otherwise.
	psi_serv::IResource const* resource() const {
		ASSERT(is_loaded());
		return m_res.get();
	}

private:
	bool m_is_loaded;
	mutable std::mutex m_load_mut;
	mutable std::condition_variable m_load_cond;

	std::unique_ptr<psi_serv::IResource> m_res;
};

typedef tbb::concurrent_hash_map<size_t, ResourceStorage>::accessor accessor;
typedef tbb::concurrent_hash_map<size_t, ResourceStorage>::const_accessor const_accessor;

class ResourceLock : public psi_serv::IResourceLock {
public:
	explicit ResourceLock(std::unique_ptr<const_accessor>&& access)
		: m_access(std::move(access)) {}

	~ResourceLock() {}

	psi_serv::IResource const& resource() const {
		return *(*m_access)->second.resource();
	}

private:
	std::unique_ptr<const_accessor> m_access;
};

class ResourceLoader : public psi_serv::IResourceService {
public:
	explicit ResourceLoader(psi_thread::ITaskSubmitter* tasks)
		: m_task_submitter(tasks) {}

	psi_serv::ResourceState request_resource(
	size_t h,
	std::function<std::unique_ptr<psi_serv::IResource>()> loader
	) const override {
		// check whether the resource is already in map
		const_accessor access;
		m_resources.find(access, h);
		if (!access.empty()) {
			// access is pair<Key, Val>
			return access->second.state();
		}

		// load resource from disk
		m_task_submitter->submit_task(
			[&, h] {
				// insert element (ResourceStorage() auto sets state as Loading)
				accessor access;
				m_resources.insert(access, h);
				access.release();

				// try to load the resource
				std::unique_ptr<psi_serv::IResource> res;
				try {
					res = loader();
				}
				catch (std::exception const& e) {
					psi_log::error("ResourceLoader") << "Loading resource " << h << " failed with error: " << e.what() << "\n";
					// delete and quit if loading failed
					m_resources.erase(h);
					return;
				}

				m_resources.find(access, h);
				// I just inserted it but could be empty if the Loading element got deleted
				if (access.empty())
					return;

				access->second.store_load(std::move(res));
				psi_log::debug("ResourceLoader") << "Loaded resource " << h << " successfully.\n";
			}
		);

		// happens concurrently with the task submitted to m_task_submitter
		return psi_serv::ResourceState::Loading;
	}

	boost::optional<std::unique_ptr<psi_serv::IResourceLock>> wait_for_resource(size_t hash) const override {
		auto access = std::make_unique<const_accessor>();
		m_resources.find(*access, hash);
		if (access->empty())
			return boost::optional<std::unique_ptr<psi_serv::IResourceLock>>();

		(*access)->second.wait_for_load();
		// construct lock and return it
		return boost::optional<std::unique_ptr<psi_serv::IResourceLock>> (std::make_unique<ResourceLock>(std::move(access)));
	}

	psi_serv::ResourceState resource_state(size_t hash) const override {
		const_accessor access;
		m_resources.find(access, hash);
		if (access.empty())
			return psi_serv::ResourceState::Unavailable;

		return access->second.state();
	}

	// TODO this method
	psi_serv::ResourceState free_resource(size_t hash) const override {
		return psi_serv::ResourceState::Unavailable;
	};

private:
	mutable tbb::concurrent_hash_map<size_t, ResourceStorage> m_resources;
	psi_thread::ITaskSubmitter const* m_task_submitter;
};

std::unique_ptr<psi_serv::IResourceService> psi_serv::start_resource_loader(ResourceLoaderArgs args) {
	auto loader = new ResourceLoader(args.task_submitter);

	psi_log::info("ResourceLoader") << "Initialized the ResourceLoader service successfully.\n";
	return std::unique_ptr<IResourceService>(loader);
}
