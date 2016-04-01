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

#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/gil/extension/dynamic_image/dynamic_image_all.hpp>


namespace psi_util {
	/// Tries to load a text file from the specified path.
	/// @throws if the file does not exist, is invalid, or otherwise occupied
	/// @returns a vector of lines from this file
	std::vector<std::string> load_text(boost::filesystem::path const& file);

	/// Tries to load a binary file from the specified path.
	/// @throws if the file does not exist, is invalid, or otherwise occupied
	/// @returns a vector containing the data from this file
	std::vector<char> load_binary(boost::filesystem::path const& file);

	/// Valid pixel types for an image file.
	using ImagePixelFormats = boost::mpl::vector<
		boost::gil::rgb8_image_t,
		boost::gil::rgba8_image_t,
		boost::gil::rgb16_image_t,
		boost::gil::rgba16_image_t,
		boost::gil::rgb32_image_t,
		boost::gil::rgba32_image_t>;

	enum class ImageFormat {
		PNG,
		JPEG,
		TIFF,
	};

	/// Tries to load an image file of the specified format from the specified path.
	/// @throws if the file does not exist, is invalid, is of different format, is otherwise occupied,
	/// or if the image file does not fit any pixel format in images_t
	/// @returns an owned image object
	boost::gil::any_image<ImagePixelFormats> load_image(boost::filesystem::path const& file, ImageFormat);
} // namespace psi_util
