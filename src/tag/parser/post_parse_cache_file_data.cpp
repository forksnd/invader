// SPDX-License-Identifier: GPL-3.0-only

#include <invader/map/map.hpp>
#include <invader/map/tag.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void Invader::Parser::ActorVariant::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->grenade_velocity *= TICK_RATE;
    }

    void Invader::Parser::Bitmap::post_parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer>) {
        for(auto &bitmap_data : this->bitmap_data) {
            const std::byte *bitmap_data_ptr;
            bitmap_data_ptr = tag.get_map().get_data_at_offset(bitmap_data.pixels_offset, bitmap_data.pixels_count, bitmap_data.flags.external ? Map::DATA_MAP_BITMAP : Map::DATA_MAP_CACHE);
            bitmap_data.pixels_offset = static_cast<std::size_t>(this->processed_pixel_data.size());
            this->processed_pixel_data.insert(this->processed_pixel_data.end(), bitmap_data_ptr, bitmap_data_ptr + bitmap_data.pixels_count);
            bitmap_data.flags.external = 0;
        }
    }

    void Invader::Parser::Scenario::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        auto *script_data = this->script_syntax_data.data();
        auto script_data_size = this->script_syntax_data.size();

        // If we don't have a script node table, give up
        if(script_data_size < sizeof(ScenarioScriptNodeTable::struct_little)) {
            eprintf("scenario tag has an invalid scenario script node table\n");
            throw InvalidTagDataException();
        }

        // Copy the table header
        ScenarioScriptNodeTable::struct_big table = *reinterpret_cast<ScenarioScriptNodeTable::struct_little *>(script_data);
        *reinterpret_cast<ScenarioScriptNodeTable::struct_big *>(script_data) = table;

        // Make sure it's not bullshit
        auto *script_nodes = reinterpret_cast<ScenarioScriptNode::struct_little *>(script_data + sizeof(table));
        auto table_size = table.maximum_count.read();
        std::size_t expected_size = (reinterpret_cast<std::byte *>(script_nodes + table_size) - script_data);
        if(expected_size != script_data_size) {
            eprintf("scenario tag has an invalid scenario script node table (%zu vs %zu)\n", expected_size, script_data_size);
            throw InvalidTagDataException();
        }

        // Copy the rest of the table
        for(std::size_t i = 0; i < table_size; i++) {
            ScenarioScriptNode::struct_big big = script_nodes[i];
            *reinterpret_cast<ScenarioScriptNode::struct_big *>(script_nodes + i) = big;
        }
    }

    void Invader::Parser::GBXModel::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        for(auto &marker : this->markers) {
            for(auto &instance : marker.instances) {
                // Figure out the region
                std::size_t region_index = instance.region_index;
                std::size_t region_count = this->regions.size();
                if(region_index >= region_count) {
                    eprintf("invalid region index %zu / %zu\n", region_index, region_count);
                    throw OutOfBoundsException();
                }

                // Next, figure out the region permutation
                auto &region = this->regions[region_index];
                std::size_t permutation_count = region.permutations.size();
                std::size_t permutation_index = instance.permutation_index;
                if(permutation_index >= permutation_count) {
                    eprintf("invalid permutation index %zu / %zu for region #%zu\n", permutation_index, permutation_count, region_index);
                    throw OutOfBoundsException();
                }

                // Lastly
                auto &new_marker = region.permutations[permutation_index].markers.emplace_back();
                new_marker.name = marker.name;
                new_marker.node_index = instance.node_index;
                new_marker.rotation = instance.rotation;
                new_marker.translation = instance.translation;
            }
        }

        float super_low = this->super_low_detail_cutoff;
        float low = this->low_detail_cutoff;
        float high = this->high_detail_cutoff;
        float super_high = this->super_high_detail_cutoff;

        this->super_low_detail_cutoff = super_high;
        this->low_detail_cutoff = high;
        this->high_detail_cutoff = low;
        this->super_high_detail_cutoff = super_low;
    }

    void Invader::Parser::Glow::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->attachment_0 = static_cast<HEK::FunctionOut>(this->attachment_0 + 1);
        this->attachment_1 = static_cast<HEK::FunctionOut>(this->attachment_1 + 1);
        this->attachment_2 = static_cast<HEK::FunctionOut>(this->attachment_2 + 1);
        this->attachment_3 = static_cast<HEK::FunctionOut>(this->attachment_3 + 1);
        this->attachment_4 = static_cast<HEK::FunctionOut>(this->attachment_4 + 1);
        this->attachment_5 = static_cast<HEK::FunctionOut>(this->attachment_5 + 1);
    }

    void Invader::Parser::PointPhysics::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->air_friction /= 10000.0f;
        this->water_friction /= 10000.0f;
    }

    void Invader::Parser::Projectile::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->initial_velocity *= TICK_RATE;
        this->final_velocity *= TICK_RATE;
    }

    void Invader::Parser::ScenarioCutsceneTitle::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->fade_in_time *= TICK_RATE;
        this->fade_out_time *= TICK_RATE;
        this->up_time *= TICK_RATE;
    }

    void Invader::Parser::Light::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->duration /= TICK_RATE;
    }

    void Invader::Parser::ScenarioStructureBSPMaterial::post_parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer) {
        // Do nothing if there is nothing to do
        if(this->rendered_vertices_count == 0) {
            this->lightmap_vertices_count = 0;
            return;
        }

        // Extract vertices
        auto &bsp_material = tag.get_struct_at_pointer<HEK::ScenarioStructureBSPMaterial>(*pointer);
        std::size_t uncompressed_vertices_size = this->rendered_vertices_count * sizeof(ScenarioStructureBSPMaterialUncompressedRenderedVertex::struct_little);
        const auto *uncompressed_bsp_vertices = reinterpret_cast<const ScenarioStructureBSPMaterialUncompressedRenderedVertex::struct_little *>(
            tag.data(bsp_material.uncompressed_vertices.pointer, uncompressed_vertices_size)
        );
        this->uncompressed_vertices.insert(this->uncompressed_vertices.end(), reinterpret_cast<const std::byte *>(uncompressed_bsp_vertices), reinterpret_cast<const std::byte *>(uncompressed_bsp_vertices + this->rendered_vertices_count));

        // Compress the vertices too
        auto *new_compressed_bsp_vertices = reinterpret_cast<ScenarioStructureBSPMaterialCompressedRenderedVertex::struct_little *>(
            this->compressed_vertices.insert(
                this->compressed_vertices.end(),
                this->rendered_vertices_count * sizeof(ScenarioStructureBSPMaterialCompressedRenderedVertex::struct_little),
                std::byte()
            ).base()
        );
        for(std::size_t v = 0; v < this->rendered_vertices_count; v++) {
            new_compressed_bsp_vertices[v] = HEK::compress_sbsp_rendered_vertex(uncompressed_bsp_vertices[v]);
        }

        // Add lightmap vertices
        if(this->lightmap_vertices_count == this->rendered_vertices_count) {
            auto *uncompressed_bsp_lightmap_vertices = reinterpret_cast<const ScenarioStructureBSPMaterialUncompressedLightmapVertex::struct_little *>(tag.data(bsp_material.uncompressed_vertices.pointer, uncompressed_vertices_size + this->lightmap_vertices_count * sizeof(ScenarioStructureBSPMaterialUncompressedLightmapVertex::struct_little)) + uncompressed_vertices_size);
            this->uncompressed_vertices.insert(this->uncompressed_vertices.end(), reinterpret_cast<const std::byte *>(uncompressed_bsp_lightmap_vertices), reinterpret_cast<const std::byte *>(uncompressed_bsp_lightmap_vertices + this->lightmap_vertices_count));

            // Compress them as well
            auto *new_compressed_bsp_lightmap_vertices = reinterpret_cast<ScenarioStructureBSPMaterialCompressedLightmapVertex::struct_little *>(
                this->compressed_vertices.insert(
                    this->compressed_vertices.end(),
                    this->lightmap_vertices_count * sizeof(ScenarioStructureBSPMaterialCompressedLightmapVertex::struct_little),
                    std::byte()
                ).base()
            );
            for(std::size_t v = 0; v < this->lightmap_vertices_count; v++) {
                new_compressed_bsp_lightmap_vertices[v] = HEK::compress_sbsp_lightmap_vertex(uncompressed_bsp_lightmap_vertices[v]);
            }
        }
        else if(this->lightmap_vertices_count != 0) {
            eprintf("non-zero lightmap vertex count (%zu) != rendered vertex count (%zu)\n", static_cast<std::size_t>(this->lightmap_vertices_count), static_cast<std::size_t>(this->rendered_vertices_count));
            throw InvalidTagDataException();
        }
    }

    void Invader::Parser::GBXModelGeometryPart::post_parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer) {
        const auto &part = tag.get_struct_at_pointer<HEK::GBXModelGeometryPart>(*pointer);
        const auto &map = tag.get_map();
        const auto &header = *reinterpret_cast<const HEK::CacheFileTagDataHeaderPC *>(&map.get_tag_data_header());

        // Get model vertices
        std::size_t vertex_count = part.vertex_count.read();
        const auto *vertices = reinterpret_cast<const GBXModelVertexUncompressed::struct_little *>(map.get_data_at_offset(header.model_data_file_offset.read() + part.vertex_offset.read(), sizeof(GBXModelVertexUncompressed::struct_little) * vertex_count));

        for(std::size_t v = 0; v < vertex_count; v++) {
            GBXModelVertexUncompressed::struct_big vertex_uncompressed = vertices[v];
            GBXModelVertexCompressed::struct_big vertex_compressed = HEK::compress_model_vertex(vertex_uncompressed);

            std::size_t data_read;
            this->uncompressed_vertices.emplace_back(GBXModelVertexUncompressed::parse_hek_tag_data(reinterpret_cast<const std::byte *>(&vertex_uncompressed), sizeof(vertex_uncompressed), data_read));
            this->compressed_vertices.emplace_back(GBXModelVertexCompressed::parse_hek_tag_data(reinterpret_cast<const std::byte *>(&vertex_compressed), sizeof(vertex_compressed), data_read));
        }

        // Get model indices
        std::size_t index_count = part.triangle_count.read() + 2;

        std::size_t triangle_count = (index_count) / 3;
        std::size_t triangle_modulo = index_count % 3;
        const auto *indices = reinterpret_cast<const HEK::LittleEndian<HEK::Index> *>(map.get_data_at_offset(header.model_data_file_offset.read() + part.triangle_offset.read() + header.vertex_size.read(), sizeof(std::uint16_t) * index_count));

        for(std::size_t t = 0; t < triangle_count; t++) {
            auto &triangle = this->triangles.emplace_back();
            auto *triangle_indices = indices + t * 3;
            triangle.vertex0_index = triangle_indices[0];
            triangle.vertex1_index = triangle_indices[1];
            triangle.vertex2_index = triangle_indices[2];
        }

        if(triangle_modulo) {
            auto &straggler_triangle = this->triangles.emplace_back();
            auto *triangle_indices = indices + triangle_count * 3;
            straggler_triangle.vertex0_index = triangle_indices[0];
            straggler_triangle.vertex1_index = triangle_modulo > 1 ? triangle_indices[1] : NULL_INDEX;
            straggler_triangle.vertex2_index = NULL_INDEX;
        }
    }

    void Invader::Parser::Sound::post_parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer>) {
        this->maximum_bend_per_second = std::pow(this->maximum_bend_per_second, TICK_RATE);
        if(tag.is_indexed()) {
            auto &tag_data = *(reinterpret_cast<const struct_little *>(&tag.get_struct_at_pointer<HEK::SoundPitchRange>(0, 0)) - 1);
            this->compression = tag_data.compression;
            this->encoding = tag_data.encoding;
        }
    }

    void Invader::Parser::SoundPermutation::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        if(this->compression == HEK::SoundCompression::SOUND_COMPRESSION_NONE) {
            auto *start = reinterpret_cast<HEK::LittleEndian<std::uint16_t> *>(this->samples.data());
            auto *end = start + this->samples.size() / sizeof(*start);

            while(start < end) {
                *reinterpret_cast<HEK::BigEndian<std::uint16_t> *>(start) = *start;
                start++;
            }
        }
    }

    void Invader::Parser::LensFlare::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->rotation_function_scale = DEGREES_TO_RADIANS(this->rotation_function_scale);
    }

    template <typename A, typename B> static void swap_endian_array(A *to, const B *from, std::size_t count) {
        for(std::size_t i = 0; i < count; i++) {
            to[i] = from[i];
        }
    }

    void Invader::Parser::ModelAnimationAnimation::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        std::vector<std::byte> frame_data = this->frame_data;
        std::vector<std::byte> frame_info = this->frame_info;
        std::vector<std::byte> default_data = this->default_data;

        // Frame info
        std::size_t required_frame_info_size;
        switch(this->frame_info_type) {
            case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_NONE:
                required_frame_info_size = 0;
                break;
            case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY:
                required_frame_info_size = sizeof(ModelAnimationFrameInfoDxDy::struct_little);
                break;
            case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY_DYAW:
                required_frame_info_size = sizeof(ModelAnimationFrameInfoDxDyDyaw::struct_little);
                break;
            case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY_DZ_DYAW:
                required_frame_info_size = sizeof(ModelAnimationFrameInfoDxDyDzDyaw::struct_little);
                break;
            default:
                throw InvalidTagDataException();
        }
        if(required_frame_info_size * this->frame_count != frame_info.size()) {
            throw OutOfBoundsException();
        }

        // Convert endianness
        if(frame_info.size() > 0) {
            switch(this->frame_info_type) {
                case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY:
                    swap_endian_array(
                        reinterpret_cast<ModelAnimationFrameInfoDxDy::struct_big *>(this->frame_info.data()),
                        reinterpret_cast<ModelAnimationFrameInfoDxDy::struct_little *>(frame_info.data()),
                        this->frame_count
                    );
                    break;
                case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY_DYAW:
                    swap_endian_array(
                        reinterpret_cast<ModelAnimationFrameInfoDxDyDyaw::struct_big *>(this->frame_info.data()),
                        reinterpret_cast<ModelAnimationFrameInfoDxDyDyaw::struct_little *>(frame_info.data()),
                        this->frame_count
                    );
                    break;
                case HEK::AnimationFrameInfoType::ANIMATION_FRAME_INFO_TYPE_DX_DY_DZ_DYAW:
                    swap_endian_array(
                        reinterpret_cast<ModelAnimationFrameInfoDxDyDzDyaw::struct_big *>(this->frame_info.data()),
                        reinterpret_cast<ModelAnimationFrameInfoDxDyDzDyaw::struct_little *>(frame_info.data()),
                        this->frame_count
                    );
                    break;
                default:
                    break;
            }
        }

        // Now do nodes
        if(this->node_count > 64) {
            throw OutOfBoundsException();
        }

        // Count this stuff
        std::size_t rotation_count = 0;
        std::size_t scale_count = 0;
        std::size_t transform_count = 0;
        std::uint32_t rotation_flag_0 = this->node_rotation_flag_data[0];
        std::uint32_t rotation_flag_1 = this->node_rotation_flag_data[1];
        std::uint32_t scale_flag_0 = this->node_scale_flag_data[0];
        std::uint32_t scale_flag_1 = this->node_scale_flag_data[1];
        std::uint32_t transform_flag_0 = this->node_transform_flag_data[0];
        std::uint32_t transform_flag_1 = this->node_transform_flag_data[1];
        bool rotate[64];
        bool scale[64];
        bool transform[64];
        for(std::size_t i = 0; i < node_count; i++) {
            bool has_rotation = false;
            bool has_scale = false;
            bool has_transform = false;

            if(i > 31) {
                has_rotation = (rotation_flag_1 >> (i - 32)) & 1;
                has_scale = (scale_flag_1 >> (i - 32)) & 1;
                has_transform = (transform_flag_1 >> (i - 32)) & 1;
            }
            else {
                has_rotation = (rotation_flag_0 >> i) & 1;
                has_scale = (scale_flag_0 >> i) & 1;
                has_transform = (transform_flag_0 >> i) & 1;
            }

            rotation_count += has_rotation;
            scale_count += has_scale;
            transform_count += has_transform;

            rotate[i] = has_rotation;
            scale[i] = has_scale;
            transform[i] = has_transform;
        }

        // Make sure frame and default size is correct
        std::size_t total_frame_size = rotation_count * sizeof(ModelAnimationRotation::struct_big) + scale_count * sizeof(ModelAnimationScale::struct_big) + transform_count * sizeof(ModelAnimationTransform::struct_big);
        std::size_t max_frame_size = node_count * (sizeof(ModelAnimationRotation::struct_big) + sizeof(ModelAnimationScale::struct_big) + sizeof(ModelAnimationTransform::struct_big));

        if(frame_size != total_frame_size) {
            throw InvalidTagDataException();
        }
        if(default_data.size() != (max_frame_size - total_frame_size)) {
            throw InvalidTagDataException();
        }

        // Do default data
        if(default_data.size() > 0) {
            auto *default_data_big = this->default_data.data();
            auto *default_data_little = default_data.data();

            for(std::size_t node = 0; node < this->node_count; node++) {
                if(!rotate[node]) {
                    auto &rotation_big = *reinterpret_cast<ModelAnimationRotation::struct_big *>(default_data_big);
                    const auto &rotation_little = *reinterpret_cast<const ModelAnimationRotation::struct_little *>(default_data_little);
                    rotation_big = rotation_little;
                    default_data_big += sizeof(rotation_big);
                    default_data_little += sizeof(rotation_big);
                }
                if(!transform[node]) {
                    auto &transform_big = *reinterpret_cast<ModelAnimationTransform::struct_big *>(default_data_big);
                    const auto &transform_little = *reinterpret_cast<const ModelAnimationTransform::struct_little *>(default_data_little);
                    transform_big = transform_little;
                    default_data_big += sizeof(transform_big);
                    default_data_little += sizeof(transform_big);
                }
                if(!scale[node]) {
                    auto &scale_big = *reinterpret_cast<ModelAnimationScale::struct_big *>(default_data_big);
                    const auto &scale_little = *reinterpret_cast<const ModelAnimationScale::struct_little *>(default_data_little);
                    scale_big = scale_little;
                    default_data_big += sizeof(scale_big);
                    default_data_little += sizeof(scale_big);
                }
            }
        }

        // Get whether or not it's compressed
        bool compressed = flags.compressed_data;

        // Convert frame data
        if(!compressed && frame_data.size() > 0) {
            auto *frame_data_big = this->frame_data.data();
            auto *frame_data_little = frame_data.data();
            for(std::size_t frame = 0; frame < frame_count; frame++) {
                for(std::size_t node = 0; node < this->node_count; node++) {
                    if(rotate[node]) {
                        auto &rotation_big = *reinterpret_cast<ModelAnimationRotation::struct_little *>(frame_data_big);
                        const auto &rotation_little = *reinterpret_cast<const ModelAnimationRotation::struct_big *>(frame_data_little);
                        rotation_big = rotation_little;
                        frame_data_big += sizeof(rotation_big);
                        frame_data_little += sizeof(rotation_big);
                    }
                    if(transform[node]) {
                        auto &transform_big = *reinterpret_cast<ModelAnimationTransform::struct_little *>(frame_data_big);
                        const auto &transform_little = *reinterpret_cast<const ModelAnimationTransform::struct_big *>(frame_data_little);
                        transform_big = transform_little;
                        frame_data_big += sizeof(transform_big);
                        frame_data_little += sizeof(transform_big);
                    }
                    if(scale[node]) {
                        auto &scale_big = *reinterpret_cast<ModelAnimationScale::struct_little *>(frame_data_big);
                        const auto &scale_little = *reinterpret_cast<const ModelAnimationScale::struct_big *>(frame_data_little);
                        scale_big = scale_little;
                        frame_data_big += sizeof(scale_big);
                        frame_data_little += sizeof(scale_big);
                    }
                }
            }
        }
    }
}