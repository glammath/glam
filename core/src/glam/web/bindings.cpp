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

#include <emscripten/bind.h>
#include "../colors.h"
#include "bindings.h"
#include "../morphemes.h"
#include "../multipoint.h"
#include "../jit/math_compiler.h"

EMSCRIPTEN_BINDINGS(glam_module) {
    emscripten::class_<color_buffer>("ColorBuffer").function("getBuffer", &color_buffer::get_buffer)
                                                   .function("getLength", &color_buffer::get_length);

    emscripten::value_array<js_complex>("complex").element(&js_complex::real).element(&js_complex::imag);

    emscripten::value_array<js_buffer>("JSBuffer").element(&js_buffer::ptr).element(&js_buffer::len);

    emscripten::class_<math_compiler_dp>("MathCompilerDP").constructor<std::string, std::string, std::string>()
                                                          .function("compile", &math_compiler_dp::compile);

#define bind_fxn(fxn_type, type, name) emscripten::class_<fxn<type, fxn_type<type>>>(name) \
    .function("ready", &fxn<type, fxn_type<type>>::ready) \
    .function("release", &fxn<type, fxn_type<type>>::release) \
    .function("getName", &fxn<type, fxn_type<type>>::get_name) \
    .function("getFxnName", &fxn<type, fxn_type<type>>::get_fxn_name) \
    .function("getParameterName", &fxn<type, fxn_type<type>>::get_parameter_name) \
    .function("getDisassembly", &fxn<type, fxn_type<type>>::get_disassembly)

    bind_fxn(compiled_fxn, std::complex<double>, "CompiledFxnDP");
    bind_fxn(compiled_fxn, mp_complex, "CompiledFxnMP");
    bind_fxn(virtual_fxn, std::complex<double>, "VirtualFxnDP");
    bind_fxn(virtual_fxn, mp_complex, "VirtualFxnMP");
#undef bind_fxn

#define bind_multipoint(D, R, name) emscripten::class_<multipoint<D, R>>(name) \
    .constructor<fxn<R, compiled_fxn<R>>, js_type<D>::type, js_type<D>::type, uint32_t>()     \
    .function("fullEval", &multipoint<D, R>::full_eval) \
    .function("getValues", &multipoint<D, R>::get_values) \
    .function("getColors", &multipoint<D, R>::get_colors)

    bind_multipoint(mp_float, mp_complex, "RealMultipointMP");
    bind_multipoint(mp_complex, mp_complex, "ComplexMultipointMP");
    bind_multipoint(double, std::complex<double>, "RealMultipointDP");
    bind_multipoint(std::complex<double>, std::complex<double>, "ComplexMultipointDP");

#undef bind_multipoint
}
