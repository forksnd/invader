/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__HEK__ENDIAN_HPP
#define INVADER__HEK__ENDIAN_HPP

#include <cstring>
#include <cstddef>
#include <cstdint>
#include <exception>

namespace Invader::HEK {
    #define ENDIAN_TEMPLATE(tname) template <template<typename> class tname>
    #define COPY_THIS(what) copy . what = this -> what;
    #define COPY_THIS_ARRAY(what) for(std::size_t copy_this_array_iterator = 0; copy_this_array_iterator < sizeof(this -> what)/sizeof(this -> what[0]); copy_this_array_iterator++) { copy . what [copy_this_array_iterator] = this -> what [copy_this_array_iterator]; }

    /**
     * This is a simple interface for reading/writing swapped endian data
     */
    template <typename T> class SwappedEndian {
    public:
        std::byte value[sizeof(T)];

        /**
         * Convert the value to host endian.
         * @return the value in host endian
         */
        inline T read() const noexcept {
            std::byte value_copy_byte[sizeof(T)];
            for(std::size_t i = 0; i < sizeof(T); i++) {
                value_copy_byte[i] = value[sizeof(T) - (i + 1)];
            }
            return *reinterpret_cast<const T *>(value_copy_byte);
        }

        /**
         * Convert the value to host endian.
         * @return the value in host endian
         */
        inline operator T() const noexcept {
            return this->read();
        }

        /**
         * Overwrite the value
         * @param new_value value to overwrite
         */
        inline void write(const T &new_value) noexcept {
            *reinterpret_cast<T *>(this->value) = reinterpret_cast<const SwappedEndian<T> *>(&new_value)->read();
        }

        /**
         * Overwrite the value
         * @param new_value value to overwrite
         */
        inline void operator =(const T &new_value) noexcept {
            return this->write(new_value);
        }
    };

    /**
     * This is a simple interface for reading/writing native endian data
     */
    template <typename T> class NativeEndian {
    public:
        std::byte value[sizeof(T)];

        /**
         * Convert the value to host endian.
         * @return the value in host endian
         */
        inline T read() const noexcept {
            return *reinterpret_cast<const T *>(value);
        }

        /**
         * Convert the value to host endian.
         * @return the value in host endian
         */
        inline operator T() const noexcept {
            return this->read();
        }

        /**
         * Overwrite the value
         * @param new_value value to overwrite
         */
        inline void write(const T &new_value) noexcept {
            *reinterpret_cast<T *>(value) = new_value;
        }

        /**
         * Overwrite the value
         * @param new_value value to overwrite
         */
        inline void operator =(const T &new_value) noexcept {
            return this->write(new_value);
        }
    };

    #ifdef __BYTE_ORDER__
        #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            #define LittleEndian NativeEndian
            #define BigEndian SwappedEndian
        #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            #define BigEndian SwappedEndian
            #define LittleEndian NativeEndian
        #endif
    #else
        #define LittleEndian NativeEndian
        #define BigEndian SwappedEndian
    #endif
}
#endif
