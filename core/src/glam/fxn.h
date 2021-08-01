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

#ifndef GLAMCORE_FXN_H
#define GLAMCORE_FXN_H

#include <string>
#include <vector>
#include "mem/fixed_arena.h"
#include "types.h"
#include "utilities.h"

template <typename T> struct functor_type {
    using type = T(T);
    constexpr static const char *emscripten_type = "ii";
};
template <> struct functor_type<std::complex<double>> {
    using type = std::complex<double> *(double, double);
    constexpr static const char *emscripten_type = "idd";
};

template <> struct functor_type<mp_complex> {
    using type = mp_complex *(mp_complex *);
    constexpr static const char *emscripten_type = "ii";
};

/**
 * Represents a compiled complex math function. For simplicity, we assume the domain and range are the same type.
 * @tparam T type of the domain and range
 * @tparam FxnType
 */
template <typename T, typename FxnType> class fxn {
protected:
    using functor = typename functor_type<T>::type;

    functor *handle;
    fixed_arena<T> *arena;
    std::string name;
    std::string fxn_name;
    std::string parameter_name;
    std::string disassembly;
public:
    fxn(const std::string &_name, const std::string &_fxn_name, const std::string _parameter_name)
            : name(_name), fxn_name(_fxn_name), parameter_name(_parameter_name) { }

    inline T operator()(T z) {
        return static_cast<FxnType *>(this)->operator()(z);
    }

    inline bool ready() {
        return static_cast<FxnType *>(this)->ready();
    }

    inline void release() {
        static_cast<FxnType *>(this)->release();
    }

    std::string get_name() {
        return name;
    }

    std::string get_fxn_name() {
        return fxn_name;
    }

    std::string get_parameter_name() {
        return parameter_name;
    }

    std::string get_disassembly() {
        return disassembly;
    }
};

template <typename T> class compiled_fxn: public fxn<T, compiled_fxn<T>> {
    using module_ptr = void *;
    using functor = typename fxn<T, compiled_fxn<T>>::functor;
public:
    compiled_fxn(const std::string &_name, const std::string &_fxn_name, const std::string &_parameter_name, module_ptr _mod,
                 size_t _mod_len, uint32_t _arena_size): fxn<T, compiled_fxn<T>>(_name, _fxn_name, _parameter_name) {
        this->arena = new fixed_arena<T>(_arena_size);
    }

    void install(module_ptr mod, size_t mod_len);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

    T operator()(T z);

    bool ready();

    void release();

#pragma clang diagnostic pop
};

template <typename T> class virtual_fxn: public fxn<T, virtual_fxn<T>> {
public:
    virtual_fxn(const std::string &_name, const std::string &_fxn_name, const std::string &_parameter_name): fxn<T, virtual_fxn<T>>(_name,
                                                                                                                                    _fxn_name,
                                                                                                                                    _parameter_name) {
        this->disassembly = "; virtual fxn " + _name;
        this->arena = nullptr;
        this->handle = nullptr;
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

    virtual T operator()(T z) = 0;

    bool ready() {
        return true;
    }

    void release() {
        GLAM_TRACE("cannot release virtual fxn!");
    }

#pragma clang diagnostic pop
};

#endif //GLAMCORE_FXN_H
