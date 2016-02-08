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

#include <iostream>

#include "manager/environment.hpp"
#include "log/log.hpp"
#include "service/window_gl.hpp"


int main(int argc, char** argv) {
	gsg::Environment env;
	if (!gsg::parse_command_line(argc, argv, env))
		return 0;

	gsg::log::init(env, gsg::log::LEVEL_DEBUG);

	auto window = gsg::GLWindowService::start_gl_window_service(gsg::GLWindowServiceArgs{
		640,
		480,
		8,
		"Ultra Window",
		true,
		true,
		true,
	});

	// TODO cap FPS
	while(!window->should_close()) {
		window->update_window();
	}

	return 0;
}


/*
    let tasks = manager::TaskManager;

    let mut systems = manager::SystemManager::new();
    let mut services = manager::ServiceManager::<resource::UniversalResourceLoader, window::WindowGl>::new();
    let window = match window::WindowGl::new(window::WindowGlArgs{
        width: 800,
        height: 600,
        samples: 2,
        title: "UltraWindow3600-K Ultimate Edition Beta v.2.5.7",
        resizable: true,
        always_on_top: false,
        debug: false,
        fullscreen: false,
    }) {
        Ok(w) => w,
        Err(e) => panic!(e),
    };
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
