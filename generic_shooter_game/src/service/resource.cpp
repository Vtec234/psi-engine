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
		m_res = cpy.m_res->copy();
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
gsg::ResourceLoader::ResourceLock::ResourceLock(const_accessor* access) {
	// why? :(
	memcpy(&m_access, access, sizeof(const_accessor));
	memset(access, 0, sizeof(const_accessor));
}

gsg::ResourceLoader::ResourceLock::~ResourceLock() {}

sol::IResource const* gsg::ResourceLoader::ResourceLock::resource() const {
	return m_access->second.resource();
}

// -- ResourceLoader --
gsg::ResourceLoader::ResourceLoader(sol::ITaskSubmitter* tasks, fs::path dir)
	: m_task_submitter(tasks)
	, m_resource_dir(dir) {}

std::unique_ptr<sol::IResourceService> gsg::ResourceLoader::start_resource_loader(ResourceLoaderArgs args) {
	auto loader = new ResourceLoader(args.task_submitter, args.resource_dir);
	return std::unique_ptr<sol::IResourceService>(loader);
}

size_t gsg::ResourceLoader::hash_string(std::string const& s) const {
	return tbb::tbb_hash_compare<std::string>::hash(s);
}

sol::ResourceState gsg::ResourceLoader::request_resource(
	std::string const& s,
	std::function<std::unique_ptr<sol::IResource>(std::string const)> loader
	) const {
	// check whether a file with this name exists
	if (!fs::exists(m_resource_dir.string() + s))
		return sol::Unavailable;

	// check whether the resource is already in map
	const_accessor access;
	m_resources.find(access, hash_string(s));
	if (!access.empty()) {
		// access is pair<Key, Val>
		return access->second.state();
	}

	// load resource from disk
	m_task_submitter->submit_task(
	// TODO does this copy the string or only the std::string const& (ptr)?
		[&, s] {
			// insert element (ResourceStorage() auto sets state as Loading)
			accessor access;
			m_resources.insert(access, hash_string(s));
			access.release();

			// try to load the resource
			std::unique_ptr<sol::IResource> res;
			try {
				res = loader(s);
			}
			catch (std::exception const& e) {
				log::error("ResourceLoader") << "Loading resource " << s << " failed with error: " << e.what() << "\n";
				// delete and quit if loading failed
				m_resources.erase(hash_string(s));
				return;
			}

			m_resources.find(access, hash_string(s));
			// I just inserted it but could be empty if the Loading element got deleted
			if (access.empty())
				return;

			access->second.store_load(std::move(res));
			log::debug("ResourceLoader") << "Loaded resource " << s << " successfully.\n";
		}
	);

	// happens concurrently with the task submitted to m_task_submitter
	return sol::Loading;
}

boost::optional<std::unique_ptr<sol::IResourceLock>> gsg::ResourceLoader::wait_for_resource(size_t hash) const {
	const_accessor access;
	m_resources.find(access, hash);
	if (access.empty())
		return boost::optional<std::unique_ptr<sol::IResourceLock>>();

	access->second.wait_for_load();
	// construct lock and return it
	return boost::optional<std::unique_ptr<sol::IResourceLock>> (std::make_unique<ResourceLock>(&access));

}

sol::ResourceState gsg::ResourceLoader::resource_state(size_t hash) const {
	const_accessor access;
	m_resources.find(access, hash);
	if (access.empty())
		return sol::Unavailable;

	return access->second.state();
}

bool gsg::ResourceLoader::free_resource(size_t hash) const {
	// TODO this method
	return false;
}
