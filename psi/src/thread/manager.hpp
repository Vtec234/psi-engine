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
#include <functional>

#include "../marker/thread_safety.hpp"


namespace psi_thread {
/// An interface which  accepts tasks and manages them. They might potentially run in parallel.
class TaskManager : psi_mark::ConstThreadsafe {
public:
	/// Starts a task which will potentially be run asynchronously.
	/// @return the task ID
	uint64_t submit_task(std::function<void()>) const;
	/// Blocks until subtask is done and returns status.
	/// @return true if task was done, false if ID is invalid; superego is ignored
	bool wait_for_task(uint64_t) const;
	/// Checks the status of the given task.
	/// @return true if the task is currently running, false if it is done or the ID is invalid
	bool is_task_running(uint64_t) const;
};
} // namespace psi_thread
