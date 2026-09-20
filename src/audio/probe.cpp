#include "zh/audio/manager.h"

#include "zh/audio/vfs.h"
#include "zh/data/vfs.h"
#include "miniaudio.h"

#include <algorithm>
#include <array>
#include <cctype>

namespace zh::audio {
namespace {

std::uint16_t le16(const foundation::UInt8* bytes) noexcept
{
    return static_cast<std::uint16_t>(bytes[0]) | (static_cast<std::uint16_t>(bytes[1]) << 8U);
}

std::string detect_encoding(const data::VirtualFileSystem& vfs, std::string_view logical_path)
{
    const auto bytes = vfs.read_prefix(logical_path, 256);
    if (bytes.size() >= 12 && std::equal(bytes.begin(), bytes.begin() + 4, "RIFF") &&
        std::equal(bytes.begin() + 8, bytes.begin() + 12, "WAVE")) {
        std::size_t offset = 12;
        while (offset + 8 <= bytes.size()) {
            const auto size = static_cast<std::uint32_t>(bytes[offset + 4]) |
                (static_cast<std::uint32_t>(bytes[offset + 5]) << 8U) |
                (static_cast<std::uint32_t>(bytes[offset + 6]) << 16U) |
                (static_cast<std::uint32_t>(bytes[offset + 7]) << 24U);
            if (std::equal(bytes.begin() + offset, bytes.begin() + offset + 4, "fmt ")) {
                if (size < 2 || offset + 8 + size > bytes.size()) break;
                switch (le16(bytes.data() + offset + 8)) {
                case 1: return "pcm-wav";
                case 2: return "microsoft-adpcm-wav";
                case 17: return "ima-adpcm-wav";
                case 85: return "mp3-wav";
                default: return "wav-format-" + std::to_string(le16(bytes.data() + offset + 8));
                }
            }
            if (size > bytes.size() - offset - 8) break;
            offset += 8 + size + (size & 1U);
        }
        throw AudioError("corrupt audio logical asset '" + std::string(logical_path) + "': missing WAV format chunk");
    }
    if (bytes.size() >= 3 && ((bytes[0] == 'I' && bytes[1] == 'D' && bytes[2] == '3') ||
        (bytes[0] == 0xff && (bytes[1] & 0xe0U) == 0xe0U))) return "mp3";
    throw AudioError("unsupported or corrupt audio logical asset '" + std::string(logical_path) + "'");
}

} // namespace

AudioMetadata probe_audio(const data::VirtualFileSystem& vfs, std::string_view logical_path)
{
    const auto encoding = detect_encoding(vfs, logical_path);
    MiniaudioVfs adapter(vfs);
    ma_decoder decoder{};
    const auto path = std::string(logical_path);
    auto config = ma_decoder_config_init(ma_format_f32, 2, 48000);
    const auto result = ma_decoder_init_vfs(static_cast<ma_vfs*>(adapter.native_handle()), path.c_str(), &config, &decoder);
    if (result != MA_SUCCESS) {
        throw AudioError("cannot decode audio logical asset '" + path + "': miniaudio result " + std::to_string(result));
    }
    ma_uint32 channels = 0; ma_uint32 sample_rate = 0; ma_format format = ma_format_unknown;
    ma_channel channel_map[MA_MAX_CHANNELS]{};
    ma_decoder_get_data_format(&decoder, &format, &channels, &sample_rate, channel_map, MA_MAX_CHANNELS);
    ma_uint64 frames = 0;
    if (ma_decoder_get_length_in_pcm_frames(&decoder, &frames) != MA_SUCCESS || frames == 0) {
        std::array<float, 2> sample{}; ma_uint64 decoded = 0;
        ma_decoder_read_pcm_frames(&decoder, sample.data(), 1, &decoded);
        frames = decoded;
    }
    ma_decoder_uninit(&decoder);
    if (frames == 0 || channels == 0 || sample_rate == 0) {
        throw AudioError("audio logical asset '" + path + "' contains no decodable frames");
    }
    return {encoding, channels, sample_rate, frames};
}

} // namespace zh::audio
