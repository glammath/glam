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

#ifndef GLAM_MORPHEMES_H
#define GLAM_MORPHEMES_H

#include "types.h"
#include "mem/fixed_arena.h"
#include <complex>
#include <emscripten.h>

#define DEFINE_MPCx1(name) EMSCRIPTEN_KEEPALIVE mp_complex *_morpheme_ ## name(mp_complex *a, fixed_arena<mp_complex> *arena)
#define DEFINE_MPCx2(name) EMSCRIPTEN_KEEPALIVE mp_complex *_morpheme_ ## name(mp_complex *a, mp_complex *b, fixed_arena<mp_complex> *arena)
#define DEFINE_f64x2(name) EMSCRIPTEN_KEEPALIVE std::complex<double> *_fmorpheme_ ## name(double a, double b, fixed_arena<std::complex<double>> *arena)
#define DEFINE_f64x4(name) EMSCRIPTEN_KEEPALIVE std::complex<double> *_fmorpheme_ ## name(double a, double b, double c, double d, fixed_arena<std::complex<double>> *arena)

using morpheme_mpcx1 = mp_complex *(mp_complex *, fixed_arena<mp_complex> *);
using morpheme_mpcx2 = mp_complex *(mp_complex *, mp_complex *, fixed_arena<mp_complex> *);
using morpheme_f64x2 = std::complex<double> *(double, double, fixed_arena<std::complex<double>> *);
using morpheme_f64x4 = std::complex<double> *(double, double, double, double, fixed_arena<std::complex<double>> *);

DEFINE_MPCx2(add);

DEFINE_MPCx2(sub);

DEFINE_MPCx2(mul);

DEFINE_MPCx2(div);

DEFINE_MPCx2(exp);

DEFINE_MPCx1(sin);

DEFINE_MPCx1(cos);

DEFINE_MPCx1(tan);

DEFINE_MPCx1(sinh);

DEFINE_MPCx1(cosh);

DEFINE_MPCx1(tanh);

DEFINE_f64x4(div);

DEFINE_f64x4(exp);

DEFINE_f64x2(wrap);

DEFINE_f64x2(sin);

DEFINE_f64x2(cos);

DEFINE_f64x2(tan);

DEFINE_f64x2(sinh);

DEFINE_f64x2(cosh);

DEFINE_f64x2(tanh);

#endif //GLAM_MORPHEMES_H
