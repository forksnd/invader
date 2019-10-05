// SPDX-License-Identifier: GPL-3.0-only

#include "../compile.hpp"
#include "shader_transparent_chicago.hpp"

namespace Invader::HEK {
    void compile_shader_transparent_chicago_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ShaderTransparentChicago);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.lens_flare);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.extra_layers, shader);
        ADD_CHICAGO_MAP(tag.maps);
        tag.shader_type = SHADER_TYPE_SHADER_TRANSPARENT_CHICAGO;
        FINISH_COMPILE
    }
}
