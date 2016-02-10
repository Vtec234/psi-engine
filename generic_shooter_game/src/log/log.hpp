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

#include <io/streamer.hpp>

#include "../manager/environment.hpp"


namespace gsg {
namespace log {
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

	void init(Environment const&, Level max);
	sol::Streamer log(Level, std::string const& module, std::string const& part);

	inline sol::Streamer emergency(std::string const& module = "", std::string const& part = "") {
		return log(LEVEL_EMERGENCY, module, part);
	}
	inline sol::Streamer alert(std::string const& module = "", std::string const& part = "") {
		return log(LEVEL_ALERT, module, part);
	}
	inline sol::Streamer critical(std::string const& module = "", std::string const& part = "") {
		return log(LEVEL_CRITICAL, module, part);
	}
	inline sol::Streamer error(std::string const& module = "", std::string const& part = "") {
		return log(LEVEL_ERROR, module, part);
	}
	inline sol::Streamer warning(std::string const& module = "", std::string const& part = "") {
		return log(LEVEL_WARNING, module, part);
	}
	inline sol::Streamer notice(std::string const& module = "", std::string const& part = "") {
		return log(LEVEL_NOTICE, module, part);
	}
	inline sol::Streamer info(std::string const& module = "", std::string const& part = "") {
		return log(LEVEL_INFO, module, part);
	}
	inline sol::Streamer debug(std::string const& module = "", std::string const& part = "") {
		return log(LEVEL_DEBUG, module, part);
	}
} // namespace log
} // namespace gsg
