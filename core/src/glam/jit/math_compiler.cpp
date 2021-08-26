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

#include "math_compiler.h"
#include <wasm-stack.h>
#include <binaryen-c.h>
#include "globals.h"

#define GLAM_COMPILER_TRACE(msg) GLAM_TRACE("[compiler] " << msg)

void module_visitor::visit_module() {
    GLAM_COMPILER_TRACE("visit_module");
    this->module = BinaryenModuleCreate();
    // we import memory and table from the core module so we can indirect-call functions
    BinaryenAddMemoryImport(module, "memory", "env", "memory", 0);
    BinaryenAddTableImport(module, "table", "env", "table");
}

void module_visitor::visit_export(const std::string &inner_name, const std::string &outer_name) {
    GLAM_COMPILER_TRACE("visit_export " << inner_name << " -> " << outer_name);
    auto exp = new wasm::Export;
    exp->name = outer_name;
    exp->value = inner_name;
    exp->kind = wasm::ExternalKind::Function;
    module->addExport(exp);
}

function_visitor *module_visitor::visit_function(const std::string &name, wasm::Signature sig) {
    GLAM_COMPILER_TRACE("visit_function " << name);
    auto fv = new function_visitor(this);
    this->children.push_back(fv);
    fv->parent = this;
    fv->func->name = name;
    fv->func->type = sig;
    return fv;
}

template <typename T> compiled_fxn<T> module_visitor::visit_end() {
    GLAM_COMPILER_TRACE("visit_end module");
    uint32_t globalFlags = 0;
    uint32_t totalArenaSize = 0;
    std::for_each(children.begin(), children.end(), [&](function_visitor *fv) {
        globalFlags |= fv->flags;
        totalArenaSize += fv->arena_size;
    });

    // add fake function imports so binaryen is aware of the function types. increases binary size but i'm not sure
    // of a better way to do this (using stack IR).
    if (globalFlags & function_visitor::USES_MPCx1) {
        GLAM_COMPILER_TRACE("uses mpcx1");
        BinaryenType ii[2] = { BinaryenTypeInt32(), BinaryenTypeInt32() };
        auto fakeType = BinaryenTypeCreate(ii, 2);
        BinaryenAddFunctionImport(module, "_operator_nop1", "env", "_operator_nop1", fakeType, BinaryenTypeInt32());
    }

    if (globalFlags & function_visitor::USES_MPCx2) {
        GLAM_COMPILER_TRACE("uses mpcx2");
        BinaryenType iii[3] = { BinaryenTypeInt32(), BinaryenTypeInt32(), BinaryenTypeInt32() };
        auto fakeType = BinaryenTypeCreate(iii, 3);
        BinaryenAddFunctionImport(module, "_operator_nop2", "env", "_operator_nop2", fakeType, BinaryenTypeInt32());
    }

    if (globalFlags & function_visitor::USES_F64x2) {
        GLAM_COMPILER_TRACE("uses f64x2");
        BinaryenType ddi[3] = { BinaryenTypeFloat64(), BinaryenTypeFloat64(), BinaryenTypeInt32() };
        auto fakeType = BinaryenTypeCreate(ddi, 3);
        BinaryenAddFunctionImport(module, "_operator_nop2d", "env", "_operator_nop2", fakeType, BinaryenTypeInt32());
    }

    if (globalFlags & function_visitor::USES_F64x4) {
        GLAM_COMPILER_TRACE("uses f64x4");
        BinaryenType ddddi[5] = { BinaryenTypeFloat64(), BinaryenTypeFloat64(), BinaryenTypeFloat64(), BinaryenTypeFloat64(),
                BinaryenTypeInt32() };
        auto fakeType = BinaryenTypeCreate(ddddi, 5);
        BinaryenAddFunctionImport(module, "_operator_nop4", "env", "_operator_nop4", fakeType, BinaryenTypeInt32());
    }

    if (globalFlags &
        (function_visitor::USES_F64x2 | function_visitor::USES_F64x4 | function_visitor::USES_MPCx1 | function_visitor::USES_MPCx2)) {
        GLAM_COMPILER_TRACE("uses arena");
        BinaryenAddGlobalImport(module, "_arena", "env", "_arena", wasm::Type::i32, false);
    }

    auto result = BinaryenModuleAllocateAndWrite(module, nullptr);
    compiled_fxn<T> fxn(entry_point, fxn_name, parameter_name, result.binary, result.binaryBytes, totalArenaSize);
    BinaryenModuleDispose(module);

    std::for_each(children.begin(), children.end(), [&](function_visitor *fv) {
        delete fv;
    });

    fxn.install(result.binary, result.binaryBytes);
    free(result.binary);

    GLAM_COMPILER_TRACE("compilation complete");
    return fxn;
}

