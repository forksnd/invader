/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "resource_map.hpp"

#include "hek/resource_map.hpp"

namespace Invader {
    std::vector<Resource> load_resource_map(const std::byte *data, std::size_t size) {
        using namespace HEK;
        if(size < sizeof(ResourceMapHeader)) {
            throw;
        }
        const auto &header = *reinterpret_cast<const ResourceMapHeader *>(data);
        std::size_t resource_count = header.resource_count;
        std::size_t resource_offset = header.resources;
        std::size_t path_offset = header.paths;
        if(resource_offset > size || resource_offset + sizeof(ResourceMapResource) * resource_count > size) {
            throw;
        }
        const auto *resources = reinterpret_cast<const ResourceMapResource *>(data + resource_offset);

        std::vector<Resource> returned_resources;

        for(std::size_t r = 0; r < resource_count; r++) {
            std::size_t resource_data_offset = resources[r].data_offset;
            std::size_t resource_path_offset = resources[r].path_offset + path_offset;
            std::size_t resource_data_size = resources[r].size;

            const auto *resource_data = data + resource_data_offset;
            if(resource_data_offset >= size || resource_data_offset + resource_data_size > size) {
                throw;
            }

            std::size_t resource_path_length = 0;
            if(resource_path_offset >= size) {
                throw;
            }
            const auto *resource_path = reinterpret_cast<const char *>(data + resource_path_offset);
            for(;;resource_path_length++) {
                if(resource_path_length >= size) {
                    throw;
                }
                else if(resource_path[resource_path_length] == 0) {
                    break;
                }
            }

            returned_resources.push_back(Resource { resource_path, std::vector<std::byte>(resource_data, resource_data + resource_data_size) });
        }

        return returned_resources;
    }
}
