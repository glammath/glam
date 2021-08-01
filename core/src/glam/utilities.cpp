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

#include "utilities.h"
#include "types.h"
#include <algorithm>
#include <iostream>

template <typename T, typename Cont> void linspace(T left, T right, Cont &container) {
    using size_type = typeof(container.size());
    T step_size = (right - left) / (container.size() - 1);
    size_type i = 0;
    std::generate(container.begin(), container.end(), [left, step_size, i]() mutable {
        return left + (step_size * (i++));
    });
}

template void linspace<double, std::vector<double>>(double left, double right, std::vector<double> &container); // for testing
template void linspace<mp_float, std::vector<mp_float>>(mp_float left, mp_float right, std::vector<mp_float> &container);


template <typename T, typename Cont> void latspace(const T &left, const T &right, const T &spacing, Cont &container) {
    auto iter = container.begin();

    for (T y = left; y.imag() <= right.imag(); y += spacing) {
        for (T x = left; x.real() <= right.real(); x += spacing) {
            if (iter != container.end()) {
                *iter++ = T(x.real(), y.imag());
            }
        }
    }
}

template void latspace<mp_complex, std::vector<mp_complex>>(const mp_complex &left, const mp_complex &right, const mp_complex &spacing,
                                                            std::vector<mp_complex> &cont);

template void latspace<std::complex<double>, std::vector<std::complex<double>>>(const std::complex<double> &left,
                                                                                const std::complex<double> &right,
                                                                                const std::complex<double> &spacing,
                                                                                std::vector<std::complex<double>> &cont);