void module_visitor::abort() {
    GLAM_COMPILER_TRACE("mv: aborting early");
    std::for_each(children.begin(), children.end(), [&](function_visitor *fv) {
        delete fv;
    });
    BinaryenModuleDispose(module);
}

function_visitor::function_visitor(module_visitor *_parent): parent(_parent) {
    this->func = new wasm::Function;
    auto nop = parent->module->allocator.alloc<wasm::Nop>();
    nop->type = wasm::Type::none;
    this->func->body = nop;
    this->func->stackIR = std::make_unique<wasm::StackIR>();
}

void function_visitor::visit_entry_point() {
    GLAM_COMPILER_TRACE("visit_entry_point " << func->name.str);
    parent->entry_point = func->name.str;
}

void function_visitor::visit_float(double d) {
    GLAM_COMPILER_TRACE("visit_float " << d);
    auto c = parent->module->allocator.alloc<wasm::Const>();
    c->type = wasm::Type::f64;
    c->value = wasm::Literal(d);
    visit_basic(c);
}

void function_visitor::visit_complex(std::complex<double> z) {
    visit_float(z.real());
    visit_float(z.imag());
}

void function_visitor::visit_basic(wasm::Expression *inst) {
    inst->finalize();
    auto si = parent->module->allocator.alloc<wasm::StackInst>();
    si->op = wasm::StackInst::Basic;
    si->type = inst->type;
    si->origin = inst;
    func->stackIR->push_back(si);
}

void function_visitor::visit_complex(const mp_complex &z) {
    auto ptr = std::find(local_consts.begin(), local_consts.end(), z);
    if (ptr == local_consts.end()) {
        local_consts.push_back(z);
    }
    visit_global_get("_complex_" + std::to_string(ptr - local_consts.begin()));
}

template <typename T> void function_visitor::visit_ptr(T *ptr) {
    auto c = parent->module->allocator.alloc<wasm::Const>();
    c->type = wasm::Type::i32;
    c->value = wasm::Literal(static_cast<int32_t>(reinterpret_cast<uintptr_t>(ptr)));
    visit_basic(c);
}

void function_visitor::visit_global_get(const std::string &name) {
    auto globalGet = parent->module->allocator.alloc<wasm::GlobalGet>();
    globalGet->type = wasm::Type::i32;
    globalGet->name = name;
}

void function_visitor::visit_mpcx2(morpheme_mpcx2 *morph) {
    GLAM_COMPILER_TRACE("visit_mpcx2");
    arena_size += 2;
    flags |= USES_MPCx2;
    auto globalGet = parent->module->allocator.alloc<wasm::GlobalGet>();
    globalGet->type = wasm::Type::i32;
    globalGet->name = "_arena";
    visit_basic(globalGet);

    visit_ptr(morph);

    auto callIndirect = parent->module->allocator.alloc<wasm::CallIndirect>();
    callIndirect->sig = wasm::Signature(wasm::Type({ wasm::Type::i32, wasm::Type::i32, wasm::Type::i32 }), wasm::Type::i32); // (iii)->i
    callIndirect->table = "table";
    callIndirect->isReturn = false;
    callIndirect->type = wasm::Type::i32;
    visit_basic(callIndirect);
}

