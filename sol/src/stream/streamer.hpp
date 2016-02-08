/*
 * Copyright (C) 2015-2016 Wojciech Nawrocki
 *
 * This file is part of the Sol Library.
 *
 * The Sol Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Sol Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Sol Library. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <string>
#include <functional>
#include <cstdint>


namespace sol {
class Streamer {
public:
	explicit Streamer(std::function<void(std::string const&)> out);
	~Streamer();

	// disable argument-less ctr and copying
	Streamer() = delete;
	Streamer(Streamer const&) = delete;
	Streamer operator=(Streamer const&) = delete;

	Streamer(Streamer&&) = default;

	Streamer& operator<<(std::string const&);
	Streamer& operator<<(char const*);
	Streamer& operator<<(uint8_t);
	Streamer& operator<<(uint16_t);
	Streamer& operator<<(uint32_t);
	Streamer& operator<<(uint64_t);
	Streamer& operator<<(int8_t);
	Streamer& operator<<(int16_t);
	Streamer& operator<<(int32_t);
	Streamer& operator<<(int64_t);
	Streamer& operator<<(float);
	Streamer& operator<<(double);
	Streamer& operator<<(long double);
	Streamer& operator<<(bool);

private:
	std::string m_buffer;
	std::function<void(std::string const&)> m_out;
};
} // namespace sol
