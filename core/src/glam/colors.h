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

#ifndef GLAM_COLORS_H
#define GLAM_COLORS_H

#include <cstdint>
#include <cmath>
#include "types.h"
#include "web/bindings.h"
#include <algorithm>

/**
 * Uses the Oklab color space, which is described here: https://bottosson.github.io/posts/oklab/
 * Some code is copied from this post, which is available in the public domain.
 */
struct Lab {
    float L;
    float a;
    float b;

    Lab() { }

    /**
     * computes a and b from chroma and hue
     * @param _L luminance
     * @param _C chroma
     * @param _h hue
     */
    Lab(float _L, float _C, float _h): L(_L), a(_C * std::cos(_h)), b(_C * std::sin(_h)) { }

    template <typename T> static Lab from_complex(T complex) {
        std::complex<double> z;
        if constexpr (is_mp<T>()) {
            // we convert to single-precision first because multiprecision atan2 is *much* slower
            z = complex.template convert_to<std::complex<double>>();
        } else {
            z = std::complex<double>(complex);
        }
        const auto argument = static_cast<float>(arg(z) + M_PI_4);
        const auto modulus = static_cast<float>(abs(z));
        const float mod_scaled = modulus / (modulus + 1);
        const float x = 0.3f;

        return Lab(mod_scaled, x - x * 2.f * std::abs(mod_scaled - 0.5f), argument); // NOLINT(modernize-return-braced-init-list)
    }
};

struct rgba {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    rgba() { }

    static float from_linear(float x) {
        if (x <= 0.0031308f) {
            return 12.92f * x;
        }
        return 1.055f * std::pow(x, 1.f / 2.4f) - 0.055f;
    }

    explicit rgba(const Lab &lab, uint8_t alpha) {
        float l = std::pow(lab.L + 0.3963377774f * lab.a + 0.2158037573f * lab.b, 3);
        float m = std::pow(lab.L - 0.1055613458f * lab.a - 0.0638541728f * lab.b, 3);
        float s = std::pow(lab.L - 0.0894841775f * lab.a - 1.2914855480f * lab.b, 3);

        this->r = std::clamp(static_cast<int>(std::round(from_linear(4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s) * 255)),
                             0,
                             255);
        this->g = std::clamp(static_cast<int>(std::round(from_linear(-1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s) * 255)),
                             0,
                             255);
        this->b = std::clamp(static_cast<int>(std::round(from_linear(-0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s) * 255)),
                             0,
                             255);
        this->a = alpha;
    }
};

struct color_buffer {
    uint32_t length;
    rgba *buffer;

    color_buffer() { }

    color_buffer(uint32_t _length): length(_length), buffer(new rgba[length]) { }

    js_buffer get_buffer() {
        return { .ptr = reinterpret_cast<uintptr_t>(buffer), .len = length * 4 };
    }

    uint32_t get_length() {
        return length;
    }
};

#endif //GLAM_COLORS_H
