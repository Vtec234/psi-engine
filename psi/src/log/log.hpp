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

#pragma once

#include <boost/filesystem.hpp>

#include "../util/stream.hpp"


namespace psi_log {
	/// The importance level of a log message.
	enum class Level {
		EMERGENCY,
		ALERT,
		CRITICAL,
		ERROR,
		WARNING,
		NOTICE,
		INFO,
		DEBUG,
	};

	/// Initializes the logger.
	/// @param[in] work_dir working directory to put log files in
	/// @param[in] max the maximum logged level
	void init(boost::filesystem::path const& work_dir, Level max);

	/// Begins logging a string with the given level.
	/// @param[in] lvl the importance level
	/// @param[in] module the program module
	/// @param[in] part the program part
	/// @returns a cout-like streamer object to log into
	psi_util::Streamer log(Level lvl, std::string const& module, std::string const& part);

	inline psi_util::Streamer emergency(std::string const& module = "", std::string const& part = "") {
		return log(Level::EMERGENCY, module, part);
	}
	inline psi_util::Streamer alert(std::string const& module = "", std::string const& part = "") {
		return log(Level::ALERT, module, part);
	}
	inline psi_util::Streamer critical(std::string const& module = "", std::string const& part = "") {
		return log(Level::CRITICAL, module, part);
	}
	inline psi_util::Streamer error(std::string const& module = "", std::string const& part = "") {
		return log(Level::ERROR, module, part);
	}
	inline psi_util::Streamer warning(std::string const& module = "", std::string const& part = "") {
		return log(Level::WARNING, module, part);
	}
	inline psi_util::Streamer notice(std::string const& module = "", std::string const& part = "") {
		return log(Level::NOTICE, module, part);
	}
	inline psi_util::Streamer info(std::string const& module = "", std::string const& part = "") {
		return log(Level::INFO, module, part);
	}
	inline psi_util::Streamer debug(std::string const& module = "", std::string const& part = "") {
		return log(Level::DEBUG, module, part);
	}
} // namespace psi_log
