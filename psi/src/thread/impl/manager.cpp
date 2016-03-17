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

#include "manager.hpp"


class TaskManager : public psi_thread::ITaskSubmitter {
public:
	uint64_t submit_task(std::function<void()> f) const override {
		// sshhh. nobody will ever know
		// to outsiders we are concurrent af
		f();
		return 0;
	}

	bool wait_for_task(uint64_t) const override {
		return false;
	}

	bool is_task_running(uint64_t) const override {
		return false;
	}
};

std::unique_ptr<psi_thread::ITaskSubmitter> psi_thread::start_default_task_manager() {
	return std::make_unique<TaskManager>();
}
