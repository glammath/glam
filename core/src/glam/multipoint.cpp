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

#include "multipoint.h"

#include <utility>
#include <emscripten/bind.h>
#include "utilities.h"
#include "web/bindings.h"
#include "colors.h"

template <typename D, typename R> EMSCRIPTEN_KEEPALIVE multipoint<D, R>::multipoint(std::string _name, _domain_t from, _domain_t to,
                                                                                    uint32_t res, functor_t _generator)
        : name(std::move(_name)), resolution(res), generator(std::move(_generator)) {
    inner_init(from, to, res);
}

template <typename D, typename R> EMSCRIPTEN_KEEPALIVE multipoint<D, R>::multipoint(fxn<_range_t, compiled_fxn<_range_t>> f,
                                                                                    multipoint::D_JS from, multipoint::D_JS to,
                                                                                    uint32_t res)
        : multipoint(f.get_fxn_name(), js_type<D>::from(from), js_type<D>::from(to), res, [f, this, i = 0](const D &z) mutable {
    auto result = f(z);
    colors.buffer[i++] = rgba(Lab::from_complex<R>(result), 0xff);
    return result;
}) {
    GLAM_TRACE("constructed multipoint for " << f.get_name());
}

template EMSCRIPTEN_KEEPALIVE multipoint<double, std::complex<double>>::multipoint(fxn<_range_t, compiled_fxn<_range_t>> f, D_JS from,
                                                                                   D_JS to, uint32_t res);

template EMSCRIPTEN_KEEPALIVE multipoint<std::complex<double>, std::complex<double>>::multipoint(fxn<_range_t, compiled_fxn<_range_t>> f,
                                                                                                 D_JS from, D_JS to, uint32_t res);

template EMSCRIPTEN_KEEPALIVE multipoint<mp_float, mp_complex>::multipoint(fxn<_range_t, compiled_fxn<_range_t>> f, D_JS from, D_JS to,
                                                                           uint32_t res);

template EMSCRIPTEN_KEEPALIVE multipoint<mp_complex, mp_complex>::multipoint(fxn<_range_t, compiled_fxn<_range_t>> f, D_JS from, D_JS to,
                                                                             uint32_t res);

template <typename D, typename R> EMSCRIPTEN_KEEPALIVE std::pair<D, R> multipoint<D, R>::operator[](size_t n) {
    return std::make_pair(this->samples[n], this->values[n]);
}

template <typename D, typename R> EMSCRIPTEN_KEEPALIVE void multipoint<D, R>::full_eval() {
    std::transform(samples.begin(), samples.end(), std::back_inserter(values), [&](auto z) {
        return this->generator(z);
    });
}

template <typename D, typename R> EMSCRIPTEN_KEEPALIVE emscripten::val multipoint<D, R>::get_values() {
    if constexpr (is_mp<R>()) {
        auto converted = new js_complex[this->values.size()];
        GLAM_TRACE("converting values to JS");
        std::transform(this->values.begin(), this->values.end(), converted, [](const R &z) {
            return js_complex{ .real = z.real().template convert_to<double>(), .imag = z.imag().template convert_to<double>() };
        });
        return emscripten::val(emscripten::typed_memory_view(this->values.size() * 2, reinterpret_cast<double *>(converted)));
    } else {
        // if we use double-precision values, we can share the memory buffer directly with JS
        std::complex<double> *ptr = &(*this->values.begin());
        return emscripten::val(emscripten::typed_memory_view(this->values.size() * 2, reinterpret_cast<double *>(ptr)));
    }
}

template <typename D, typename R> EMSCRIPTEN_KEEPALIVE emscripten::val multipoint<D, R>::get_colors() {
    return emscripten::val(emscripten::typed_memory_view(colors.length * 4, reinterpret_cast<uint8_t *>(colors.buffer)));
}

template <> EMSCRIPTEN_KEEPALIVE void multipoint<mp_float, mp_complex>::inner_init(const _domain_t &from, const _domain_t &to,
                                                                                   uint32_t res) {
    GLAM_TRACE("inner init mp (R->C)");
    samples = std::vector<_domain_t>(boost::multiprecision::ceil((to - from) * res).convert_to<size_t>());
    linspace(from, to, samples);
    this->colors = color_buffer(samples.size());
    GLAM_TRACE("initialized with " << samples.size() << " samples.");
}

template <> EMSCRIPTEN_KEEPALIVE void multipoint<mp_complex, mp_complex>::inner_init(const _domain_t &from, const _domain_t &to,
                                                                                     uint32_t res) {
    GLAM_TRACE("inner init mp (C->C)");
    auto dim = (to - from).convert_to<_domain_t>();
    samples = std::vector<_domain_t>(boost::multiprecision::ceil(dim.real() * dim.imag() * res * res).convert_to<size_t>());
    latspace(from, to, _domain_t(1. / res, 1. / res), samples);
    this->colors = color_buffer(samples.size());
    GLAM_TRACE("initialized with " << samples.size() << " samples.");
}

template <> EMSCRIPTEN_KEEPALIVE void multipoint<double, std::complex<double>>::inner_init(const _domain_t &from, const _domain_t &to,
                                                                                           uint32_t res) {
    GLAM_TRACE("inner init dp (R->C)");
    samples = std::vector<_domain_t>(static_cast<size_t>(std::ceil(to - from) * res));
    linspace(from, to, samples);
    this->colors = color_buffer(samples.size());
    GLAM_TRACE("initialized with " << samples.size() << " samples.");
}

template <> EMSCRIPTEN_KEEPALIVE void multipoint<std::complex<double>, std::complex<double>>::inner_init(const _domain_t &from,
                                                                                                         const _domain_t &to,
                                                                                                         uint32_t res) {
    GLAM_TRACE("inner init dp (C->C)");
    auto dim = to - from;
    samples = std::vector<_domain_t>(static_cast<size_t>(ceil(dim.real() * dim.imag() * res * res)));
    latspace(from, to, _domain_t(1. / res, 1. / res), samples);
    this->colors = color_buffer(samples.size());
    GLAM_TRACE("initialized with " << samples.size() << " samples.");
}

template <> void multipoint<mp_float, mp_complex>::resize(const _domain_t &from, const _domain_t &to, uint32_t res) {

}

template <> void multipoint<mp_complex, mp_complex>::resize(const _domain_t &from, const _domain_t &to, uint32_t res) {

}
