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

#include <service_resource.hpp>
#include <service_window.hpp>


namespace gsg {
// TODO maybe move this to compile-time dispatch with templates
// template <typename Res, typename Win>
class ServiceManager {
public:
	ServiceManager();

	void set_resource_service(std::unique_ptr<sol::ResourceService>);
	void set_window_service(std::unique_ptr<sol::WindowService>);

	sol::ResourceService& resource_service() const;
	sol::WindowService& window_service() const;

private:
	std::unique_ptr<sol::ResourceService> m_resource;
	std::unique_ptr<sol::WindowService> m_window;
};
}
