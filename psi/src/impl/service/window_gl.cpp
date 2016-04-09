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
#include <mutex>

#include <tbb/concurrent_vector.h>

#include "../rendering/gl/gl.hpp"
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
		if (m_block_mouse_changed) {
			if (m_block_mouse)
				glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			else
				glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

			m_block_mouse_changed = false;
		}

		glfwSwapBuffers(m_window);
		glfwPollEvents();

		m_should_close = glfwWindowShouldClose(m_window) || glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
	}

	void set_mouse_block(bool block) const override {
		m_block_mouse = block;
		m_block_mouse_changed = true;
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

	std::pair<double, double> mouse_pos() const override {
		return std::make_pair(m_mouse_x.load(), m_mouse_y.load());
	}

	std::vector<psi_serv::KeyboardInput> active_keyboard_inputs() const override {
		std::vector<psi_serv::KeyboardInput> vec;
		m_keyboard_inputs_mut.lock();
		if (!m_keyboard_inputs.empty())
			vec.insert(vec.begin(), m_keyboard_inputs.begin(), m_keyboard_inputs.end());
		m_keyboard_inputs_mut.unlock();
		return vec;
	}

	std::vector<psi_serv::MouseButton> active_mouse_buttons() const override {
		std::vector<psi_serv::MouseButton> vec;
		m_mouse_buttons_mut.lock();
		if (!m_mouse_buttons.empty())
			vec.insert(vec.begin(), m_mouse_buttons.begin(), m_mouse_buttons.end());
		m_mouse_buttons_mut.unlock();
		return vec;
	}

	void register_keyboard_input_callback(std::function<void(psi_serv::KeyboardInput, psi_serv::InputAction)> f) const override {
		m_keyboard_input_callbacks.push_back(f);
	}

	void register_mouse_button_callback(std::function<void(psi_serv::MouseButton, psi_serv::InputAction)> f) const override {
		m_mouse_button_callbacks.push_back(f);
	}

	void register_mouse_move_callback(std::function<void(double, double)> f) const override {
		m_mouse_move_callbacks.push_back(f);
	}

	void key_pressed_callback(int key, int /*scancode*/, int action, int /*mods*/) {
		psi_serv::KeyboardInput k;

		switch (key) {
		case GLFW_KEY_Q: k = psi_serv::KeyboardInput::Q; break;
		case GLFW_KEY_W: k = psi_serv::KeyboardInput::W; break;
		case GLFW_KEY_E: k = psi_serv::KeyboardInput::E; break;
		case GLFW_KEY_R: k = psi_serv::KeyboardInput::R; break;
		case GLFW_KEY_T: k = psi_serv::KeyboardInput::T; break;
		case GLFW_KEY_Y: k = psi_serv::KeyboardInput::Y; break;
		case GLFW_KEY_U: k = psi_serv::KeyboardInput::U; break;
		case GLFW_KEY_I: k = psi_serv::KeyboardInput::I; break;
		case GLFW_KEY_O: k = psi_serv::KeyboardInput::O; break;
		case GLFW_KEY_P: k = psi_serv::KeyboardInput::P; break;
		case GLFW_KEY_A: k = psi_serv::KeyboardInput::A; break;
		case GLFW_KEY_S: k = psi_serv::KeyboardInput::S; break;
		case GLFW_KEY_D: k = psi_serv::KeyboardInput::D; break;
		case GLFW_KEY_F: k = psi_serv::KeyboardInput::F; break;
		case GLFW_KEY_G: k = psi_serv::KeyboardInput::G; break;
		case GLFW_KEY_H: k = psi_serv::KeyboardInput::H; break;
		case GLFW_KEY_J: k = psi_serv::KeyboardInput::J; break;
		case GLFW_KEY_K: k = psi_serv::KeyboardInput::K; break;
		case GLFW_KEY_L: k = psi_serv::KeyboardInput::L; break;
		case GLFW_KEY_Z: k = psi_serv::KeyboardInput::Z; break;
		case GLFW_KEY_X: k = psi_serv::KeyboardInput::X; break;
		case GLFW_KEY_C: k = psi_serv::KeyboardInput::C; break;
		case GLFW_KEY_V: k = psi_serv::KeyboardInput::V; break;
		case GLFW_KEY_B: k = psi_serv::KeyboardInput::B; break;
		case GLFW_KEY_N: k = psi_serv::KeyboardInput::N; break;
		case GLFW_KEY_M: k = psi_serv::KeyboardInput::M; break;
		case GLFW_KEY_LEFT_SHIFT: k = psi_serv::KeyboardInput::LEFT_SHIFT; break;

		default:
			return;
		}

		switch (action) {
		case GLFW_PRESS:
			m_keyboard_inputs_mut.lock();
			m_keyboard_inputs.push_back(k);
			m_keyboard_inputs_mut.unlock();
			for (auto const& f : m_keyboard_input_callbacks)
				f(k, psi_serv::InputAction::PRESSED);
			break;

		case GLFW_RELEASE:
			m_keyboard_inputs_mut.lock();
			m_keyboard_inputs.erase(std::find(m_keyboard_inputs.begin(), m_keyboard_inputs.end(), k));
			m_keyboard_inputs_mut.unlock();
			for (auto const& f : m_keyboard_input_callbacks)
				f(k, psi_serv::InputAction::RELEASED);
			break;

		default:
			return;
		}
	}

	void mouse_pressed_callback(int button, int action, int /*mods*/) {
		psi_serv::MouseButton b;

		switch (button) {
		case GLFW_MOUSE_BUTTON_1:
			b = psi_serv::MouseButton::_1;
			break;

		case GLFW_MOUSE_BUTTON_2:
			b = psi_serv::MouseButton::_2;
			break;

		case GLFW_MOUSE_BUTTON_3:
			b = psi_serv::MouseButton::_3;
			break;

		default:
			return;
		}

		switch (action) {
		case GLFW_PRESS:
			m_mouse_buttons_mut.lock();
			m_mouse_buttons.push_back(b);
			m_mouse_buttons_mut.unlock();
			for (auto const& f : m_mouse_button_callbacks)
				f(b, psi_serv::InputAction::PRESSED);
			break;

		case GLFW_RELEASE:
			m_mouse_buttons_mut.lock();
			m_mouse_buttons.erase(std::find(m_mouse_buttons.begin(), m_mouse_buttons.end(), b));
			m_mouse_buttons_mut.unlock();
			for (auto const& f : m_mouse_button_callbacks)
				f(b, psi_serv::InputAction::RELEASED);
			break;

		default:
			return;
		}
	}

	void cursor_moved_callback(double x, double y) {
		m_mouse_x = x;
		m_mouse_y = y;

		for (auto const& f : m_mouse_move_callbacks)
			f(x, y);
	}

private:
	GLFWwindow* m_window;

	std::atomic<bool> m_should_close;
	std::atomic<double> m_aspect_ratio;
	std::atomic<int> m_framebuffer_width;
	std::atomic<int> m_framebuffer_height;

	std::atomic<double> m_mouse_x;
	std::atomic<double> m_mouse_y;

	mutable std::atomic<bool> m_block_mouse;
	mutable std::atomic<bool> m_block_mouse_changed;

	mutable std::mutex m_keyboard_inputs_mut;
	mutable std::vector<psi_serv::KeyboardInput> m_keyboard_inputs;
	mutable std::mutex m_mouse_buttons_mut;
	mutable std::vector<psi_serv::MouseButton> m_mouse_buttons;

	mutable tbb::concurrent_vector<std::function<void(psi_serv::KeyboardInput, psi_serv::InputAction)>> m_keyboard_input_callbacks;
	mutable tbb::concurrent_vector<std::function<void(psi_serv::MouseButton, psi_serv::InputAction)>> m_mouse_button_callbacks;
	mutable tbb::concurrent_vector<std::function<void(double, double)>> m_mouse_move_callbacks;
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
	service->framebuffer_size_callback(args.width, args.height);

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
