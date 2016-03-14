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

#include "../../log/log.hpp"


// -- ResourceStorage --
// TODO test this
psi_serv::ResourceLoader::ResourceStorage::ResourceStorage()
	: m_is_loaded(false) {}

psi_serv::ResourceLoader::ResourceStorage::~ResourceStorage() {
	// simulate successful load and notify everybody we're done
	// TODO won't this segfault if waiters check the Pred after m_is_loaded was destroyed?
	std::unique_lock<std::mutex> lock(m_load_mut);
	m_is_loaded = true;
	lock.unlock();
	m_load_cond.notify_all();
}

psi_serv::ResourceLoader::ResourceStorage::ResourceStorage(ResourceStorage const& cpy)
	: m_is_loaded(cpy.m_is_loaded) {
	// make deep copy of resource if it was loaded into cpy
	if (cpy.m_res) {
		m_res = cpy.m_res->clone();
	}
}

void psi_serv::ResourceLoader::ResourceStorage::store_load(std::unique_ptr<IResource> res) {
	std::unique_lock<std::mutex> lock(m_load_mut);
	// is_loaded must be false and m_res must be empty
	assert(!m_is_loaded && !m_res);
	m_res = std::move(res);
	m_is_loaded = true;
	lock.unlock();
	m_load_cond.notify_all();
}

bool psi_serv::ResourceLoader::ResourceStorage::is_loaded() const {
	std::unique_lock<std::mutex> lock(m_load_mut);
	return m_is_loaded;
}

void psi_serv::ResourceLoader::ResourceStorage::wait_for_load() const {
	std::unique_lock<std::mutex> lock(m_load_mut);
	if (m_is_loaded)
		return;

	m_load_cond.wait(lock, [this]{ return m_is_loaded; });
}

psi_serv::ResourceState psi_serv::ResourceLoader::ResourceStorage::state() const {
	if (is_loaded())
		return Available;
	else
		return Loading;
}

psi_serv::IResource const* psi_serv::ResourceLoader::ResourceStorage::resource() const {
	assert(is_loaded());
	return m_res.get();
}

// -- ResourceLock --
psi_serv::ResourceLoader::ResourceLock::ResourceLock(std::unique_ptr<const_accessor>&& access)
	: m_access(std::move(access)) {}

psi_serv::ResourceLoader::ResourceLock::~ResourceLock() {}

psi_serv::IResource const& psi_serv::ResourceLoader::ResourceLock::resource() const {
	return *(*m_access)->second.resource();
}

// -- ResourceLoader --
psi_serv::ResourceLoader::ResourceLoader(psi_thread::ITaskSubmitter* tasks)
	: m_task_submitter(tasks) {}

std::unique_ptr<psi_serv::IResourceService> psi_serv::ResourceLoader::start_resource_loader(ResourceLoaderArgs args) {
	auto loader = new ResourceLoader(args.task_submitter);

	psi_log::info("ResourceLoader") << "Initialized the ResourceLoader service successfully.\n";
	return std::unique_ptr<IResourceService>(loader);
}

psi_serv::ResourceState psi_serv::ResourceLoader::request_resource(
	size_t h,
	std::function<std::unique_ptr<IResource>()> loader
	) const {
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
			std::unique_ptr<IResource> res;
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
	return Loading;
}

boost::optional<std::unique_ptr<psi_serv::IResourceLock>> psi_serv::ResourceLoader::wait_for_resource(size_t hash) const {
	auto access = std::make_unique<const_accessor>();
	m_resources.find(*access, hash);
	if (access->empty())
		return boost::optional<std::unique_ptr<IResourceLock>>();

	(*access)->second.wait_for_load();
	// construct lock and return it
	return boost::optional<std::unique_ptr<IResourceLock>> (std::make_unique<ResourceLock>(std::move(access)));
}

psi_serv::ResourceState psi_serv::ResourceLoader::resource_state(size_t hash) const {
	const_accessor access;
	m_resources.find(access, hash);
	if (access.empty())
		return Unavailable;

	return access->second.state();
}

// TODO this method
psi_serv::ResourceState psi_serv::ResourceLoader::free_resource(size_t hash) const {
	return Unavailable;
}
