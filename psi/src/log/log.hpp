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
	enum Level {
		LEVEL_EMERGENCY,
		LEVEL_ALERT,
		LEVEL_CRITICAL,
		LEVEL_ERROR,
		LEVEL_WARNING,
		LEVEL_NOTICE,
		LEVEL_INFO,
		LEVEL_DEBUG,
	};

	void init(boost::filesystem::path const& work_dir, Level max);
	psi_util::Streamer log(Level, std::string const& module, std::string const& part);

	inline psi_util::Streamer emergency(std::string const& module = "", std::string const& part = "") {
		return log(LEVEL_EMERGENCY, module, part);
	}
	inline psi_util::Streamer alert(std::string const& module = "", std::string const& part = "") {
		return log(LEVEL_ALERT, module, part);
	}
	inline psi_util::Streamer critical(std::string const& module = "", std::string const& part = "") {
		return log(LEVEL_CRITICAL, module, part);
	}
	inline psi_util::Streamer error(std::string const& module = "", std::string const& part = "") {
		return log(LEVEL_ERROR, module, part);
	}
	inline psi_util::Streamer warning(std::string const& module = "", std::string const& part = "") {
		return log(LEVEL_WARNING, module, part);
	}
	inline psi_util::Streamer notice(std::string const& module = "", std::string const& part = "") {
		return log(LEVEL_NOTICE, module, part);
	}
	inline psi_util::Streamer info(std::string const& module = "", std::string const& part = "") {
		return log(LEVEL_INFO, module, part);
	}
	inline psi_util::Streamer debug(std::string const& module = "", std::string const& part = "") {
		return log(LEVEL_DEBUG, module, part);
	}
} // namespace psi_log
