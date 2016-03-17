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

#include "window_gl.hpp"

#include <iostream>
#include <atomic>

#include "../../util/gl.hpp"
#include <GLFW/glfw3.h>

#include "../../log/log.hpp"


/// A class managing a GLFW window with an OpenGL rendering context.
class GLWindowService : public psi_serv::IWindowService {
public:
	explicit GLWindowService(GLFWwindow* win)
	: m_window(win)
	, m_should_close(false) {}

	virtual ~GLWindowService() {
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	void update_window() override {
		glfwSwapBuffers(m_window);
		glfwPollEvents();

		m_should_close = glfwWindowShouldClose(m_window) || glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
	}

	void set_mouse_block(bool block) override {
		if (block)
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	bool should_close() const override {
		return m_should_close;
	}

	uint32_t width() const override {
		return m_framebuffer_width;
	}

	uint32_t height() const override {
		return m_framebuffer_height;
	}

	double aspect_ratio() const override {
		return m_aspect_ratio;
	}

	void framebuffer_size_callback(int width, int height) {
		m_aspect_ratio = static_cast<double>(width) / static_cast<double>(height);
		m_framebuffer_width = width;
		m_framebuffer_height = height;
	}

	// TODO store all the input data atomically
	void key_pressed_callback(int key, int scancode, int action, int mods) {}

	void mouse_pressed_callback(int button, int action, int mods) {}

	void cursor_moved_callback(double x, double y) {}

private:
	GLFWwindow* m_window;

	std::atomic<bool> m_should_close;
	std::atomic<double> m_aspect_ratio;
	std::atomic<int> m_framebuffer_width;
	std::atomic<int> m_framebuffer_height;
};

static GLvoid APIENTRY gl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar* message, const GLvoid*) {
	bool is_error;
	std::string msg;

	switch (severity) {
		case gl::DEBUG_SEVERITY_HIGH:
			msg = "OpenGL HIGH severity error (" + std::to_string(id) + "):\n";
			is_error = true;
			break;

		case gl::DEBUG_SEVERITY_MEDIUM:
			msg = "OpenGL MEDIUM severity error (" + std::to_string(id) + "):\n";
			is_error = true;
			break;

		case gl::DEBUG_SEVERITY_LOW:
			msg = "OpenGL LOW severity error (" + std::to_string(id) + "):\n";
			is_error = true;
			break;

		case gl::DEBUG_SEVERITY_NOTIFICATION:
			msg = "OpenGL notification (" + std::to_string(id) + "):\n";
			is_error = false;
			break;

		default:
			return;
	}

	if 		(source == gl::DEBUG_SOURCE_API)				msg += "Source: API\n";
	else if (source == gl::DEBUG_SOURCE_WINDOW_SYSTEM)		msg += "Source: WINDOW_SYSTEM\n";
	else if (source == gl::DEBUG_SOURCE_SHADER_COMPILER)	msg += "Source: SHADER_COMPILER\n";
	else if (source == gl::DEBUG_SOURCE_THIRD_PARTY)		msg += "Source: THIRD_PARTY\n";
	else if (source == gl::DEBUG_SOURCE_APPLICATION)		msg += "Source: APPLICATION\n";
	else if (source == gl::DEBUG_SOURCE_OTHER)				msg += "Source: OTHER\n";

	if 		(type == gl::DEBUG_TYPE_ERROR)					msg += "Type: ERROR\n";
	else if (type == gl::DEBUG_TYPE_DEPRECATED_BEHAVIOR)	msg += "Type: DEPRECATED_BEHAVIOR\n";
	else if (type == gl::DEBUG_TYPE_UNDEFINED_BEHAVIOR)		msg += "Type: UNDEFINED_BEHAVIOR\n";
	else if (type == gl::DEBUG_TYPE_PORTABILITY)			msg += "Type: PORTABILITY\n";
	else if (type == gl::DEBUG_TYPE_PERFORMANCE)			msg += "Type: PERFORMANCE\n";
	else if (type == gl::DEBUG_TYPE_OTHER)					msg += "Type: OTHER\n";

	msg += "Message: ";
	msg += message;

	if (is_error)
		psi_log::error("OpenGL") << msg << "\n";
	else
		psi_log::debug("OpenGL") << msg << "\n";
}

std::unique_ptr<psi_serv::IWindowService> psi_serv::start_gl_window_service(GLWindowServiceArgs args) {
	// -- START GLFW --
	if (glfwInit() == 0)
		throw std::runtime_error("Failed to initialize GLFW.");

	glfwSetErrorCallback(
		[] (GLint code, char const* desc) {
			psi_log::error("GLFW") << "(" << code << "): " << desc << "\n";
		}
	);

	psi_log::info("GLWindowService") << "Initialized GLFW " << glfwGetVersionString() << "\n";

	// -- START GLFW WINDOW --
	// GL version 4.4
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	// No backwards compatibility
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
	// Resizability of this window
	glfwWindowHint(GLFW_RESIZABLE, args.resizable);
	// Whether the window is always on top
	glfwWindowHint(GLFW_FLOATING, args.always_on_top);
	// How many samples to take for anti-aliasing (MSAA)
	glfwWindowHint(GLFW_SAMPLES, args.samples);
	// Whether it's a debug context
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, args.debug);
	// sets the window to be sRGB capable, meaning it can convert linear output from shader to sRGB in framebuffer
	glfwWindowHint(GLFW_SRGB_CAPABLE, true);

