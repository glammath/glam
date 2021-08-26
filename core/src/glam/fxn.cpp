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

#include "fxn.h"
#include "jit/globals.h"
#include <emscripten.h>
#include <binaryen-c.h>

template <typename T> void compiled_fxn<T>::install(module_ptr mod, size_t mod_len) {
    // @formatter:off
    auto handle_ptr = EM_ASM_INT({
         const binary = new Uint8Array(wasmMemory.buffer, $0, $1);
         const module = new WebAssembly.Module(binary);
         const instance = new WebAssembly.Instance(module, { env: {
             memory: wasmMemory,
             table: wasmTable,
             _arena: $2,
             _operator_nop1: ((a, b) => 0),
             _operator_nop2: ((a, b, c) => 0),
             _operator_nop4: ((a, b, c, d, e) => 0)
         }});
         const jitFunction = instance.exports[UTF8ToString($3)];
         return addFunction(jitFunction, UTF8ToString($4))
    }, mod, mod_len, this->arena, this->name.c_str(), functor_type<T>::emscripten_type);
    // @formatter:on

    this->handle = reinterpret_cast<functor *>(handle_ptr);
    GLAM_TRACE("installed " << this->name << " at " << this->handle << " = " << handle_ptr);
    globals::fxn_table[this->fxn_name] = handle_ptr;
    auto readModule = BinaryenModuleRead(static_cast<char *>(mod), mod_len);
    auto text = BinaryenModuleAllocateAndWriteText(readModule);
    this->disassembly = text;
    free(text);
    BinaryenModuleDispose(readModule);
    GLAM_TRACE("disassembled module");
}

template <typename T> T compiled_fxn<T>::operator()(T z) {
    T result;
    if constexpr (std::is_same<T, std::complex<double>>()) { // todo this is kind of ugly
        result = *this->handle(z.real(), z.imag());
    } else {
        result = *this->handle(&z);
    }
    this->arena->reset();
    return result;
}

template <typename T> bool compiled_fxn<T>::ready() {
    return this->handle != nullptr;
}

template <typename T> void compiled_fxn<T>::release() {
    GLAM_TRACE("releasing compiled fxn " << this->name);
    this->arena->release();
    delete this->arena;
}

template EMSCRIPTEN_KEEPALIVE void compiled_fxn<mp_complex>::install(module_ptr mod, size_t mod_len);
template EMSCRIPTEN_KEEPALIVE bool compiled_fxn<mp_complex>::ready();
template EMSCRIPTEN_KEEPALIVE void compiled_fxn<mp_complex>::release();
template EMSCRIPTEN_KEEPALIVE mp_complex compiled_fxn<mp_complex>::operator()(mp_complex);

template EMSCRIPTEN_KEEPALIVE void compiled_fxn<std::complex<double>>::install(module_ptr mod, size_t mod_len);
template EMSCRIPTEN_KEEPALIVE bool compiled_fxn<std::complex<double>>::ready();
template EMSCRIPTEN_KEEPALIVE void compiled_fxn<std::complex<double>>::release();
template EMSCRIPTEN_KEEPALIVE std::complex<double> compiled_fxn<std::complex<double>>::operator()(std::complex<double>);
