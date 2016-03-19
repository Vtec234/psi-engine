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

#include <log/log.hpp>
#include <thread/impl/manager.hpp>
#include <service/manager.hpp>
#include <service/impl/resource.hpp>
#include <util/file.hpp>
#include <service/impl/glsl_source.hpp>
#include <service/impl/window_gl.hpp>
#include <system/manager.hpp>
#include <scene/impl/default_components.hpp>
#include <system/impl/renderer_gl.hpp>

#include "env/environment.hpp"


int main(int argc, char** argv) {
	gsg::Environment env;
	if (!gsg::parse_command_line(argc, argv, env))
		return 0;

	psi_log::init(env.working_dir, psi_log::Level::DEBUG);

	auto task_manager = psi_thread::start_default_task_manager();

	psi_serv::ServiceManager services;
	services.set_resource_service(psi_serv::start_resource_loader(psi_serv::ResourceLoaderArgs{
		task_manager.get(),
	}));

	try {
		auto status = services.resource_service().request_resource(
		std::hash<std::string>()("/glsl/box.vert"),
			[&]() {
				std::array<std::vector<std::string>, 6> srcs;

				srcs[0] = psi_util::load_text(env.resource_dir.string() + "/glsl/box.vert");
				srcs[1] = psi_util::load_text(env.resource_dir.string() + "/glsl/box.frag");

				return psi_serv::GLSLSource::construct_from_sources(srcs);
			}
		);
	}
	catch (std::exception const& e) {
		psi_log::error("main") << e.what() << "\n";
		return 1;
	}

	services.set_window_service(psi_serv::start_gl_window_service(psi_serv::GLWindowServiceArgs{
		640,
		480,
		8,
		"UltraWindow3600-K Ultimate Edition Beta v.2.5.7",
		true,
		true,
		true,
	}));

	psi_sys::SystemManager systems(task_manager.get());
	systems.register_component_type(psi_scene::component_type_entity_info);
	systems.register_component_type(psi_scene::component_type_model_info);
	systems.register_component_type(psi_scene::component_type_transform_info);
	systems.register_system(psi_sys::start_gl_renderer());
	systems.load_scene(nullptr);

	// TODO cap FPS
	auto& window = services.window_service();
	while(!window.should_close()) {
		systems.update_scene();
		window.update_window();
	}

	return 0;
}
