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

#include <cstdint>
#include <vector>
#include <functional>

#include "../marker/thread_safety.hpp"


namespace psi_serv {
/// A key present on a keyboard.
/// Names taken from GLFW.
enum class KeyboardInput {
	SPACE,
	APOSTROPHE,
	COMMA,
	MINUS,
	PERIOD,
	SLASH,

	_0,
	_1,
	_2,
	_3,
	_4,
	_5,
	_6,
	_7,
	_8,
	_9,

	SEMICOLON,
	EQUAL,

	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,

	LEFT_BRACKET,
	BACKSLASH,
	RIGHT_BRACKET,
	GRAVE_ACCENT,

	WORLD_1,
	WORLD_2,

	ESCAPE,
	ENTER,
	TAB,
	BACKSPACE,
	INSERT,
	DELETE,

	RIGHT,
	LEFT,
	DOWN,
	UP,

	PAGE_UP,
	PAGE_DOWN,
	HOME,
	END,
	CAPS_LOCK,
	SCROLL_LOCK,
	NUM_LOCK,
	PRINT_SCREEN,
	PAUSE,

	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	F13,
	F14,
	F15,
	F16,
	F17,
	F18,
	F19,
	F20,
	F21,
	F22,
	F23,
	F24,
	F25,

	KP_0,
	KP_1,
	KP_2,
	KP_3,
	KP_4,
	KP_5,
	KP_6,
	KP_7,
	KP_8,
	KP_9,

	KP_DECIMAL,
	KP_DIVIDE,
	KP_MULTIPLY,
	KP_SUBTRACT,
	KP_ADD,
	KP_ENTER,
	KP_EQUAL,

	LEFT_SHIFT,
	LEFT_CONTROL,
	LEFT_ALT,
	LEFT_SUPER,

	RIGHT_SHIFT,
	RIGHT_CONTROL,
	RIGHT_ALT,
	RIGHT_SUPER,

	MENU,
};

/// What happened to a given input method.
enum class InputAction {
	PRESSED,
	RELEASED,
};

/// One of the buttons present in a mouse.
enum class MouseButton {
	_1,
	_2,
	_3,
};

/// A service which manages a window and all the related events.
class IWindowService : psi_mark::ConstThreadsafe {
public:
	/// Polls window events and swaps frame buffers.
	virtual void update_window() = 0;

	/// Tells window to either hide and grab the mouse or let it work normally/display/move.
	virtual void set_mouse_block(bool) const = 0;
	/// Returns true if the window should close.
	virtual bool should_close() const = 0;
	/// Returns the width of the framebuffer in pixels.
	virtual uint32_t width() const = 0;
	/// Returns the height of the framebuffer in pixels.
	virtual uint32_t height() const = 0;
	/// Returns width/height.
	virtual double aspect_ratio() const = 0;

	/// Returns the current coordinates of the mouse cursor.
	virtual std::pair<double, double> mouse_pos() const = 0;

	/// Returns the currently active keyboard inputs, that is the currently pressed keys.
	virtual std::vector<KeyboardInput> active_keyboard_inputs() const = 0;

	/// Returns the currently pressed mouse buttons.
	virtual std::vector<MouseButton> active_mouse_buttons() const = 0;

	/// Register a function to be called on each keyboard input, that is each key press. The function must be thread-safe.
	virtual void register_keyboard_input_callback(std::function<void(KeyboardInput, InputAction)>) const = 0;

	/// Register a function to be called on each mouse button press. The function must be thread-safe.
	virtual void register_mouse_button_callback(std::function<void(MouseButton, InputAction)>) const = 0;

	/// Register a function to be called on each mouse cursor movement. The function must be thread-safe.
	virtual void register_mouse_move_callback(std::function<void(double, double)>) const = 0;
};
} // namespace psi_serv
