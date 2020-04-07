// SPDX-License-Identifier: GPL-3.0-only

#include <filesystem>
#include <invader/command_line_option.hpp>
#include <invader/printf.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/sound/sound_encoder.hpp>
#include <invader/sound/sound_reader.hpp>
#include <invader/version.hpp>
#include <vorbis/vorbisenc.h>
#include <samplerate.h>

using namespace Invader;
using namespace Invader::HEK;

struct SoundOptions {
    const char *data = "data";
    const char *tags = "tags";
    std::optional<bool> split;
    std::optional<SoundFormat> format;
    bool fs_path = false;
    std::optional<float> compression_level;
    bool extended = false;
    std::optional<std::size_t> channel_count;
    std::optional<SoundClass> sound_class;
    std::optional<std::uint32_t> sample_rate;
};

template<typename T> static std::vector<std::byte> make_sound_tag(const std::filesystem::path &tag_path, const std::filesystem::path &data_path, SoundOptions &sound_options) {
    static constexpr std::size_t SPLIT_BUFFER_SIZE = 0x38E00;
    static constexpr std::size_t MAX_PERMUTATIONS = UINT16_MAX - 1;

    // Parse the sound tag
    T sound_tag = {};
    auto *extended_sound = sizeof(Parser::ExtendedSound) == sizeof(T) ? reinterpret_cast<Parser::ExtendedSound *>(&sound_tag) : nullptr;

    if(std::filesystem::exists(tag_path)) {
        if(std::filesystem::is_directory(tag_path)) {
            eprintf_error("A directory exists at %s where a file was expected", tag_path.string().c_str());
            std::exit(EXIT_FAILURE);
        }
        auto sound_file = File::open_file(tag_path.string().c_str());
        if(sound_file.has_value()) {
            auto &sound_data = *sound_file;
            try {
                sound_tag = T::parse_hek_tag_file(sound_data.data(), sound_data.size());
            }
            catch(std::exception &e) {
                eprintf_error("An error occurred while attempting to read %s", tag_path.string().c_str());
                std::exit(EXIT_FAILURE);
            }
        }
        if(sound_options.sound_class.has_value()) {
            sound_tag.sound_class = *sound_options.sound_class;
        }
        else {
            sound_options.sound_class = sound_tag.sound_class;
        }
    }
    else {
        sound_tag.format = SoundFormat::SOUND_FORMAT_16_BIT_PCM;
        sound_tag.random_pitch_bounds.from = 1.0F;
        sound_tag.random_pitch_bounds.to = 1.0F;
        sound_tag.outer_cone_gain = 1.0F;
        sound_tag.random_gain_modifier = 1.0F;
        sound_tag.inner_cone_angle = 2 * HALO_PI;
        sound_tag.outer_cone_angle = 2 * HALO_PI;

        if(!sound_options.sound_class.has_value()) {
            eprintf_error("A sound class is required when generating new sound tags");
            std::exit(EXIT_FAILURE);
        }
        sound_tag.sound_class = *sound_options.sound_class;
    }

    if(extended_sound) {
        // Set sample rate
        if(sound_options.sample_rate.has_value()) {
            switch(*sound_options.sample_rate) {
                case 22050:
                    extended_sound->encoding_sample_rate = HEK::ExtendedSoundSampleRate::EXTENDED_SOUND_SAMPLE_RATE_22050_HZ;
                    break;
                case 44100:
                    extended_sound->encoding_sample_rate = HEK::ExtendedSoundSampleRate::EXTENDED_SOUND_SAMPLE_RATE_44100_HZ;
                    break;
                default:
                    std::terminate();
            }
        }
        else switch(extended_sound->encoding_sample_rate) {
            case HEK::ExtendedSoundSampleRate::EXTENDED_SOUND_SAMPLE_RATE_AUTOMATIC:
                break;
            case HEK::ExtendedSoundSampleRate::EXTENDED_SOUND_SAMPLE_RATE_22050_HZ:
                sound_options.sample_rate = 22050;
                break;
            case HEK::ExtendedSoundSampleRate::EXTENDED_SOUND_SAMPLE_RATE_44100_HZ:
                sound_options.sample_rate = 44100;
                break;
            case HEK::ExtendedSoundSampleRate::EXTENDED_SOUND_SAMPLE_RATE_ENUM_COUNT:
                std::terminate();
                break;
        }

        // Set channel count
        if(sound_options.channel_count.has_value()) {
            switch(*sound_options.sample_rate) {
                case 1:
                    extended_sound->encoding_channel_count = HEK::ExtendedSoundChannelCount::EXTENDED_SOUND_CHANNEL_COUNT_MONO;
                    break;
                case 2:
                    extended_sound->encoding_channel_count = HEK::ExtendedSoundChannelCount::EXTENDED_SOUND_CHANNEL_COUNT_STEREO;
                    break;
                default:
                    std::terminate();
            }
        }
        else switch(extended_sound->encoding_channel_count) {
            case HEK::ExtendedSoundChannelCount::EXTENDED_SOUND_CHANNEL_COUNT_AUTOMATIC:
                break;
            case HEK::ExtendedSoundChannelCount::EXTENDED_SOUND_CHANNEL_COUNT_MONO:
                sound_options.channel_count = 1;
                break;
            case HEK::ExtendedSoundChannelCount::EXTENDED_SOUND_CHANNEL_COUNT_STEREO:
                sound_options.channel_count = 2;
                break;
            case HEK::ExtendedSoundChannelCount::EXTENDED_SOUND_CHANNEL_COUNT_ENUM_COUNT:
                std::terminate();
                break;
        }

        // Set format
        if(sound_options.format.has_value()) {
            extended_sound->encoding_format = *sound_options.format;
        }
        else {
            sound_options.format = extended_sound->encoding_format;
        }

        // Set this stuff too
        if(sound_options.compression_level.has_value()) {
            extended_sound->compression_level = *sound_options.compression_level;
        }
        else {
            sound_options.compression_level = extended_sound->compression_level;
        }

        extended_sound->source_FLACs.clear();
    }

    // Hold onto this
    auto sound_class = sound_tag.sound_class;

    // If a format is defined, use it
    if(sound_options.format.has_value()) {
        sound_tag.format = *sound_options.format;
    }
    else {
        sound_options.format = SoundFormat::SOUND_FORMAT_16_BIT_PCM;
    }

    // Make sure we support the format
    switch(*sound_options.format) {
        case SoundFormat::SOUND_FORMAT_16_BIT_PCM:
        case SoundFormat::SOUND_FORMAT_XBOX_ADPCM:
        case SoundFormat::SOUND_FORMAT_OGG_VORBIS:
            break;
        case SoundFormat::SOUND_FORMAT_FLAC:
            if(!extended_sound) {
                eprintf_error("FLAC cannot be used without --extended");
                std::exit(EXIT_FAILURE);
            }
            break;
        case SoundFormat::SOUND_FORMAT_IMA_ADPCM:
            eprintf_error("IMA ADPCM is unsupported");
            std::exit(EXIT_FAILURE);
        default:
            eprintf_error("Unsupported audio codec");
            std::exit(EXIT_FAILURE);
    }

    auto &format = sound_tag.format;

    // If we don't have pitch ranges, add one
    bool is_new_pitch_range = sound_tag.pitch_ranges.size() == 0;
    sound_tag.pitch_ranges.resize(1, {});

    // Initialize a pitch range
    auto &old_pitch_range = sound_tag.pitch_ranges[0];
    Parser::SoundPitchRange pitch_range = {};
    if(is_new_pitch_range) {
        pitch_range.natural_pitch = 1.0F;
        pitch_range.bend_bounds.to = 1.0F;
    }
    else {
        pitch_range.natural_pitch = old_pitch_range.natural_pitch;
        pitch_range.bend_bounds.from = old_pitch_range.bend_bounds.from;
        pitch_range.bend_bounds.to = old_pitch_range.bend_bounds.to;
    }
    
    std::size_t old_actual_permutation_count;
    bool old_split = sound_tag.flags | HEK::SoundFlagsFlag::SOUND_FLAGS_FLAG_SPLIT_LONG_SOUND_INTO_PERMUTATIONS;
    
    if(old_split) {
        old_actual_permutation_count = old_pitch_range.actual_permutation_count;
        if(old_actual_permutation_count > old_pitch_range.permutations.size()) {
            eprintf_error("Existing sound tag has an invalid actual permutation count");
            std::exit(EXIT_FAILURE);
        }
    }
    else {
        old_actual_permutation_count = old_pitch_range.permutations.size();
    }

    // Set a default level
    if(!sound_options.compression_level.has_value()) {
        sound_options.compression_level = 1.0F;
    }

    // Error if bullshit compression levels were given
    if(sound_options.compression_level > 1.0F || sound_options.compression_level < 0.0F) {
        eprintf_error("Compression level (%.05f) is outside of the allowed range of 0.0 to 1.0", *sound_options.compression_level);
        std::exit(EXIT_FAILURE);
    }

    // Clear the old one
    for(auto &permutation : old_pitch_range.permutations) {
        permutation.samples = std::vector<std::byte>();
    }

    // Same with whether to split
    if(sound_options.split.has_value()) {
        if(*sound_options.split) {
            sound_tag.flags |= HEK::SoundFlagsFlag::SOUND_FLAGS_FLAG_SPLIT_LONG_SOUND_INTO_PERMUTATIONS;
        }
        else {
            sound_tag.flags &= ~HEK::SoundFlagsFlag::SOUND_FLAGS_FLAG_SPLIT_LONG_SOUND_INTO_PERMUTATIONS;
        }
    }
    bool split = sound_tag.flags | HEK::SoundFlagsFlag::SOUND_FLAGS_FLAG_SPLIT_LONG_SOUND_INTO_PERMUTATIONS;

    auto wav_iterator = std::filesystem::directory_iterator(data_path);

    std::uint16_t highest_channel_count = 0;
    std::uint32_t highest_sample_rate = 0;

    std::vector<SoundReader::Sound> permutations;

    oprintf("Loading sounds... ");
    oflush();
    for(auto &wav : wav_iterator) {
        // Skip directories
        auto path = wav.path();
        auto path_str = path.string();
        auto *path_str_cstr = path_str.c_str();
        if(wav.is_directory()) {
            eprintf_error("Unexpected directory %s", path_str_cstr);
            std::exit(EXIT_FAILURE);
        }
        auto extension = path.extension().string();

        // Get the sound
        SoundReader::Sound sound;
        try {
            if(extension == ".wav") {
                sound = SoundReader::sound_from_wav_file(path_str_cstr);
            }
            else if(extension == ".flac") {
                sound = SoundReader::sound_from_flac_file(path_str_cstr);
            }
            else {
                eprintf_error("Unknown file format for %s", path_str_cstr);
                std::exit(EXIT_FAILURE);
            }
        }
        catch(std::exception &e) {
            eprintf_error("Failed to load %s: %s", path_str_cstr, e.what());
            std::exit(EXIT_FAILURE);
        }

        // Use the highest channel count and sample rate
        if(highest_channel_count < sound.channel_count) {
            highest_channel_count = sound.channel_count;
        }
        if(highest_sample_rate < sound.sample_rate) {
            highest_sample_rate = sound.sample_rate;
        }

        // Make sure we can actually work with this
        if(sound.channel_count > 2 || sound.channel_count < 1) {
            eprintf_error("Unsupported channel count %u in %s", static_cast<unsigned int>(sound.channel_count), path_str.c_str());
            std::exit(EXIT_FAILURE);
        }
        if(sound.bits_per_sample % 8 != 0 || sound.bits_per_sample < 8 || sound.bits_per_sample > 24) {
            eprintf_error("Bits per sample (%u) is not divisible by 8 in %s (or is too small or too big)", static_cast<unsigned int>(sound.bits_per_sample), path_str.data());
            std::exit(EXIT_FAILURE);
        }
        auto filename = path.filename().string();

        // Get the permutation name
        sound.name = filename.substr(0, filename.size() - extension.size());
        if(sound.name.size() >= sizeof(HEK::TagString)) {
            eprintf_error("Permutation name %s exceeds the maximum permutation name size (%zu >= %zu)", sound.name.c_str(), sound.name.size(), sizeof(HEK::TagString));
            std::exit(EXIT_FAILURE);
        }

        // Lowercase it
        for(char &c : sound.name) {
            c = std::tolower(c);
        }

        // Make it small
        sound.pcm.shrink_to_fit();

        // Add it to the source list
        if(extended_sound) {
            auto &source = extended_sound->source_FLACs.emplace_back();
            std::memset(source.file_name.string, 0, sizeof(source.file_name.string));
            std::strncpy(source.file_name.string, sound.name.c_str(), sizeof(source.file_name) - 1);
            if(extension == ".flac") {
                source.compressed_audio_data = *File::open_file(path_str_cstr);
            }
            else {
                source.compressed_audio_data = SoundEncoder::encode_to_flac(sound.pcm, sound.bits_per_sample, sound.channel_count, sound.sample_rate, 5);
            }
        }

        // Add it
        std::size_t i;
        for(i = 0; i < permutations.size(); i++) {
            if(sound.name < permutations[i].name) {
                break;
            }
            else if(sound.name == permutations[i].name) {
                eprintf_error("Multiple permutations with the same name (%s) cannot be added", permutations[i].name.c_str());
                std::exit(EXIT_FAILURE);
            }
        }
        permutations.insert(permutations.begin() + i, std::move(sound));
    }
    oprintf("done!\n");

    // Force channel count
    if(sound_options.channel_count.has_value()) {
        highest_channel_count = *sound_options.channel_count;
    }

    // Make sure we have stuff
    if(permutations.size() == 0) {
        eprintf_error("No permutations found in %s", data_path.string().c_str());
        std::exit(EXIT_FAILURE);
    }

    // Set the actual permutation count
    auto actual_permutation_count = permutations.size();
    if(actual_permutation_count > MAX_PERMUTATIONS) {
        eprintf_error("Maximum number of actual permutations (%zu > %zu) exceeded", actual_permutation_count, MAX_PERMUTATIONS);
        std::exit(EXIT_FAILURE);
    }
    pitch_range.actual_permutation_count = static_cast<std::uint16_t>(actual_permutation_count);

    // Sound tags currently only support 22.05 kHz and 44.1 kHz
    if(!sound_options.sample_rate.has_value()) {
        if(highest_sample_rate > 22050) {
            highest_sample_rate = 44100;
        }
        else {
            highest_sample_rate = 22050;
        }
    }
    else {
        highest_sample_rate = *sound_options.sample_rate;
    }

    // Set the tag data
    if(highest_sample_rate == 22050) {
        sound_tag.sample_rate = SoundSampleRate::SOUND_SAMPLE_RATE_22050_HZ;
    }
    else if(highest_sample_rate == 44100) {
        sound_tag.sample_rate = SoundSampleRate::SOUND_SAMPLE_RATE_44100_HZ;
    }
    else {
        eprintf_error("Unsupported sample rate %u", highest_sample_rate);
        std::exit(EXIT_FAILURE);
    }

    // Sound tags currently only support single and dual channels
    if(highest_channel_count == 1) {
        sound_tag.channel_count = SoundChannelCount::SOUND_CHANNEL_COUNT_MONO;
    }
    else if(highest_channel_count == 2) {
        sound_tag.channel_count = SoundChannelCount::SOUND_CHANNEL_COUNT_STEREO;
    }
    else {
        eprintf_error("Unsupported channel count %u", highest_channel_count);
        std::exit(EXIT_FAILURE);
    }

    // Resample permutations when needed
    oprintf("Processing sounds... ");
    oflush();
    for(auto &permutation : permutations) {
        std::size_t bytes_per_sample = permutation.bits_per_sample / 8;
        std::size_t sample_count = permutation.pcm.size() / bytes_per_sample;

        // Sample rate doesn't match; this can be fixed with resampling
        if(permutation.sample_rate != highest_sample_rate) {
            double ratio = static_cast<double>(highest_sample_rate) / permutation.sample_rate;
            std::vector<float> float_samples = SoundEncoder::convert_int_to_float(permutation.pcm, permutation.bits_per_sample);
            std::vector<float> new_samples(float_samples.size() * ratio);
            permutation.sample_rate = highest_sample_rate;

            // Resample it
            SRC_DATA data = {};
            data.data_in = float_samples.data();
            data.data_out = new_samples.data();
            data.input_frames = float_samples.size() / permutation.channel_count;
            data.output_frames = new_samples.size() / permutation.channel_count;
            data.src_ratio = ratio;
            int res = src_simple(&data, SRC_SINC_BEST_QUALITY, permutation.channel_count);
            if(res) {
                eprintf_error("Failed to resample: %s", src_strerror(res));
                std::exit(EXIT_FAILURE);
            }
            new_samples.resize(data.output_frames_gen * permutation.channel_count);

            // Set stuff
            if(format == SoundFormat::SOUND_FORMAT_16_BIT_PCM) {
                bytes_per_sample = sizeof(std::uint16_t);
            }
            permutation.sample_rate = highest_sample_rate;
            permutation.bits_per_sample = bytes_per_sample * 8;
            sample_count = new_samples.size();
            permutation.pcm = SoundEncoder::convert_float_to_int(new_samples, permutation.bits_per_sample);
        }

        // Bits per sample doesn't match; we can fix that though
        if(bytes_per_sample != sizeof(std::uint16_t) && format == SoundFormat::SOUND_FORMAT_16_BIT_PCM) {
            std::size_t new_bytes_per_sample = sizeof(std::uint16_t);
            permutation.pcm = SoundEncoder::convert_int_to_int(permutation.pcm, permutation.bits_per_sample, new_bytes_per_sample * 8);
            bytes_per_sample = new_bytes_per_sample;
            permutation.bits_per_sample = new_bytes_per_sample * 8;
        }

        // Mono -> Stereo (just duplicate the channels)
        if(permutation.channel_count == 1 && highest_channel_count == 2) {
            std::vector<std::byte> new_samples(sample_count * 2 * bytes_per_sample);
            const std::byte *old_sample = permutation.pcm.data();
            const std::byte *old_sample_end = permutation.pcm.data() + permutation.pcm.size();
            std::byte *new_sample = new_samples.data();

            while(old_sample < old_sample_end) {
                std::memcpy(new_sample, old_sample, bytes_per_sample);
                std::memcpy(new_sample + bytes_per_sample, old_sample, bytes_per_sample);
                old_sample += bytes_per_sample;
                new_sample += bytes_per_sample * 2;
            }

            permutation.pcm = std::move(new_samples);
            permutation.pcm.shrink_to_fit();
            permutation.channel_count = 2;
        }

        // Stereo -> Mono (mixdown)
        else if(permutation.channel_count == 2 && highest_channel_count == 1) {
            std::vector<std::byte> new_samples(sample_count * bytes_per_sample / 2);
            std::byte *new_sample = new_samples.data();
            const std::byte *old_sample = permutation.pcm.data();
            const std::byte *old_sample_end = permutation.pcm.data() + permutation.pcm.size();

            while(old_sample < old_sample_end) {
                std::int32_t a = Invader::SoundEncoder::read_sample(old_sample, permutation.bits_per_sample);
                std::int32_t b = Invader::SoundEncoder::read_sample(old_sample + bytes_per_sample, permutation.bits_per_sample);
                std::int64_t ab = a + b;
                Invader::SoundEncoder::write_sample(static_cast<std::int32_t>(ab / 2), new_sample, permutation.bits_per_sample);

                old_sample += bytes_per_sample * 2;
                new_sample += bytes_per_sample;
            }

            permutation.pcm = std::move(new_samples);
            permutation.pcm.shrink_to_fit();
            permutation.channel_count = 1;
        }
    }
    oprintf("done!\n");

    // Add a new permutation
    auto new_permutation = [&pitch_range, &format](SoundReader::Sound &permutation) -> Parser::SoundPermutation & {
        auto &p = pitch_range.permutations.emplace_back();
        std::memset(p.name.string, 0, sizeof(p.name.string));
        std::strncpy(p.name.string, permutation.name.c_str(), sizeof(p.name.string) - 1);
        p.format = format;
        return p;
    };

    // Add initial permutations
    for(auto &permutation : permutations) {
        new_permutation(permutation);
    }

    // Make the sound tag
    const char *output_name = nullptr;
    switch(format) {
        case SoundFormat::SOUND_FORMAT_16_BIT_PCM:
            output_name = "16-bit PCM";
            break;
        case SoundFormat::SOUND_FORMAT_IMA_ADPCM:
            output_name = "IMA ADPCM";
            break;
        case SoundFormat::SOUND_FORMAT_XBOX_ADPCM:
            output_name = "Xbox ADPCM";
            break;
        case SoundFormat::SOUND_FORMAT_OGG_VORBIS:
            output_name = "Ogg Vorbis";
            break;
        case SoundFormat::SOUND_FORMAT_FLAC:
            output_name = "Free Lossless Audio Codec";
            break;
        case SoundFormat::SOUND_FORMAT_ENUM_COUNT:
            std::terminate();
    }
    oprintf("Found %zu sound%s:\n", actual_permutation_count, actual_permutation_count == 1 ? "" : "s");

    // Check if this is dialogue
    bool is_dialogue;
    switch(sound_class) {
        case SoundClass::SOUND_CLASS_UNIT_DIALOG:
        case SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_PLAYER:
        case SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_OTHER:
        case SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_FORCE_UNSPATIALIZED:
            is_dialogue = true;
            break;
        default:
            is_dialogue = false;
    }

    if(is_dialogue && split) {
        eprintf_error("Split dialogue is unsupported.");
        std::exit(EXIT_FAILURE);
    }

    for(std::size_t i = 0; i < actual_permutation_count; i++) {
        auto &permutation = permutations[i];

        // Calculate length
        double seconds = permutation.pcm.size() / static_cast<double>(static_cast<std::size_t>(permutation.sample_rate) * static_cast<std::size_t>(permutation.bits_per_sample / 8) * static_cast<std::size_t>(permutation.channel_count));
        std::size_t bytes_per_sample_one_channel = permutation.bits_per_sample / 8;
        std::size_t bytes_per_sample_all_channels = bytes_per_sample_one_channel * permutation.channel_count;

        // Generate mouth data (a lot of this comes from MosesofEgypt's Reclaimer)
        auto generate_mouth_data = [&permutation](Parser::SoundPermutation &p, const std::vector<std::uint8_t> &pcm_8_bit) {
            // Basically, take the sample rate, multiply by channel count, divide by tick rate (30 Hz), and round the result
            std::size_t samples_per_tick = static_cast<std::size_t>((permutation.sample_rate * permutation.channel_count) / TICK_RATE + 0.5);
            std::size_t sample_count = pcm_8_bit.size();

            // Generate samples, adding an extra tick for incomplete ticks
            std::size_t tick_count = (sample_count + samples_per_tick - 1) / samples_per_tick;
            p.mouth_data = std::vector<std::byte>(tick_count);
            auto *pcm_data = pcm_8_bit.data();

            // Get max and total
            std::uint8_t max = 0;
            double mouth_total = 0;
            for(std::size_t t = 0; t < tick_count; t++) {
                // Get the sample range, accounting for when there aren't enough ticks
                std::size_t first_sample = t * samples_per_tick;
                std::size_t sample_count_to_check = sample_count - first_sample;
                if(sample_count_to_check > samples_per_tick) {
                    sample_count_to_check = samples_per_tick;
                }
                std::size_t last_sample = first_sample + sample_count_to_check;
                double total = 0;
                for(std::size_t s = first_sample; s < last_sample; s++) {
                    total += pcm_data[s];
                }

                // Divide by samples per tick
                double average = total / samples_per_tick;
                mouth_total += average;
                p.mouth_data[t] = static_cast<std::byte>(average);

                if(average > max) {
                    max = average;
                }
            }

            // Get average and min, clamping min to 0-255
            double average = mouth_total / tick_count;
            double min = 2.0 * average - max;
            if(min > UINT8_MAX) {
                min = UINT8_MAX;
            }
            else if(min < 0) {
                min = 0;
            }

            // Get range
            double range = static_cast<double>(max + average) / 2 - min;

            // Do nothing if there's no range
            if(range == 0) {
                return;
            }

            // Go through each sample
            for(std::size_t t = 0; t < tick_count; t++) {
                double sample = (static_cast<std::uint8_t>(p.mouth_data[t]) - min) / range;

                // Clamp to 0 - 255
                if(sample >= 1.0) {
                    p.mouth_data[t] = static_cast<std::byte>(UINT8_MAX);
                }
                else if(sample <= 0.0) {
                    p.mouth_data[t] = static_cast<std::byte>(0);
                }
                else {
                    p.mouth_data[t] = static_cast<std::byte>(sample * UINT8_MAX);
                }
            }
        };

        // Encode a permutation
        auto encode_permutation = [&permutation, &format, &sound_options, &generate_mouth_data, &is_dialogue](Parser::SoundPermutation &p, std::vector<std::byte> &pcm) {
            // Set the default stuff
            p.gain = 1.0F;

            // Generate mouth data if needed
            if(is_dialogue) {
                // Convert samples to 8-bit unsigned so we can use it to generate mouth data
                auto samples_float = SoundEncoder::convert_int_to_float(permutation.pcm, permutation.bits_per_sample);
                std::vector<std::uint8_t> pcm_8_bit;
                pcm_8_bit.reserve(samples_float.size());
                for(auto &f : samples_float) {
                    float ff = f;
                    if(ff < 0.0F) {
                        ff *= -1.0F;
                    }
                    pcm_8_bit.emplace_back(static_cast<std::uint8_t>(ff * UINT8_MAX));
                }
                samples_float = {};
                generate_mouth_data(p, pcm_8_bit);
            }

            switch(format) {
                // Basically, just make it 16-bit big endian
                case SoundFormat::SOUND_FORMAT_16_BIT_PCM:
                    p.samples = Invader::SoundEncoder::convert_to_16_bit_pcm_big_endian(pcm, permutation.bits_per_sample);
                    p.buffer_size = p.samples.size();
                    break;

                // Encode to Vorbis in an Ogg container
                case SoundFormat::SOUND_FORMAT_OGG_VORBIS: {
                    if(sound_options.compression_level > 1.0F) {
                        sound_options.compression_level = 1.0F;
                    }
                    else if(sound_options.compression_level < 0.0F) {
                        sound_options.compression_level = 0.0F;
                    }
                    p.samples = Invader::SoundEncoder::encode_to_ogg_vorbis(pcm, permutation.bits_per_sample, permutation.channel_count, permutation.sample_rate, *sound_options.compression_level);
                    p.buffer_size = pcm.size() / (permutation.bits_per_sample / 8) * sizeof(std::int16_t);
                    break;
                }

                // Encode to FLAC
                case SoundFormat::SOUND_FORMAT_FLAC: {
                    // Clamp to 0.0 - 0.8
                    if(sound_options.compression_level > 0.8F) {
                        sound_options.compression_level = 0.8F;
                    }
                    else if(sound_options.compression_level < 0.0F) {
                        sound_options.compression_level = 0.0F;
                    }
                    // Convert to integer, rounding to the nearest level
                    int flac_level = static_cast<int>(*sound_options.compression_level * 10.0F + 0.5F);
                    p.samples = Invader::SoundEncoder::encode_to_flac(pcm, permutation.bits_per_sample, permutation.channel_count, permutation.sample_rate, flac_level);
                    break;
                }

                // Encode to Xbox ADPCMeme
                case SoundFormat::SOUND_FORMAT_XBOX_ADPCM:
                    p.samples = Invader::SoundEncoder::encode_to_xbox_adpcm(pcm, permutation.bits_per_sample, permutation.channel_count);
                    p.buffer_size = 0;
                    break;

                default:
                    std::terminate();
            }
        };

        // Split if requested
        if(split) {
            const std::size_t MAX_SPLIT_SIZE = SPLIT_BUFFER_SIZE - (SPLIT_BUFFER_SIZE % bytes_per_sample_all_channels);
            std::size_t digested = 0;
            while(permutation.pcm.size() > 0) {
                // Basically, if we haven't encoded anything, use the i-th permutation, otherwise make a new one
                auto &p = digested == 0 ? pitch_range.permutations[i] : new_permutation(permutation);
                std::size_t remaining_size = permutation.pcm.size();
                std::size_t permutation_size = remaining_size > MAX_SPLIT_SIZE ? MAX_SPLIT_SIZE : remaining_size;

                // Encode it
                auto *sample_data_start = permutation.pcm.data();
                auto sample_data = std::vector<std::byte>(sample_data_start, sample_data_start + permutation_size);
                permutation.pcm = std::vector<std::byte>(permutation.pcm.begin() + permutation_size, permutation.pcm.end());
                encode_permutation(p, sample_data);
                p.samples.shrink_to_fit();
                permutation.pcm.shrink_to_fit();
                digested += permutation_size;

                if(permutation.pcm.size() == 0) {
                    p.next_permutation_index = NULL_INDEX;
                }
                else {
                    std::size_t next_permutation = pitch_range.permutations.size();
                    if(next_permutation > MAX_PERMUTATIONS) {
                        eprintf_error("Maximum number of total permutations (%zu > %zu) exceeded", next_permutation, MAX_PERMUTATIONS);
                        std::exit(EXIT_FAILURE);
                    }
                    p.next_permutation_index = static_cast<Index>(next_permutation);
                }
            }
        }
        else {
            auto &p = pitch_range.permutations[i];
            p.next_permutation_index = NULL_INDEX;
            encode_permutation(p, permutation.pcm);
            p.samples.shrink_to_fit();
        }

        // Write the old actual permutation stuff if needed
        for(std::size_t o = 0; o < old_actual_permutation_count; o++) {
            if(std::strcmp(old_pitch_range.permutations[o].name.string, permutation.name.c_str()) == 0) {
                std::size_t new_permutation = i;
                std::size_t old_permutation = o;
                while(new_permutation != NULL_INDEX && old_permutation != NULL_INDEX) {
                    if(old_permutation > old_pitch_range.permutations.size()) {
                        eprintf_error("Existing sound tag has an invalid next permutation index");
                        std::exit(EXIT_FAILURE);
                    }

                    auto &old_pr = old_pitch_range.permutations[old_permutation];
                    auto &new_pr = pitch_range.permutations[new_permutation];

                    new_pr.gain = old_pr.gain;
                    new_pr.skip_fraction = old_pr.skip_fraction;

                    new_permutation = new_pr.next_permutation_index;
                    old_permutation = old_pr.next_permutation_index;

                    if(!old_split || !split) {
                        break;
                    }
                }

                break;
            }
        }

        // Print sound info
        oprintf("    %-32s%2zu:%06.3f (%2zu-bit %6s %5zu Hz)\n", permutation.name.c_str(), static_cast<std::size_t>(seconds) / 60, std::fmod(seconds, 60.0), static_cast<std::size_t>(permutation.input_bits_per_sample), permutation.input_channel_count == 1 ? "mono" : "stereo", static_cast<std::size_t>(permutation.input_sample_rate));
        permutation.pcm = std::vector<std::byte>();
    }

    // Wrap it up
    std::memset(pitch_range.name.string, 0, sizeof(pitch_range.name));
    static constexpr char DEFAULT_NAME[] = "default";
    std::memcpy(pitch_range.name.string, DEFAULT_NAME, sizeof(DEFAULT_NAME));

    // Move it
    sound_tag.pitch_ranges[0].permutations.clear();
    sound_tag.pitch_ranges[0].permutations.reserve(pitch_range.permutations.size());
    for(auto &permutation : pitch_range.permutations) {
        sound_tag.pitch_ranges[0].permutations.push_back(std::move(permutation));
        permutation.samples = std::vector<std::byte>();
        permutation.mouth_data = std::vector<std::byte>();
    }
    sound_tag.pitch_ranges[0].name = pitch_range.name;

    auto sound_tag_data = sound_tag.generate_hek_tag_data(extended_sound == nullptr ? TagClassInt::TAG_CLASS_SOUND : TagClassInt::TAG_CLASS_EXTENDED_SOUND, true);
    oprintf("Output: %s, %s, %zu Hz%s, %s, %.03f MiB%s\n", output_name, highest_channel_count == 1 ? "mono" : "stereo", static_cast<std::size_t>(highest_sample_rate), split ? ", split" : "", SoundClass_to_string(sound_class), sound_tag_data.size() / 1024.0 / 1024.0, extended_sound == nullptr ? "" : " [--extended]");

    return sound_tag_data;
}

