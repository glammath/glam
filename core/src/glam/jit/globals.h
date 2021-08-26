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

#ifndef GLAMCORE_GLOBALS_H
#define GLAMCORE_GLOBALS_H

#include <map>
#include <string>
#include <utility>
#include "../types.h"


struct globals {
    static std::map<std::string, mp_complex *> consts_mp;
    static std::map<std::string, std::complex<double>> consts_dp;
    static std::map<std::string, uintptr_t> fxn_table;

    static bool is_global(const std::string &name);
    static bool is_fxn(const std::string &name);
};

#endif //GLAMCORE_GLOBALS_H
