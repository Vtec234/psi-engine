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

#include <service/resource.hpp>


namespace gsg {
class GLSLSource : public sol::IResource {
public:
	~GLSLSource();

	std::unique_ptr<sol::IResource> copy() const override;

private:
	std::vector<std::string> m_source;
	// mappings of uniforms and uniform buffers to their names in the source
	std::unordered_map<Uniform, std::string> m_unifs;
	std::unordered_map<UniformBuffer, std::string> m_bufs;

};
} // namespace gsg
