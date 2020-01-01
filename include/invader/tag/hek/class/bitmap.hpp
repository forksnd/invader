// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__BITMAP_HPP
#define INVADER__TAG__HEK__CLASS__BITMAP_HPP

#include "../definition.hpp"

namespace Invader::HEK {
    /**
     * Get the string name of the bitmap data format
     * @param  format format to get
     * @return        bitmap data format
     */
    const char *bitmap_data_format_name(BitmapDataFormat format);
}
#endif
