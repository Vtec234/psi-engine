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

#include "file.hpp"

#include <fstream>

// "libpng 1.4 dropped definitions of png_infopp_NULL and int_p_NULL. So add
#define png_infopp_NULL (png_infopp)NULL
#define int_p_NULL (int*)NULL
// in your code before including GIL headers."
// ~ wdscxsj @ Stack Overflow
// https://stackoverflow.com/questions/2442335/libpng-boostgil-png-infopp-null-not-found
#include <boost/gil/extension/io/png_dynamic_io.hpp>
#include <boost/gil/extension/io/jpeg_dynamic_io.hpp>
#include <boost/gil/extension/io/tiff_dynamic_io.hpp>
namespace gil = boost::gil;
namespace fs = boost::filesystem;


std::vector<std::string> psi_util::load_text(fs::path const& file) {
	// try to open input stream
	std::ifstream in_file(file.c_str());
	if (!in_file.good()) {
		in_file.close();
		throw std::runtime_error("Failed to open text file " + file.string() + ".");
	}

	// read line by line
	std::string line;
	std::vector<std::string> contents;
	while (in_file.good()) {
		getline(in_file, line);
		contents.push_back(line + "\n");
	}

	// close file and return contents
	in_file.close();
	return contents;
}

std::vector<char> psi_util::load_binary(fs::path const& file) {
	// might throw, pass exception if it does
	auto size = fs::file_size(file);

	// allocate memory of file size
	std::vector<char> bin(size);

	// try to open binary input stream
	std::ifstream in_file(file.c_str(), std::ios::in | std::ios::binary);
	if (!in_file.good()) {
		in_file.close();
		throw std::runtime_error("Failed to open binary file " + file.string() + ".");
	}

	// copy whole file into vector
	in_file.read(bin.data(), size);

	// close file and return contents
	in_file.close();
	return bin;
}

gil::any_image<psi_util::ImagePixelFormats> psi_util::load_image(boost::filesystem::path const& file, ImageFormat format) {
	gil::any_image<ImagePixelFormats> img;

	switch (format) {
		case psi_util::ImageFormat::PNG:
			gil::png_read_image(file.c_str(), img);

		case psi_util::ImageFormat::JPEG:
			gil::jpeg_read_image(file.c_str(), img);

		case psi_util::ImageFormat::TIFF:
			gil::jpeg_read_image(file.c_str(), img);
	}

	return img;
}
