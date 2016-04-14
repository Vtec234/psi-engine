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

#include "log.hpp"

#include <fstream>
#include <ctime>
#include <iostream>
#include <mutex>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "../util/assert.hpp"


static bool INITIALIZED = false;
static psi_log::Level MAX_LEVEL = psi_log::Level::EMERGENCY;
static std::ofstream LOG_FILE_OUT;
static std::mutex STREAMS;


void psi_log::init(fs::path const& work_dir, Level max) {
	ASSERT(!INITIALIZED);

	fs::path log_file = work_dir;
	log_file += "/log.log";
	fs::path old_log_file = work_dir;
	old_log_file += "/log_old.log";

	// remove old log if it exists and rename most recent one to old
	fs::remove(old_log_file);
	if (fs::exists(log_file))
		fs::rename(log_file, old_log_file);

	MAX_LEVEL = max;
	LOG_FILE_OUT = std::ofstream(log_file.c_str());
	INITIALIZED = true;

	if (!LOG_FILE_OUT.good()) {
		warning("log") << "Unable to create log.log file. Logging to stdout only.\n";
	}

	info("log") << "Initialized logger module.\n";
}

static inline std::string __header(psi_log::Level lvl, std::string const& module, std::string const& part) {
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
		case psi_log::Level::EMERGENCY:
			output += "EMERGENCY:\n";
			break;
		case psi_log::Level::ALERT:
			output += "ALERT:\n";
			break;
		case psi_log::Level::CRITICAL:
			output += "CRITICAL:\n";
			break;
		case psi_log::Level::ERROR:
			output += "ERROR:\n";
			break;
		case psi_log::Level::WARNING:
			output += "WARNING:\n";
			break;
		case psi_log::Level::NOTICE:
			output += "NOTICE:\n";
			break;
		case psi_log::Level::INFO:
			output += "INFO:\n";
			break;
		case psi_log::Level::DEBUG:
			output += "DEBUG:\n";
			break;
	}

	return output;
}

psi_util::Streamer psi_log::log(Level lvl, std::string const& module, std::string const& part) {
	ASSERT(INITIALIZED);

	if (lvl <= MAX_LEVEL) {
        psi_util::Streamer stream([](std::string const& s){
            STREAMS.lock();

            std::cout << s;
            std::cout.flush();

            LOG_FILE_OUT << s;
            LOG_FILE_OUT.flush();

            STREAMS.unlock();
        });
		stream << __header(lvl, module, part);
		return stream;
	}

	return psi_util::Streamer([](std::string const&){});
}
