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

#pragma once

#include <cstdint>


namespace sol {
/// TODO this class
/// TODO replace void* with Scene*
class System {
public:
    virtual uint64_t required_components() const = 0;
    virtual void on_scene_loaded(void*) const = 0;
    virtual void on_scene_update(void*) const = 0;
    virtual void on_scene_save(void* scene, void* mem) const = 0;
    virtual void on_scene_shutdown(void*) const = 0;
};
} // namespace sol
