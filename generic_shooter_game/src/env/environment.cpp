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

#include "environment.hpp"

#include <iostream>

#include <boost/program_options.hpp>
namespace po = boost::program_options;


bool gsg::parse_command_line(int argc, char** argv, Environment& store) {
	po::options_description options("Options");
	options.add_options()
		("help,h", "Display this help message.")
		("version,v", "Display the program version.")
		("directory,D", po::value<fs::path>()->default_value("./"), "Specifies a working directory for the program.")
	;

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, options), vm);
		po::notify(vm);
	}
	catch (std::exception const& e) {
		std::cout
			<< "Invalid option:\n"
			<< e.what() << std::endl;
		return false;
	}

	if (vm.count("help")) {
		std::cout
			<< "------Generic Shooter Game v. 0.1.0------\n"
			<< "Copyright (C) 2015-2016 Wojciech Nawrocki\n"
			<< "\n"
			<< options << std::endl;
		return false;
	}

	if (vm.count("version")) {
		std::cout << "0.1.0" << std::endl;
		return false;
	}

	if (!vm.count("directory")) {
		// wat. should be default "./"
		assert(false);
		return false;
	}

	store.working_dir = vm["directory"].as<fs::path>();
	if (!fs::is_directory(store.working_dir)) {
		std::cout << "Invalid working directory " << store.working_dir << "." << std::endl;
		return false;
	}
	// why is there no + operator on fs::path and char const*
	// but there exists a += operator? eh, boost
	store.resource_dir = store.working_dir;
	store.resource_dir += "/resources/";
	if (!fs::is_directory(store.resource_dir)) {
		std::cout << "Working directory does not contain a /resources/ subdirectory." << std::endl;
		return false;
	}

	return true;
}
