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

#include "resource.hpp"

#include <algorithm>
#include <memory>

#include "../log/log.hpp"


// -- ResourceStorage --
// TODO test this
gsg::ResourceLoader::ResourceStorage::ResourceStorage()
	: m_is_loaded(false) {}

gsg::ResourceLoader::ResourceStorage::~ResourceStorage() {
	// simulate successful load and notify everybody we're done
	// TODO won't this segfault if waiters check the Pred after m_is_loaded was destroyed?
	std::unique_lock<std::mutex> lock(m_load_mut);
	m_is_loaded = true;
	lock.unlock();
	m_load_cond.notify_all();
}

gsg::ResourceLoader::ResourceStorage::ResourceStorage(ResourceStorage const& cpy)
	: m_is_loaded(cpy.m_is_loaded) {
	// make deep copy of resource if it was loaded into cpy
	if (cpy.m_res) {
		m_res = cpy.m_res->clone();
	}
}

void gsg::ResourceLoader::ResourceStorage::store_load(std::unique_ptr<sol::IResource> res) {
	std::unique_lock<std::mutex> lock(m_load_mut);
	// is_loaded must be false and m_res must be empty
	assert(!m_is_loaded && !m_res);
	m_res = std::move(res);
	m_is_loaded = true;
	lock.unlock();
	m_load_cond.notify_all();
}

bool gsg::ResourceLoader::ResourceStorage::is_loaded() const {
	std::unique_lock<std::mutex> lock(m_load_mut);
	return m_is_loaded;
}

void gsg::ResourceLoader::ResourceStorage::wait_for_load() const {
	std::unique_lock<std::mutex> lock(m_load_mut);
	if (m_is_loaded)
		return;

	m_load_cond.wait(lock, [this]{ return m_is_loaded; });
}

sol::ResourceState gsg::ResourceLoader::ResourceStorage::state() const {
	if (is_loaded())
		return sol::Available;
	else
		return sol::Loading;
}

sol::IResource const* gsg::ResourceLoader::ResourceStorage::resource() const {
	assert(is_loaded());
	return m_res.get();
}

// -- ResourceLock --
gsg::ResourceLoader::ResourceLock::ResourceLock(std::unique_ptr<const_accessor>&& access)
	: m_access(std::move(access)) {}

gsg::ResourceLoader::ResourceLock::~ResourceLock() {}

sol::IResource const& gsg::ResourceLoader::ResourceLock::resource() const {
	return *(*m_access)->second.resource();
}

// -- ResourceLoader --
gsg::ResourceLoader::ResourceLoader(sol::ITaskSubmitter* tasks)
	: m_task_submitter(tasks) {}

std::unique_ptr<sol::IResourceService> gsg::ResourceLoader::start_resource_loader(ResourceLoaderArgs args) {
	auto loader = new ResourceLoader(args.task_submitter);

	gsg::log::info("ResourceLoader") << "Initialized the ResourceLoader service successfully.\n";
	return std::unique_ptr<sol::IResourceService>(loader);
}

sol::ResourceState gsg::ResourceLoader::request_resource(
	size_t h,
	std::function<std::unique_ptr<sol::IResource>()> loader
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
			std::unique_ptr<sol::IResource> res;
			try {
				res = loader();
			}
			catch (std::exception const& e) {
				log::error("ResourceLoader") << "Loading resource " << h << " failed with error: " << e.what() << "\n";
				// delete and quit if loading failed
				m_resources.erase(h);
				return;
			}

			m_resources.find(access, h);
			// I just inserted it but could be empty if the Loading element got deleted
			if (access.empty())
				return;

			access->second.store_load(std::move(res));
			log::debug("ResourceLoader") << "Loaded resource " << h << " successfully.\n";
		}
	);

	// happens concurrently with the task submitted to m_task_submitter
	return sol::Loading;
}

boost::optional<std::unique_ptr<sol::IResourceLock>> gsg::ResourceLoader::wait_for_resource(size_t hash) const {
	auto access = std::make_unique<const_accessor>();
	m_resources.find(*access, hash);
	if (access->empty())
		return boost::optional<std::unique_ptr<sol::IResourceLock>>();

	(*access)->second.wait_for_load();
	// construct lock and return it
	return boost::optional<std::unique_ptr<sol::IResourceLock>> (std::make_unique<ResourceLock>(std::move(access)));
}

sol::ResourceState gsg::ResourceLoader::resource_state(size_t hash) const {
	const_accessor access;
	m_resources.find(access, hash);
	if (access.empty())
		return sol::Unavailable;

	return access->second.state();
}

// TODO this method
sol::ResourceState gsg::ResourceLoader::free_resource(size_t hash) const {
	return sol::Unavailable;
}
