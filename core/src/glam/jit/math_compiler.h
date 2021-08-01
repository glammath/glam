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

#ifndef GLAMCORE_MATH_COMPILER_H
#define GLAMCORE_MATH_COMPILER_H

#include <wasm.h>
#include <complex>
#include <vector>
#include <emscripten/val.h>
#include "../types.h"
#include "../morphemes.h"
#include "../fxn.h"

class function_visitor;

class module_visitor {
    friend class function_visitor;

    wasm::Module *module = nullptr;
    std::map<std::string, uintptr_t> global_imports;
    std::vector<function_visitor *> children;
    std::string fxn_name;
    std::string parameter_name;
    std::string entry_point;

public:
    module_visitor(const std::string &_name, const std::string &_parameter_name): fxn_name(_name), parameter_name(_parameter_name) { }

    void visit_module();

    function_visitor *visit_function(const std::string &name, wasm::Signature sig);

    void visit_export(const std::string &inner_name, const std::string &outer_name);

    template <typename T> compiled_fxn<T> visit_end();

    void abort();
};

class function_visitor {
    friend class module_visitor;

    enum {
        USES_MPCx1 = 1 << 0, USES_MPCx2 = 1 << 1, GEN_SIMD = 1 << 2, USES_F64x2 = 1 << 3, USES_F64x4 = 1 << 4, USES_UNWRAP = 1 << 5,
        USES_BINARY = 1 << 6, USES_DUPF64 = 1 << 7
    };

    uint32_t flags = 0;

    module_visitor *parent;
    wasm::Function *func;
    std::vector<mp_complex> local_consts; // stored in arena
    uint32_t arena_size = 0;
    bool needs_unwrap = false;

    explicit function_visitor(module_visitor *_parent);

    ~function_visitor() noexcept;

public:
    void visit_entry_point();

    template <typename T> void visit_ptr(T *ptr);

    void visit_basic(wasm::Expression *inst);

    void visit_global_get(const std::string &name);

    void visit_dupi32();

    void visit_dupf64();

    // mpc
    void visit_mpcx2(morpheme_mpcx2 *morph);

    void visit_mpcx1(morpheme_mpcx1 *morph);

    bool visit_variable_mp(const std::string &name);

    void visit_complex(const mp_complex &z);

    // double-precision
    wasm::Index visit_binary();

    void visit_binary_splat(wasm::BinaryOp op);

    bool visit_variable_dp(const std::string &name);

    void visit_unwrap();

    void visit_float(double d);

    void visit_complex(std::complex<double> z);

    void visit_add();

    void visit_sub();

    void visit_mul();

    void visit_div();

    void visit_f64x2(morpheme_f64x2 *morph);

    void visit_f64x4(morpheme_f64x4 *morph);

    std::string visit_end();
};

class math_compiler_dp {
    static std::map<std::string, morpheme_f64x2 *> unary_morphemes;
    static std::map<std::string, morpheme_f64x4 *> binary_morphemes;

    std::string name;
    std::string fxn_name;
    std::string parameter_name;

    void visit_operator(function_visitor *fv, const std::string &op);

public:
    math_compiler_dp(const std::string &_name, const std::string &_fxn_name, const std::string &_parameter_name)
            : name(_name), fxn_name(_fxn_name), parameter_name(_parameter_name) { }

    fxn<std::complex<double>, compiled_fxn<std::complex<double>>> compile(const emscripten::val &stack);
};

#endif //GLAMCORE_MATH_COMPILER_H
