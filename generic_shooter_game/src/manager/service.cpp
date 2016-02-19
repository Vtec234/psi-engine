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

#include "service.hpp"


gsg::ServiceManager::ServiceManager()
	: m_resource()
	, m_window() {}

void gsg::ServiceManager::set_resource_service(std::unique_ptr<sol::IResourceService> ptr) {
	m_resource = std::move(ptr);
}

void gsg::ServiceManager::set_window_service(std::unique_ptr<sol::IWindowService> ptr) {
	m_window = std::move(ptr);
}

sol::IResourceService const& gsg::ServiceManager::resource_service() const {
	return *m_resource;
}

sol::IWindowService const& gsg::ServiceManager::window_service() const {
	return *m_window;
}

sol::IResourceService& gsg::ServiceManager::resource_service() {
	return *m_resource;
}

sol::IWindowService& gsg::ServiceManager::window_service() {
	return *m_window;
}
