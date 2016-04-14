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

#include "manager.hpp"


psi_serv::ServiceManager::ServiceManager()
	: _resource()
	, _window() {}

void psi_serv::ServiceManager::set_resource_service(std::unique_ptr<IResourceService> ptr) {
	_resource = std::move(ptr);
}

void psi_serv::ServiceManager::set_window_service(std::unique_ptr<IWindowService> ptr) {
	_window = std::move(ptr);
}

psi_serv::IResourceService const& psi_serv::ServiceManager::resource_service() const {
	return *_resource;
}

psi_serv::IWindowService const& psi_serv::ServiceManager::window_service() const {
	return *_window;
}

psi_serv::IResourceService& psi_serv::ServiceManager::resource_service() {
	return *_resource;
}

psi_serv::IWindowService& psi_serv::ServiceManager::window_service() {
	return *_window;
}
