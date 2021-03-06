/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "flag.hpp"

namespace Invader::HEK {
    void compile_flag_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Flag)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.red_flag_shader);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.physics);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.blue_flag_shader);
        ADD_REFLEXIVE(tag.attachment_points);
        FINISH_COMPILE
    }
}
