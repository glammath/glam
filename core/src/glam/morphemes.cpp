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

#include "morphemes.h"

DEFINE_MPCx2(add) {
    auto r = arena->alloc();
    *r = *a + *b;
    return r;
}

DEFINE_MPCx2(mul) {
    auto r = arena->alloc();
    *r = *a * *b;
    return r;
}

DEFINE_MPCx2(sub) {
    auto r = arena->alloc();
    *r = *a - *b;
    return r;
}

DEFINE_MPCx2(div) {
    auto r = arena->alloc();
    *r = *a / *b;
    return r;
}

DEFINE_MPCx2(exp) {
    auto r = arena->alloc();
    *r = boost::multiprecision::pow(*a, *b);
    return r;
}

DEFINE_MPCx1(sin) {
    auto r = arena->alloc();
    *r = boost::multiprecision::sin(*a);
    return r;
}

DEFINE_MPCx1(cos) {
    auto r = arena->alloc();
    *r = boost::multiprecision::cos(*a);
    return r;
}

DEFINE_MPCx1(tan) {
    auto r = arena->alloc();
    *r = boost::multiprecision::tan(*a);
    return r;
}

DEFINE_MPCx1(sinh) {
    auto r = arena->alloc();
    *r = boost::multiprecision::sin(*a);
    return r;
}

DEFINE_MPCx1(cosh) {
    auto r = arena->alloc();
    *r = boost::multiprecision::cos(*a);
    return r;
}

DEFINE_MPCx1(tanh) {
    auto r = arena->alloc();
    *r = boost::multiprecision::tan(*a);
    return r;
}

DEFINE_MPCx1(conj) {
    auto r = arena->alloc();
    *r = mp_complex(a->real(), -a->imag());
    return r;
}

DEFINE_f64x4(div) {
    auto r = arena->alloc();
    *r = std::complex(a, b) / std::complex(c, d);
    return r;
}

DEFINE_f64x4(exp) {
    auto r = arena->alloc();
    *r = std::pow(std::complex(a, b), std::complex(c, d));
    return r;
}

DEFINE_f64x2(wrap) {
    auto r = arena->alloc();
    *r = std::complex(a, b);
    return r;
}

DEFINE_f64x2(sin) {
    auto r = arena->alloc();
    *r = sin(std::complex(a, b));
    return r;
}

DEFINE_f64x2(cos) {
    auto r = arena->alloc();
    *r = cos(std::complex(a, b));
    return r;

}

DEFINE_f64x2(tan) {
    auto r = arena->alloc();
    *r = tan(std::complex(a, b));
    return r;

}

DEFINE_f64x2(sinh) {
    auto r = arena->alloc();
    *r = sinh(std::complex(a, b));
    return r;

}

DEFINE_f64x2(cosh) {
    auto r = arena->alloc();
    *r = cosh(std::complex(a, b));
    return r;

}

DEFINE_f64x2(tanh) {
    auto r = arena->alloc();
    *r = tanh(std::complex(a, b));
    return r;
}
