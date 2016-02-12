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

#include <file/load.hpp>

#include "manager/environment.hpp"
#include "log/log.hpp"
#include "service/window_gl.hpp"
#include "service/resource.hpp"
#include "task/manager.hpp"
#include "resource/glsl_source.hpp"


int main(int argc, char** argv) {
	gsg::Environment env;
	if (!gsg::parse_command_line(argc, argv, env))
		return 0;

	gsg::log::init(env, gsg::log::LEVEL_DEBUG);

	gsg::TaskManager task_manager;

	auto resource_service = gsg::ResourceLoader::start_resource_loader(gsg::ResourceLoaderArgs{
		&task_manager,
	});
	

    try {
		auto status = resource_service->request_resource(
			std::hash<std::string>()("/glsl/box.vert"),
			[&]() {
				std::array<std::vector<std::string>, 6> srcs;
				
				srcs[0] = sol::file::load_text(env.resource_dir.string() + "/glsl/box.vert");
				srcs[1] = sol::file::load_text(env.resource_dir.string() + "/glsl/box.frag");
				
				return gsg::GLSLSource::construct_from_sources(srcs);
			}
		);
    }
	catch (std::exception const& e) {
		gsg::log::error("main") << e.what() << "\n";
		return 1;
	}

	auto window_service = gsg::GLWindowService::start_gl_window_service(gsg::GLWindowServiceArgs{
		640,
		480,
		8,
		"UltraWindow3600-K Ultimate Edition Beta v.2.5.7",
		true,
		true,
		true,
	});

	// TODO cap FPS
	while(!window_service->should_close()) {
		window_service->update_window();
	}

	return 0;
}


/*
    let tasks = manager::TaskManager;

    let mut systems = manager::SystemManager::new();
    let mut services = manager::ServiceManager::<resource::UniversalResourceLoader, window::WindowGl>::new();
	
    let resources = match resource::UniversalResourceLoader::new(resource::ResourceServiceArgs{
        resource_path: environment.resource_dir(),
        tasks: &tasks,
    }) {
        Ok(r) => r,
        Err(e) => panic!(e),
    };
    services.set_window_service(window);
    services.set_resource_service(resources);
    while !services.window_service_mut().should_close() {
        services.window_service_mut().update_window();
    }

}*/
