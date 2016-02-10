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

#include <memory>

#include <service/resource.hpp>
#include <service/window.hpp>


namespace gsg {
class ServiceManager {
public:
	ServiceManager();

	void set_resource_service(std::unique_ptr<sol::IResourceService>);
	void set_window_service(std::unique_ptr<sol::IWindowService>);

	sol::IResourceService const& resource_service() const;
	sol::IWindowService const& window_service() const;

private:
	std::unique_ptr<sol::IResourceService> m_resource;
	std::unique_ptr<sol::IWindowService> m_window;
};
} // namespace gsg
