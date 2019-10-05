// SPDX-License-Identifier: GPL-3.0-only

#include "../compile.hpp"

#include "garbage.hpp"

namespace Invader::HEK {
    void compile_garbage_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Garbage)
        COMPILE_ITEM_DATA
        tag.object_type = OBJECT_TYPE_GARBAGE;
        FINISH_COMPILE
    }
}
