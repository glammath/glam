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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include <gtest/gtest.h>
#include <glam/utilities.h>
#include <glam/types.h>

static const mp_float epsilon = boost::multiprecision::pow(mp_float(1), -20);

TEST(linspace_test, small_double) {
    std::vector<double> v(3);
    linspace(0., 1., v);
    EXPECT_TRUE(v[0] == 0. && v[1] == 0.5 && v[2] == 1);
}

TEST(linspace_test, big_mpf) {
    std::vector<mp_float> v(101);
    linspace(mp_float(0), mp_float(1), v);
    for (int i = 0; i < 101; ++i) {
        EXPECT_LE(v[i] - (mp_float(i) / 100), epsilon);
    }
}

TEST(latspace_test, small) {
    std::vector<mp_complex> v(100);
    latspace(mp_complex(0), 1 + 1 * mp_i, (0.1 + 0.1 * mp_i), v);
    for (int i = 0; i < 100; ++i) {
        EXPECT_LE(v[i].real() - mp_float(i % 10) / 10, epsilon);
        EXPECT_LE(v[i].imag() - i / 10, epsilon);
    }
}

#pragma clang diagnostic pop
