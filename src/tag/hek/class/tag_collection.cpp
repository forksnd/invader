/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "tag_collection.hpp"

namespace Invader::HEK {
    void compile_tag_collection_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(TagCollection);
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.tags, reference);
        FINISH_COMPILE
    }
}
