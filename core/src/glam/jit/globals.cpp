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

#include "globals.h"

std::map<std::string, mp_complex *> globals::consts_mp = { std::make_pair("e", new mp_complex(mp_e)),
        std::make_pair("\\pi", new mp_complex(mp_pi)), std::make_pair("i", new mp_i) };

std::map<std::string, std::complex<double>> globals::consts_dp = { std::make_pair("e", std::complex(M_E, 0.)),
        std::make_pair("\\pi", std::complex(M_PI, 0.)), std::make_pair("i", std::complex(0., 1.)) };

std::map<std::string, uintptr_t> globals::fxn_table;
