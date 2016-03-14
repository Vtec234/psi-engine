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

#include "stream.hpp"


psi_util::Streamer::Streamer(std::function<void(std::string const&)> out)
    : m_out(out) {}

psi_util::Streamer::~Streamer() {
    if (m_out)
        m_out(m_buffer);
}

psi_util::Streamer& psi_util::Streamer::operator<<(std::string const& s) {
	m_buffer += s;
	return *this;
}

psi_util::Streamer& psi_util::Streamer::operator<<(char const* c) {
	m_buffer += c;
	return *this;
}

psi_util::Streamer& psi_util::Streamer::operator<<(uint8_t i) {
	m_buffer += std::to_string(i);
	return *this;
}

psi_util::Streamer& psi_util::Streamer::operator<<(uint16_t i) {
	m_buffer += std::to_string(i);
	return *this;
}

psi_util::Streamer& psi_util::Streamer::operator<<(uint32_t i) {
	m_buffer += std::to_string(i);
	return *this;
}

psi_util::Streamer& psi_util::Streamer::operator<<(uint64_t i) {
	m_buffer += std::to_string(i);
	return *this;
}

psi_util::Streamer& psi_util::Streamer::operator<<(int8_t i) {
	m_buffer += std::to_string(i);
	return *this;
}

psi_util::Streamer& psi_util::Streamer::operator<<(int16_t i) {
	m_buffer += std::to_string(i);
	return *this;
}

psi_util::Streamer& psi_util::Streamer::operator<<(int32_t i) {
	m_buffer += std::to_string(i);
	return *this;
}

psi_util::Streamer& psi_util::Streamer::operator<<(int64_t i) {
	m_buffer += std::to_string(i);
	return *this;
}

psi_util::Streamer& psi_util::Streamer::operator<<(float f) {
	m_buffer += std::to_string(f);
	return *this;
}

psi_util::Streamer& psi_util::Streamer::operator<<(double d) {
	m_buffer += std::to_string(d);
	return *this;
}

psi_util::Streamer& psi_util::Streamer::operator<<(long double d) {
	m_buffer += std::to_string(d);
	return *this;
}

psi_util::Streamer& psi_util::Streamer::operator<<(bool b) {
	m_buffer += std::to_string(b);
	return *this;
}