int main(int argc, const char **argv) {
    EXIT_IF_INVADER_EXTRACT_HIDDEN_VALUES

    SoundOptions sound_options;

    std::vector<CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");
    options.emplace_back("data", 'd', 1, "Use the specified data directory.", "<dir>");
    options.emplace_back("split", 's', 0, "Split permutations into 227.5 KiB chunks. This is necessary for longer sounds (e.g. music) when being played in the original Halo engine.");
    options.emplace_back("no-split", 'S', 0, "Do not split permutations.");
    options.emplace_back("extended", 'x', 0, "Create an extended_sound tag.");
    options.emplace_back("format", 'F', 1, "Set the format. Can be: 16-bit-pcm, ogg-vorbis, or xbox-adpcm. Using flac requires --extended. Setting this is required unless creating an extended tag, in which case it defaults to 16-bit-pcm.", "<fmt>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the data.");
    options.emplace_back("channel-count", 'C', 1, "[REQUIRES --extended] Set the channel count. Can be: mono, stereo. By default, this is determined based on the input audio.", "<#>");
    options.emplace_back("sample-rate", 'r', 1, "[REQUIRES --extended] Set the sample rate in Hz. Halo supports 22050 and 44100. By default, this is determined based on the input audio.", "<Hz>");
    options.emplace_back("compress-level", 'l', 1, "Set the compression level. This can be between 0.0 and 1.0. For Ogg Vorbis, higher levels result in better quality but worse sizes. For FLAC, higher levels result in better sizes but longer compression time, clamping from 0.0 to 0.8 (FLAC 0 to FLAC 8). Default: 1.0", "<lvl>");
    options.emplace_back("class", 'c', 1, "Set the class. This is required when generating new sounds. Can be: ambient-computers, ambient-machinery, ambient-nature, device-computers, device-door, device-force-field, device-machinery, device-nature, first-person-damage, game-event, music, object-impacts, particle-impacts, projectile-impact, projectile-detonation, scripted-dialog-force-unspatialized, scripted-dialog-other, scripted-dialog-player, scripted-effect, slow-particle-impacts, unit-dialog, unit-footsteps, vehicle-collision, vehicle-engine, weapon-charge, weapon-empty, weapon-fire, weapon-idle, weapon-overheat, weapon-ready, weapon-reload", "<class>");

    static constexpr char DESCRIPTION[] = "Create or modify a sound tag.";
    static constexpr char USAGE[] = "[options] <sound-tag>";

    auto remaining_arguments = CommandLineOption::parse_arguments<SoundOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, sound_options, [](char opt, const std::vector<const char *> &arguments, auto &sound_options) {
        switch(opt) {
            case 'd':
                sound_options.data = arguments[0];
                break;

            case 't':
                sound_options.tags = arguments[0];
                break;

            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);

            case 's':
                sound_options.split = true;
                break;

            case 'S':
                sound_options.split = false;
                break;

            case 'C':
                try {
                    sound_options.channel_count = SoundChannelCount_from_string(arguments[0]) == SoundChannelCount::SOUND_CHANNEL_COUNT_MONO ? 1 : 2;
                }
                catch(std::exception &) {
                    eprintf_error("Unknown channel count %s (should be \"mono\" or \"stereo\")", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 'F':
                try {
                    sound_options.format = SoundFormat_from_string(arguments[0]);
                }
                catch(std::exception &) {
                    eprintf_error("Unknown sound format %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 'l':
                sound_options.compression_level = std::atof(arguments[0]);
                break;

            case 'P':
                sound_options.fs_path = true;
                break;

            case 'x':
                sound_options.extended = true;
                break;

            case 'r':
                try {
                    sound_options.sample_rate = static_cast<std::uint32_t>(std::stol(arguments[0]));
                }
                catch(std::exception &) {
                    eprintf_error("Invalid sample rate: %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                if(sound_options.sample_rate != 22050 && sound_options.sample_rate != 44100) {
                    eprintf_error("Only 22050 Hz and 44100 Hz sample rates are allowed");
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 'c':
                try {
                    sound_options.sound_class = SoundClass_from_string(arguments[0]);
                }
                catch(std::exception &) {
                    eprintf_error("Unknown sound class %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
        }
    });

    // Get our paths and make sure a data directory exists
    std::string halo_tag_path;
    if(sound_options.fs_path) {
        std::vector<std::string> data;
        data.emplace_back(std::string(sound_options.data));
        try {
            halo_tag_path = Invader::File::file_path_to_tag_path(remaining_arguments[0], data, false).value();
        }
        catch(std::exception &) {
            eprintf_error("Cannot find %s in %s", remaining_arguments[0], sound_options.data);
            return EXIT_FAILURE;
        }
    }
    else {
        halo_tag_path = remaining_arguments[0];
    }

    // Remove trailing slashes
    halo_tag_path = Invader::File::remove_trailing_slashes(halo_tag_path);
    auto data_path = std::filesystem::path(sound_options.data) / halo_tag_path;
    if(!std::filesystem::is_directory(data_path)) {
        eprintf_error("No directory exists at %s", data_path.string().c_str());
        return EXIT_FAILURE;
    }

    // Find it!
    auto tag_path = std::filesystem::path(sound_options.tags) / (halo_tag_path + ".extended_sound");
    if(std::filesystem::exists(tag_path)) {
        sound_options.extended = true;
    }

    std::vector<std::byte> sound_tag_data;
    if(!sound_options.extended) {
        tag_path = std::filesystem::path(sound_options.tags) / (halo_tag_path + ".sound");

        // Make sure format was set
        if(!sound_options.format.has_value()) {
            eprintf_error("No sound format set (required for .sound tags). Use -h for more information.");
            return EXIT_FAILURE;
        }

        sound_tag_data = make_sound_tag<Parser::Sound>(tag_path, data_path, sound_options);
    }
    else {
        sound_tag_data = make_sound_tag<Parser::ExtendedSound>(tag_path, data_path, sound_options);
    }

    // Create missing directories if needed
    try {
        if(!std::filesystem::exists(tag_path.parent_path())) {
            std::filesystem::create_directories(tag_path.parent_path());
        }
    }
    catch(std::exception &e) {
        eprintf_error("Error: Failed to create a directory: %s\n", e.what());
        return EXIT_FAILURE;
    }

    // Save
    if(!Invader::File::save_file(tag_path.string().c_str(), sound_tag_data)) {
        eprintf_error("Failed to save %s", tag_path.string().c_str());
        return EXIT_FAILURE;
    }
}
