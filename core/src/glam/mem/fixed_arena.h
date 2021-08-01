/*
 * Copyright 2021 Kioshi Morosin <glam@hex.lc>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GLAMCORE_FIXED_ARENA_H
#define GLAMCORE_FIXED_ARENA_H

#include "../utilities.h"

template <typename T> class fixed_arena {
    std::vector<T *> arena;
    uint32_t index;

public:
    std::vector<T *> consts;

    explicit fixed_arena(uint32_t _arena_size): arena(_arena_size), index(0) {
        GLAM_TRACE("arena size " << _arena_size);
        std::generate(arena.begin(), arena.end(), []() { return new T(0); });
    }

    ~fixed_arena() noexcept {
        std::for_each(consts.begin(), consts.end(), [](auto v) { delete v; });
    }

    T *alloc() {
        if (this->index >= this->arena.size()) {
            this->reset();
        }
        auto r = arena[this->index];
        this->index++;
        return r;
    }

    void reset() {
        this->index = 0;
    }

    uint32_t get_size() {
        return arena.size();
    }

    void release() {
        std::for_each(arena.begin(), arena.end(), [](auto v) { delete v; });
    }
};

#endif //GLAMCORE_FIXED_ARENA_H