	auto window = glfwCreateWindow(args.width, args.height, args.title, nullptr, nullptr);
	if (window == nullptr)
		throw std::runtime_error("Failed to create a GLFW window.");

	// key presses are remembered until polled
	glfwSetInputMode(window, GLFW_STICKY_KEYS, true);

	// create the service and map its address to the user pointer in GLFW
	auto service = new GLWindowService(window);
	glfwSetWindowUserPointer(window, service);

	glfwSetFramebufferSizeCallback(window,
		[] (GLFWwindow* win, int width, int height) {
			if (width <= 1 || height <= 1)
				return;

			static_cast<GLWindowService*>(glfwGetWindowUserPointer(win))->framebuffer_size_callback(width, height);
		}
	);

	glfwSetKeyCallback(window,
		[] (GLFWwindow* win, int key, int scancode, int action, int mods) {
			static_cast<GLWindowService*>(glfwGetWindowUserPointer(win))->key_pressed_callback(key, scancode, action, mods);
		}
	);

	glfwSetMouseButtonCallback(window,
		[] (GLFWwindow* win, int button, int action, int mods) {
			static_cast<GLWindowService*>(glfwGetWindowUserPointer(win))->mouse_pressed_callback(button, action, mods);
		}
	);

	glfwSetCursorPosCallback(window,
		[] (GLFWwindow* win, double x, double y) {
			static_cast<GLWindowService*>(glfwGetWindowUserPointer(win))->cursor_moved_callback(x, y);
		}
	);

	glfwMakeContextCurrent(window);

	// -- INITIALIZE OPENGL --
	auto load_status = gl::sys::LoadFunctions();
	if (!load_status)
		throw std::runtime_error(std::string("OpenGL Core ") + std::to_string(gl::sys::GetMajorVersion()) + "." + std::to_string(gl::sys::GetMinorVersion()) + " initialization failed.\n"
		+ "Number of functions that failed to load: " + std::to_string(load_status.GetNumMissing()));

	gl::Enable(gl::DEBUG_OUTPUT_SYNCHRONOUS);
	gl::DebugMessageCallback(gl_error_callback, nullptr);

	psi_log::info("GLWindowService") << "Initialized OpenGL Core " << gl::sys::GetMajorVersion() << "." << gl::sys::GetMinorVersion() << "\n";

	// -- RETURN SERVICE --
	return std::unique_ptr<IWindowService>(service);
}