void function_visitor::visit_mpcx1(morpheme_mpcx1 *morph) {
    GLAM_COMPILER_TRACE("visit_mpcx1");
    arena_size++;
    flags |= USES_MPCx1;
    auto globalGet = parent->module->allocator.alloc<wasm::GlobalGet>();
    globalGet->type = wasm::Type::i32;
    globalGet->name = "_arena";
    visit_basic(globalGet);

    visit_ptr(morph);

    auto callIndirect = parent->module->allocator.alloc<wasm::CallIndirect>();
    callIndirect->sig = wasm::Signature(wasm::Type({ wasm::Type::i32, wasm::Type::i32 }), wasm::Type::i32); // (ii)->i
    callIndirect->table = "table";
    callIndirect->isReturn = false;
    callIndirect->type = wasm::Type::i32;
    visit_basic(callIndirect);
}

bool function_visitor::visit_variable_mp(const std::string &name) {
    GLAM_COMPILER_TRACE("visit_variable_mp " << name);
    if (name == parent->parameter_name) {
        auto localGet = parent->module->allocator.alloc<wasm::LocalGet>();
        localGet->type = wasm::Type::i32;
        localGet->index = 0;
        visit_basic(localGet);
        return true;
    } else {
        auto ptr = globals::consts_mp.find(name);
        if (ptr == globals::consts_mp.end()) {
            return false;
        } else {
            visit_ptr(ptr->second);
            return true;
        }
    }
}

void function_visitor::visit_binary_splat(wasm::BinaryOp op) {
    auto localSetD = parent->module->allocator.alloc<wasm::LocalSet>();
    localSetD->index = wasm::Builder::addVar(func, wasm::Type::f64);
    visit_basic(localSetD);

    auto localSetC = parent->module->allocator.alloc<wasm::LocalSet>();
    localSetC->index = wasm::Builder::addVar(func, wasm::Type::f64);
    visit_basic(localSetC);

    auto localSetB = parent->module->allocator.alloc<wasm::LocalSet>();
    localSetB->index = wasm::Builder::addVar(func, wasm::Type::f64);
    visit_basic(localSetB);

    auto localGetC = parent->module->allocator.alloc<wasm::LocalGet>();
    localGetC->index = localSetC->index;
    localGetC->type = wasm::Type::f64;
    visit_basic(localGetC);

    auto inst = parent->module->allocator.alloc<wasm::Binary>();
    inst->type = wasm::Type::f64;
    inst->op = op;
    visit_basic(inst);

    auto localGetB = parent->module->allocator.alloc<wasm::LocalGet>();
    localGetB->index = localSetB->index;
    localGetB->type = wasm::Type::f64;
    visit_basic(localGetB);

    auto localGetD = parent->module->allocator.alloc<wasm::LocalGet>();
    localGetD->index = localSetD->index;
    localGetD->type = wasm::Type::f64;
    visit_basic(localGetD);

    visit_basic(inst);
}

void function_visitor::visit_add() {
    GLAM_COMPILER_TRACE("visit_add (dp)");
    if (flags & GEN_SIMD) {

    } else {
        visit_unwrap();
        visit_binary_splat(wasm::AddFloat64);
    }
}

void function_visitor::visit_sub() {
    GLAM_COMPILER_TRACE("visit_sub (dp)");
    if (flags & GEN_SIMD) {

    } else {
        visit_unwrap();
        visit_binary_splat(wasm::SubFloat64);
    }
}

