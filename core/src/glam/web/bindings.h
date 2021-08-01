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

#ifndef GLAMUI_BINDINGS_H
#define GLAMUI_BINDINGS_H

#include <emscripten/val.h>
#include <emscripten.h>

struct js_complex {
    double real;
    double imag;
};

struct js_buffer {
    uintptr_t ptr;
    unsigned long len;
};

template <typename T> struct js_type {
    using type = T;

    static T from(type t) {
        return T(t);
    }
};

template <> struct js_type<mp_float> {
    using type = double;

    static mp_float from(type t) {
        return mp_float(t);
    }
};

template <> struct js_type<mp_complex> {
    using type = js_complex;

    static mp_complex from(type t) {
        return mp_complex(t.real, t.imag);
    }
};

template <> struct js_type<std::complex<double>> {
    using type = js_complex;

    static std::complex<double> from(type t) {
        return std::complex<double>(t.real, t.imag);
    }
};

namespace {
    template <typename T> struct _is_mp: public std::false_type { };
    template <> struct _is_mp<mp_float>: public std::true_type { };
    template <> struct _is_mp<mp_complex>: public std::true_type { };
}

template <typename T> constexpr bool is_mp() {
    return _is_mp<T>::value;
}

template <typename T> T make_mp(typename js_type<T>::type js);

template <> EMSCRIPTEN_KEEPALIVE inline mp_float make_mp<mp_float>(js_type<mp_float>::type x) {
    return mp_float(x);
}

template <> EMSCRIPTEN_KEEPALIVE inline mp_complex make_mp<mp_complex>(js_type<mp_complex>::type x) {
    return mp_complex(x.real, x.imag);
}

#endif //GLAMUI_BINDINGS_H
