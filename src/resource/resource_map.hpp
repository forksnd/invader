/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__RESOURCE__RESOURCE_MAP_HPP
#define INVADER__RESOURCE__RESOURCE_MAP_HPP

#include <cstddef>
#include <string>
#include <vector>

namespace Invader {
    struct Resource {
        std::string name;
        std::vector<std::byte> data;
    };

    /**
     * Return an array of containers for the given resource map
     * @param  data pointer to resource data
     * @param  size size of resource data
     * @return array of containers
     * @throws if failed
     */
    std::vector<Resource> load_resource_map(const std::byte *data, std::size_t size);
}
#endif
