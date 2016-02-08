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

#include <memory>
#include <atomic>

#include <service_window.hpp>


typedef struct GLFWwindow GLFWwindow;


namespace gsg {
struct GLWindowServiceArgs {
	uint16_t width;
	uint16_t height;

	uint8_t samples;

	char const* title;

	bool resizable;
	bool always_on_top;

	bool debug;
};

class GLWindowService : public sol::WindowService {
public:
	~GLWindowService();

	/// Starts the service and spawns a window.
	/// Might throw an exception or two.
	/// Shouldn't probably be called more than once - might bork.
	static std::unique_ptr<sol::WindowService> start_gl_window_service(GLWindowServiceArgs);

	void update_window() override;
	void set_mouse_block(bool) override;

	bool should_close() const override;
	uint32_t width() const override;
	uint32_t height() const override;
	double aspect_ratio() const override;

private:
	explicit GLWindowService(GLFWwindow*);

	void framebuffer_size_callback(int width, int height);
	void key_pressed_callback(int key, int scancode, int action, int mods);
	void mouse_pressed_callback(int button, int action, int mods);
	void cursor_moved_callback(double x, double y);

	GLFWwindow* m_window;

	std::atomic<bool> m_should_close;
	std::atomic<double> m_aspect_ratio;
	std::atomic<int> m_framebuffer_width;
	std::atomic<int> m_framebuffer_height;
};
}
