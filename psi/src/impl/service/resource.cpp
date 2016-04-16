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
#include <unordered_map>

#include <tbb/concurrent_hash_map.h>

#include "../../log/log.hpp"
#include "../../util/assert.hpp"


class ResourceStorage {
public:
	ResourceStorage()
		: _is_loaded(false) {}

	/// ResourceStorage must be CopyConstructible for concurrent_hash_map.
	ResourceStorage(ResourceStorage const& cpy)
		: _is_loaded(cpy._is_loaded) {
		// make deep copy of resource if it was loaded into cpy
		if (cpy._is_loaded) {
			_res = cpy._res;
		}
	}

	~ResourceStorage() {
		// simulate successful load and notify everybody we're done
		// TODO won't this segfault if waiters check the Pred after m_is_loaded was destroyed?
		std::unique_lock<std::mutex> lock(_load_mut);
		_is_loaded = true;
		lock.unlock();
		_load_cond.notify_all();
	}

	/// Store the result of a successful load operation in a newly initialized ResourceStorage.
	void store_load(boost::any&& res) {
		std::unique_lock<std::mutex> lock(_load_mut);
		// is_loaded must be false and m_res must be empty
		ASSERT(!_is_loaded);
		_res = std::move(res);
		_is_loaded = true;
		lock.unlock();
		_load_cond.notify_all();
	}

	/// Check if it is still loading == nothing was stored.
	bool is_loaded() const {
		std::unique_lock<std::mutex> lock(_load_mut);
		return _is_loaded;
	}

	/// Wait until something gets stored.
	void wait_for_load() const {
		std::unique_lock<std::mutex> lock(_load_mut);
		if (_is_loaded)
			return;

		_load_cond.wait(lock, [this]{ return _is_loaded; });
	}

	/// Retrieve the state of this resource.
	psi_serv::ResourceState state() const {
		if (is_loaded())
			return psi_serv::ResourceState::AVAILABLE;
		else
			return psi_serv::ResourceState::LOADING;
	}

	/// Returns the stored resource if it was loaded. Assertion fails otherwise.
	boost::any const& resource() const {
		ASSERT(is_loaded());
		return _res;
	}

private:
	bool _is_loaded;
	mutable std::mutex _load_mut;
	mutable std::condition_variable _load_cond;

	boost::any _res;
};

typedef tbb::concurrent_hash_map<size_t, ResourceStorage>::accessor accessor;
typedef tbb::concurrent_hash_map<size_t, ResourceStorage>::const_accessor const_accessor;

class ResourceLock : public psi_serv::IResourceLock {
public:
	explicit ResourceLock(std::unique_ptr<const_accessor>&& access)
		: _access(std::move(access)) {}

	~ResourceLock() {}

	boost::any const& resource() override {
		return (*_access)->second.resource();
	}

private:
	std::unique_ptr<const_accessor> _access;
};

class ResourceLoader : public psi_serv::IResourceService {
public:
	explicit ResourceLoader(psi_thread::TaskManager const& tasks)
		: _task_submitter(tasks) {}

	void register_loader(ResourceLoaderId id, std::function<boost::any(std::string const&)> loader) override {
		_loaders[id] = loader;
	}

	psi_serv::ResourceState request_resource(ResourceHandle h, ResourceLoaderId id, std::string location) const override {
		ASSERT(_loaders.count(id));

		return request_resource(h, [&,this]()->boost::any{ return (_loaders.at(id))(location); });
	}

	psi_serv::ResourceState request_resource(
	ResourceHandle h,
	std::function<boost::any()> loader
	) const override {
		// check whether the resource is already in map
		const_accessor access;
		_resources.find(access, h);
		if (!access.empty()) {
			// access is pair<Key, Val>
			return access->second.state();
		}

		// load resource from disk
		_task_submitter.submit_task(
			[&, h] {
				// insert element (ResourceStorage() auto sets state as Loading)
				accessor access;
				_resources.insert(access, h);
				access.release();

				// try to load the resource
				boost::any res;
				try {
					res = loader();
				}
				catch (std::exception const& e) {
					psi_log::error("ResourceLoader") << "Loading resource " << h << " failed with error: " << e.what() << "\n";
					// delete and quit if loading failed
					_resources.erase(h);
					return;
				}

				_resources.find(access, h);
				// I just inserted it but could be empty if the Loading element got deleted
				if (access.empty())
					return;

				access->second.store_load(std::move(res));
				psi_log::debug("ResourceLoader") << "Loaded resource " << h << " successfully.\n";
			}
		);

		// happens concurrently with the task submitted to m_task_submitter
		return psi_serv::ResourceState::LOADING;
	}

	boost::optional<std::unique_ptr<psi_serv::IResourceLock>> retrieve_resource(ResourceHandle h) const override {
		auto access = std::make_unique<const_accessor>();
		_resources.find(*access, h);
		if (access->empty())
			return boost::optional<std::unique_ptr<psi_serv::IResourceLock>>();

		(*access)->second.wait_for_load();
		// construct lock and return it
		return boost::optional<std::unique_ptr<psi_serv::IResourceLock>> (std::make_unique<ResourceLock>(std::move(access)));
	}

	psi_serv::ResourceState resource_state(ResourceHandle h) const override {
		const_accessor access;
		_resources.find(access, h);
		if (access.empty())
			return psi_serv::ResourceState::UNAVAILABLE;

		return access->second.state();
	}

	// TODO this method
	psi_serv::ResourceState free_resource(ResourceHandle h) const override {
		return psi_serv::ResourceState::UNAVAILABLE;
	};

private:
	std::unordered_map<ResourceLoaderId, std::function<boost::any(std::string const&)>> _loaders;

	mutable tbb::concurrent_hash_map<size_t, ResourceStorage> _resources;
	psi_thread::TaskManager const& _task_submitter;
};

std::unique_ptr<psi_serv::IResourceService> psi_serv::start_resource_loader(ResourceLoaderArgs args) {
	auto loader = new ResourceLoader(args.task_submitter);

	psi_log::info("ResourceLoader") << "Initialized the ResourceLoader service successfully.\n";
	return std::unique_ptr<IResourceService>(loader);
}
