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

#include "log.hpp"

#include <fstream>
#include <ctime>
#include <cassert>
#include <iostream>
#include <mutex>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


static bool INITIALIZED = false;
static gsg::log::Level MAX_LEVEL = gsg::log::LEVEL_EMERGENCY;
static std::ofstream LOG_FILE_OUT;
static std::mutex STREAMS;


void gsg::log::init(Environment const& env, Level max) {
	assert(!INITIALIZED);

	fs::path log_file = env.working_dir;
	log_file += "/log.log";
	fs::path old_log_file = env.working_dir;
	old_log_file += "/log_old.log";

	// remove old log if it exists and rename most recent one to old
	fs::remove(old_log_file);
	if (fs::exists(log_file))
		fs::rename(log_file, old_log_file);

	MAX_LEVEL = max;
	LOG_FILE_OUT = std::ofstream(log_file.c_str());
	INITIALIZED = true;

	if (!LOG_FILE_OUT.good()) {
		warning("Logger") << "Unable to create log.log file.\n";
	}

	info("Logger") << "Initialized logger module.\n";
}

static inline std::string header(gsg::log::Level lvl, std::string const& module, std::string const& part) {
	time_t time;
	std::time(&time);
	struct tm* info;
	info = localtime(&time);

	// format date and time
	std::string output = "[";
	output += (info->tm_hour >= 10 ? std::to_string(info->tm_hour) : "0" + std::to_string(info->tm_hour)) + ":";
	output += (info->tm_min >= 10 ? std::to_string(info->tm_min) : "0" + std::to_string(info->tm_min)) + ":";
	output += info->tm_sec >= 10 ? std::to_string(info->tm_sec) : "0" + std::to_string(info->tm_sec);
	output += "] ";

	// format part and module
	if (!part.empty()) {
		output += "[" + part + "] ";
	}

	if (!module.empty()) {
		output += "[" + module + "] ";
	}

	switch (lvl) {
		case gsg::log::LEVEL_EMERGENCY:
			output += "EMERGENCY:\n";
			break;
		case gsg::log::LEVEL_ALERT:
			output += "ALERT:\n";
			break;
		case gsg::log::LEVEL_CRITICAL:
			output += "CRITICAL:\n";
			break;
		case gsg::log::LEVEL_ERROR:
			output += "ERROR:\n";
			break;
		case gsg::log::LEVEL_WARNING:
			output += "WARNING:\n";
			break;
		case gsg::log::LEVEL_NOTICE:
			output += "NOTICE:\n";
			break;
		case gsg::log::LEVEL_INFO:
			output += "INFO:\n";
			break;
		case gsg::log::LEVEL_DEBUG:
			output += "DEBUG:\n";
			break;
	}

	return output;
}

sol::Streamer gsg::log::log(Level lvl, std::string const& module, std::string const& part) {
	assert(INITIALIZED);

	if (lvl <= MAX_LEVEL) {
        sol::Streamer stream([](std::string const& s){
            STREAMS.lock();

            std::cout << s;
            std::cout.flush();

            LOG_FILE_OUT << s;
            LOG_FILE_OUT.flush();

            STREAMS.unlock();
        });
		stream << header(lvl, module, part);
		return stream;
	}

	return sol::Streamer([](std::string const&){});
}
