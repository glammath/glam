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
#include <glam/multipoint.h>

static const mp_float epsilon = boost::multiprecision::pow(mp_float(1), -20);

TEST(R2C_test, twice) {
    multipoint<mp_float, mp_complex> mpt("\\Gamma_1", mp_float(-2), mp_float(2), 50, [](const auto &z){ return mp_complex(2 * z); });
    mpt.full_eval();
    for (int i = 0; i < 200; i++) {
        EXPECT_LE((mpt[i].second.convert_to<mp_float>()) - (2 * mpt[i].first), epsilon);
    }
}

TEST(R2C_test, unit_circle) {
    multipoint<mp_float, mp_complex> mpt("U", mp_float(0), mp_float(2 * mp_pi), 100, [](const auto &z){
        return boost::multiprecision::pow(mp_e, mp_i * z);
    });
    mpt.full_eval();
//    for (int i = 0; i < mpt.values.size(); i++) {
//        std::cout << "f(" << mpt[i].first << ") = " << mpt[i].second << "  ";
//    }
    EXPECT_LE(boost::multiprecision::abs(mpt[0].second - (1 + 0 * mp_i)), epsilon);
    EXPECT_LE(boost::multiprecision::abs(mpt[628].second - (1 + 0 * mp_i)), epsilon);
}

#pragma clang diagnostic pop