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

#include <array>
#include <string>
#include <locale>

#include <log/log.hpp>
#include <impl/impl.hpp>
#include <util/file.hpp>
#include <system/manager.hpp>

#include "env/environment.hpp"


int main(int argc, char** argv) {
	gsg::Environment env;
	if (!gsg::parse_command_line(argc, argv, env))
		return 0;

	psi_log::init(env.working_dir, psi_log::Level::DEBUG);

	psi_thread::TaskManager task_manager;

	psi_serv::ServiceManager services;
	services.set_resource_service(psi_serv::start_resource_loader(psi_serv::ResourceLoaderArgs{
		task_manager,
	}));

	services.set_window_service(psi_serv::start_gl_window_service(psi_serv::GLWindowServiceArgs{
		640,
		480,
		8,
		u8"UltraWindow3600-K Ultimate Edition Beta v.0.0.1",
		true,
		true,
		true,
	}));

	psi_rndr::load_texture("/home/wojciech/Programming/C++/UltraSuite2K/resources/textures/cone.dds");

	std::hash<std::string> hash;
	services.resource_service().register_loader(hash(u8"mesh"),
		[] (std::string const& s) -> auto {
			return psi_rndr::load_mesh(s);
		});
	services.resource_service().register_loader(hash(u8"texture"),
		[] (std::string const& s) -> auto {
			return psi_util::load_image(s, psi_util::ImageFormat::PNG);
		});
	services.resource_service().register_loader(hash(u8"shader"),
		[] (std::string const& s) -> auto {
			return psi_gl::parse_glsl_source({{
				psi_util::load_text(s+u8".vert"),
				psi_util::load_text(s+u8".frag"),
			}});
		});

	psi_sys::SystemManager systems(task_manager);
	systems.register_component_type(psi_scene::component_type_entity_info);
	systems.register_component_type(psi_scene::component_type_model_info);
	systems.register_component_type(psi_scene::component_type_transform_info);
	systems.register_system(psi_sys::start_gl_renderer(task_manager, services));
	systems.load_scene(nullptr);

	// TODO cap FPS
	auto& window = services.window_service();
	while(!window.should_close()) {
		systems.update_scene();
		window.update_window();
	}

	return 0;
}
