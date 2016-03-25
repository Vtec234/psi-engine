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

#include <memory>

#include "resource.hpp"
#include "window.hpp"

#include "../marker/thread_safety.hpp"


namespace psi_serv {
/// A manager for objects implements I..Service interfaces.
/// The point of this manager is that a I..Service const& can safely be passed to any thread.
/// A requirement for I..Service implementations is that each const function be safe
/// with respect to other const functions, as well as non-const ones.
/// Non-const functions can only be called by a single thread.
class ServiceManager : psi_mark::ConstThreadsafe {
public:
	ServiceManager();

	void set_resource_service(std::unique_ptr<IResourceService>);
	void set_window_service(std::unique_ptr<IWindowService>);

	IResourceService const& resource_service() const;
	IWindowService const& window_service() const;

	IResourceService& resource_service();
	IWindowService& window_service();

private:
	std::unique_ptr<IResourceService> m_resource;
	std::unique_ptr<IWindowService> m_window;
};
} // namespace psi_serv
