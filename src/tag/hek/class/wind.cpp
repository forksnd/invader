// SPDX-License-Identifier: GPL-3.0-only

#include "../compile.hpp"

#include "wind.hpp"

namespace Invader::HEK {
    void compile_wind_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Wind);
        FINISH_COMPILE
    }
}