void function_visitor::visit_mul() {
    GLAM_COMPILER_TRACE("visit_mul (dp)");
    if (flags & GEN_SIMD) {

    } else {
        visit_unwrap();
        wasm::Index i = visit_binary();

        auto localGetC = parent->module->allocator.alloc<wasm::LocalGet>();
        localGetC->index = i + 1;
        localGetC->type = wasm::Type::f64;

        auto mul = parent->module->allocator.alloc<wasm::Binary>();
        mul->type = wasm::Type::f64;
        mul->op = wasm::MulFloat64;

        auto localGetB = parent->module->allocator.alloc<wasm::LocalGet>();
        localGetB->index = i + 2;
        localGetB->type = wasm::Type::f64;

        auto localGetA = parent->module->allocator.alloc<wasm::LocalGet>();
        localGetA->index = i + 3;
        localGetA->type = wasm::Type::f64;

        auto localGetD = parent->module->allocator.alloc<wasm::LocalGet>();
        localGetD->index = i;
        localGetD->type = wasm::Type::f64;

        auto add = parent->module->allocator.alloc<wasm::Binary>();
        add->type = wasm::Type::f64;
        add->op = wasm::AddFloat64;

        auto sub = parent->module->allocator.alloc<wasm::Binary>();
        sub->type = wasm::Type::f64;
        sub->op = wasm::SubFloat64;


        visit_basic(localGetC);
        visit_basic(mul);
        visit_basic(localGetB);
        visit_basic(localGetD);
        visit_basic(mul);
        visit_basic(sub);

        visit_basic(localGetA);
        visit_basic(localGetD);
        visit_basic(mul);
        visit_basic(localGetB);
        visit_basic(localGetC);
        visit_basic(mul);
        visit_basic(add);
    }
}

void function_visitor::visit_div() {
    GLAM_COMPILER_TRACE("visit_div (dp)");
    if (flags & GEN_SIMD) {

    } else {
        // we use a morpheme here because division is hard
        visit_f64x4(&_fmorpheme_div);
    }
}

void function_visitor::visit_f64x2(morpheme_f64x2 *morph) {
    GLAM_COMPILER_TRACE("visit_f64x2");
    visit_unwrap();
    arena_size++;
    flags |= USES_F64x2;

    auto globalGet = parent->module->allocator.alloc<wasm::GlobalGet>();
    globalGet->name = "_arena";
    globalGet->type = wasm::Type::i32;
    visit_basic(globalGet);

    visit_ptr(morph);

    auto callIndirect = parent->module->allocator.alloc<wasm::CallIndirect>();
    callIndirect->type = wasm::Type::i32;
    callIndirect->sig = wasm::Signature(wasm::Type({ wasm::Type::f64, wasm::Type::f64, wasm::Type::i32 }), wasm::Type::i32); // (ddi)->i
    callIndirect->table = "table";
    callIndirect->isReturn = false;
    visit_basic(callIndirect);
    needs_unwrap = true;
}

wasm::Index function_visitor::visit_binary() {
    flags |= USES_BINARY;
    // (a, b) (c, d)
    wasm::Index var = wasm::Builder::addVar(func, wasm::Type::f64);
    auto localSet1 = parent->module->allocator.alloc<wasm::LocalSet>();
    localSet1->index = var;
    visit_basic(localSet1);

    auto localSet2 = parent->module->allocator.alloc<wasm::LocalSet>();
    localSet2->index = wasm::Builder::addVar(func, wasm::Type::f64);
    visit_basic(localSet2);

    auto localSet3 = parent->module->allocator.alloc<wasm::LocalSet>();
    localSet3->index = wasm::Builder::addVar(func, wasm::Type::f64);
    visit_basic(localSet3);

    auto localSet4 = parent->module->allocator.alloc<wasm::LocalSet>();
    localSet4->index = wasm::Builder::addVar(func, wasm::Type::f64);
    localSet4->type = wasm::Type::f64;
    visit_basic(localSet4);

    return var;
}

