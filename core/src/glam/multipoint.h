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

#ifndef GLAM_MULTIPOINT_H
#define GLAM_MULTIPOINT_H

#include "types.h"
#include "colors.h"
#include "fxn.h"
#include <vector>
#include <emscripten/emscripten.h>

/**
 * Represents the result of evaluating a fxn.
 * @tparam D domain of the fxn
 * @tparam R range of the fxn
 */
template <typename D, typename R> class multipoint {
public:
    using _domain_t = D;
    using _range_t = R;
    using functor_t = std::function<R(const D &)>;
    using D_JS = typename js_type<D>::type;

    std::vector<_domain_t> samples;
    std::vector<_range_t> values;
    color_buffer colors;
    uint32_t resolution;
    std::string name;

    void inner_init(const _domain_t &from, const _domain_t &to, uint32_t res);

    functor_t generator;

    /**
     * Constructs a multipoint with the given initial bounds. This has different behavior depending on the template
     * arguments. For real or integer domains, the sample points are constructed as a standard doubly-closed interval.
     * For a complex domain, the left endpoint is taken to be the lower-left corner and the right endpoint the
     * upper-right corner of a rectangle on the complex plane. In that case, the resolution still refers to
     * the number of samples in a unit interval, for example [0,1] or [0,i]
     *
     * @param _name the name of the multipoint in latex format
     * @param from left endpoint
     * @param to right endpoint
     * @param res number of sample points per unit distance
     * @param _generator the generator of the multipoint
     */
    EMSCRIPTEN_KEEPALIVE multipoint(std::string _name, _domain_t from, _domain_t to, uint32_t res, functor_t _generator);

    /**
     * Called from javascript to instantiate a multipoint.
     * @param f the fxn to evaluate
     * @param from lower bound
     * @param to upper bound
     * @param res number of samples per unit distance
     */
    EMSCRIPTEN_KEEPALIVE multipoint(fxn<_range_t, compiled_fxn<_range_t>> f, D_JS from, D_JS to, uint32_t res);

    /**
     * Get a (domain, range) pair on the fxn.
     * @param n the index of the point
     * @return a (D, R) tuple
     */
    EMSCRIPTEN_KEEPALIVE std::pair<_domain_t, _range_t> operator[](size_t n);

    /**
     * Calculate the value of the fxn at each point in the sample vector.
     */
    EMSCRIPTEN_KEEPALIVE void full_eval();

    /**
     * Get an array representing each point in the calculated range of the function, as a javascript Float64Array. Makes a copy only
     * if the range is a multiprecision complex number.
     * @return
     */
    EMSCRIPTEN_KEEPALIVE emscripten::val get_values();

    /**
     * Get an array representing the image data corresponding to the value of the function, as a Uint8Array with every four
     * elements corresponding to (R, G, B, A).
     * @return
     */
    EMSCRIPTEN_KEEPALIVE emscripten::val get_colors();

    /**
     * Resizes the multipoint bounds. Will resize the `values` vector as well, computing any new values required
     * for the specified resolution. If not specified, the resolution is taken to be the initial resolution.
     *
     * @param from left endpoint
     * @param to right endpoint
     * @param res number of sample points per unit distance
     */
    EMSCRIPTEN_KEEPALIVE void resize(const _domain_t &from, const _domain_t &to, uint32_t res = 0);
};

#endif //GLAM_MULTIPOINT_H
