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

#include "streamer.hpp"


sol::Streamer::Streamer(std::function<void(std::string const&)> out)
	: m_out(out) {}

sol::Streamer::~Streamer() {
	m_out(m_buffer);
}

sol::Streamer& sol::Streamer::operator<<(std::string const& s) {
	m_buffer += s;
	return *this;
}

sol::Streamer& sol::Streamer::operator<<(char const* c) {
	m_buffer += c;
	return *this;
}

sol::Streamer& sol::Streamer::operator<<(uint8_t i) {
	m_buffer += std::to_string(i);
	return *this;
}

sol::Streamer& sol::Streamer::operator<<(uint16_t i) {
	m_buffer += std::to_string(i);
	return *this;
}

sol::Streamer& sol::Streamer::operator<<(uint32_t i) {
	m_buffer += std::to_string(i);
	return *this;
}

sol::Streamer& sol::Streamer::operator<<(uint64_t i) {
	m_buffer += std::to_string(i);
	return *this;
}

sol::Streamer& sol::Streamer::operator<<(int8_t i) {
	m_buffer += std::to_string(i);
	return *this;
}

sol::Streamer& sol::Streamer::operator<<(int16_t i) {
	m_buffer += std::to_string(i);
	return *this;
}

sol::Streamer& sol::Streamer::operator<<(int32_t i) {
	m_buffer += std::to_string(i);
	return *this;
}

sol::Streamer& sol::Streamer::operator<<(int64_t i) {
	m_buffer += std::to_string(i);
	return *this;
}

sol::Streamer& sol::Streamer::operator<<(float f) {
	m_buffer += std::to_string(f);
	return *this;
}

sol::Streamer& sol::Streamer::operator<<(double d) {
	m_buffer += std::to_string(d);
	return *this;
}

sol::Streamer& sol::Streamer::operator<<(long double d) {
	m_buffer += std::to_string(d);
	return *this;
}

sol::Streamer& sol::Streamer::operator<<(bool b) {
	m_buffer += std::to_string(b);
	return *this;
}