void function_visitor::visit_unwrap() {
    if (needs_unwrap) {
        GLAM_COMPILER_TRACE("visit_unwrap");
        this->flags |= USES_UNWRAP;
        visit_dupi32();

        auto loadIm = parent->module->allocator.alloc<wasm::Load>();
        loadIm->type = wasm::Type::f64;
        loadIm->offset = 8; // get the imaginary part first
        loadIm->bytes = 8;
        loadIm->isAtomic = false;
        visit_basic(loadIm);

        auto localSet = parent->module->allocator.alloc<wasm::LocalSet>();
        localSet->index = wasm::Builder::addVar(func, wasm::Type::f64);
        visit_basic(localSet);

        auto loadRe = parent->module->allocator.alloc<wasm::Load>();
        loadRe->type = wasm::Type::f64;
        loadRe->offset = 0;
        loadRe->bytes = 8;
        loadRe->isAtomic = false;
        visit_basic(loadRe);

        auto localGet1 = parent->module->allocator.alloc<wasm::LocalGet>();
        localGet1->index = localSet->index;
        localGet1->type = wasm::Type::f64;
        visit_basic(localGet1);
        needs_unwrap = false;
    }
}

void function_visitor::visit_f64x4(morpheme_f64x4 *morph) {
    GLAM_COMPILER_TRACE("visit_f64x4");
    visit_unwrap();
    arena_size++;
    flags |= USES_F64x4;

    auto globalGet = parent->module->allocator.alloc<wasm::GlobalGet>();
    globalGet->name = "_arena";
    globalGet->type = wasm::Type::i32;
    visit_basic(globalGet);

    visit_ptr(morph);

    auto callIndirect = parent->module->allocator.alloc<wasm::CallIndirect>();
    callIndirect->type = wasm::Type::i32;
    callIndirect->sig = wasm::Signature(wasm::Type({ wasm::Type::f64, wasm::Type::f64, wasm::Type::f64, wasm::Type::f64, wasm::Type::i32 }),
                                        wasm::Type::i32); // (ddddi)->i
    callIndirect->table = "table";
    callIndirect->isReturn = false;
    visit_basic(callIndirect);
    needs_unwrap = true;
}

void function_visitor::visit_fxncall(const std::string &name) {
    uintptr_t ptr = globals::fxn_table[name];
    assert(ptr); // should never fail, the parser checks first
    GLAM_COMPILER_TRACE("visit_fxncall " << name << " @ " << ptr);

    auto c = parent->module->allocator.alloc<wasm::Const>();
    c->type = wasm::Type::i32;
    c->value = wasm::Literal(static_cast<uint32_t>(ptr));
    visit_basic(c);

    // todo for now we assume that it's also a double-precision fxn, i.e. it is (f64, f64)->i32
    auto callIndirect = parent->module->allocator.alloc<wasm::CallIndirect>();
    callIndirect->isReturn = false;
    callIndirect->sig = wasm::Signature({ wasm::Type::f64, wasm::Type::f64 }, wasm::Type::i32);
    callIndirect->table = "table";
    callIndirect->type = wasm::Type::i32;
    visit_basic(callIndirect);
    needs_unwrap = true;
}

bool function_visitor::visit_variable_dp(const std::string &name) {
    GLAM_COMPILER_TRACE("visit_variable_dp " << name);
    visit_unwrap();
    if (name == parent->parameter_name) {
        auto localGet = parent->module->allocator.alloc<wasm::LocalGet>();
        localGet->type = wasm::Type::f64;
        localGet->index = 0;
        visit_basic(localGet);

        localGet = parent->module->allocator.alloc<wasm::LocalGet>();
        localGet->type = wasm::Type::f64;
        localGet->index = 1;
        visit_basic(localGet);
        return true;
    } else {
        auto z = globals::consts_dp.find(name);
        if (z == globals::consts_dp.end()) {
            return false;
        } else {
            visit_complex(z->second);
            return true;
        }
    }
}

void function_visitor::visit_dupi32() {
    // wasm doesn't have a dup opcode so this is what we have to do
    auto localTee = parent->module->allocator.alloc<wasm::LocalSet>();
    localTee->type = wasm::Type::i32;
    localTee->index = wasm::Builder::addVar(func, wasm::Type::i32);
    visit_basic(localTee);
    auto localGet0 = parent->module->allocator.alloc<wasm::LocalGet>();
    localGet0->type = wasm::Type::i32;
    localGet0->index = localTee->index;
    visit_basic(localGet0);
}

