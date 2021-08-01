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

#ifndef GLAM_TYPES_H
#define GLAM_TYPES_H

#include <boost/multiprecision/gmp.hpp>
#include <boost/multiprecision/mpc.hpp>
#include <boost/math/constants/constants.hpp>

typedef boost::multiprecision::number<boost::multiprecision::gmp_float<25>> mp_float;
typedef boost::multiprecision::number<boost::multiprecision::mpc_complex_backend<25>> mp_complex;

#define mp_e boost::math::constants::e<mp_float>()
#define mp_pi boost::math::constants::pi<mp_float>()
#define mp_i mp_complex(0., 1.)

namespace glam_literals {
    mp_complex operator "" _i(long double f);

    mp_float operator "" _f(long double f);
}

#endif //GLAM_TYPES_H
