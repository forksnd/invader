// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__TAG_COLLECTION_HPP
#define INVADER__TAG__HEK__CLASS__TAG_COLLECTION_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    SINGLE_DEPENDENCY_STRUCT(TagCollectionTag, reference); // item

    ENDIAN_TEMPLATE(EndianType) struct TagCollection {
        TagReflexive<EndianType, TagCollectionTag> tags;

        ENDIAN_TEMPLATE(NewType) operator TagCollection<NewType>() const noexcept {
            TagCollection<NewType> copy;
            COPY_THIS(tags);
            return copy;
        }
    };
    static_assert(sizeof(TagCollection<BigEndian>) == 0xC);

    void compile_tag_collection_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