void function_visitor::visit_dupf64() {
    flags |= USES_DUPF64;
    auto localTee = parent->module->allocator.alloc<wasm::LocalSet>();
    localTee->type = wasm::Type::f64;
    localTee->index = wasm::Builder::addVar(func, wasm::Type::f64);
    visit_basic(localTee);
    auto localGet0 = parent->module->allocator.alloc<wasm::LocalGet>();
    localGet0->type = wasm::Type::f64;
    localGet0->index = localTee->index;
    visit_basic(localGet0);
}

function_visitor::~function_visitor() noexcept {
    delete this->func;
}

std::string function_visitor::visit_end() {
    GLAM_COMPILER_TRACE("visit_end function");
    if (!needs_unwrap) {
        // now we actually need to wrap
        GLAM_COMPILER_TRACE("wrapping complex");
        visit_f64x2(&_fmorpheme_wrap);
    }

    auto ret = parent->module->allocator.alloc<wasm::Return>();
    visit_basic(ret);

    parent->module->addFunction(func);
    return func->name.c_str();
}

std::map<std::string, morpheme_f64x2 *> math_compiler_dp::unary_morphemes = { std::make_pair("sin", &_fmorpheme_sin),
        std::make_pair("cos", &_fmorpheme_cos), std::make_pair("tan", &_fmorpheme_tan), std::make_pair("sinh", &_fmorpheme_sinh),
        std::make_pair("cosh", &_fmorpheme_cosh), std::make_pair("tanh", &_fmorpheme_tanh) };

std::map<std::string, morpheme_f64x4 *> math_compiler_dp::binary_morphemes = { std::make_pair("^", &_fmorpheme_exp) };

void math_compiler_dp::visit_operator(function_visitor *fv, const std::string &op) {
    GLAM_COMPILER_TRACE("compiling operator " << op);
    if (op == "+") {
        fv->visit_add();
        return;
    } else if (op == "-") {
        fv->visit_sub();
        return;
    } else if (op == "*") {
        fv->visit_mul();
        return;
    } else if (op == "/") {
        fv->visit_div();
        return;
    } else {
        auto iter1 = unary_morphemes.find(op);
        if (iter1 != unary_morphemes.end()) {
            fv->visit_f64x2(*iter1->second);
            return;
        } else {
            auto iter2 = binary_morphemes.find(op);
            if (iter2 != binary_morphemes.end()) {
                fv->visit_f64x4(*iter2->second);
                return;
            }
        }
    }
    GLAM_COMPILER_TRACE("unrecognized operator");
    abort();
}

fxn<std::complex<double>, compiled_fxn<std::complex<double>>> math_compiler_dp::compile(const emscripten::val &stack) {
    module_visitor mv(fxn_name, parameter_name);
    mv.visit_module();
    auto fv = mv.visit_function(name, wasm::Signature({ wasm::Type::f64, wasm::Type::f64 }, wasm::Type::i32));
    const auto len = stack["length"].as<size_t>();
    assert(len > 0);
    for (size_t i = 0; i < len; i++) {
        emscripten::val stackObj = stack[i];
        auto type = stackObj["type"].as<int32_t>();
        auto value = stackObj["value"].as<std::string>();
        switch (type) {
            case 0: // NUMBER
                // we take advantage of boost's parsing even if we don't want a multiprecision complex number
                fv->visit_complex(mp_complex(value).convert_to<std::complex<double>>());
                break;
            case 1: // IDENTIFIER
                if (!fv->visit_variable_dp(value)) {
                    mv.abort();
                    abort();
                }
                break;
            case 2: // OPERATOR
                visit_operator(fv, value);
                break;
            case 3: // FXNCALL
                fv->visit_fxncall(value);
                break;
            default:
                GLAM_COMPILER_TRACE("unrecognized stack object " << type);
                mv.abort();
                abort();
        }
    }

    fv->visit_entry_point();
    auto f_name = fv->visit_end();
    mv.visit_export(f_name, name);
    return mv.visit_end<std::complex<double>>();
}
