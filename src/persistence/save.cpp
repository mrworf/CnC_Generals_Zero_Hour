#include "zh/persistence/save.h"

#include "zh/foundation/byte_codec.h"
#include "zh/foundation/unicode.h"

#include <cmath>
#include <string_view>

namespace zh::persistence {
namespace {

using namespace foundation;

constexpr UInt8 save_magic[]{'Z', 'H', 'S', 'G'};
constexpr UInt32 endian_marker = 0x01020304U;

void validate_text(std::u16string_view value, std::string_view field)
{
    if (value.size() > maximum_save_text_code_units) {
        throw PersistenceError(std::string(field) + " exceeds UTF-16 code-unit limit");
    }
    try {
        (void)utf16_to_utf8(value);
    } catch (const UnicodeError& error) {
        throw PersistenceError(std::string(field) + " contains malformed UTF-16: " + error.what());
    }
}

void validate_state(const SaveState& state)
{
    validate_text(state.scenario, "save scenario");
    if (state.entities.size() > maximum_save_entities) {
        throw PersistenceError("save entity count exceeds limit");
    }
    for (const auto& entity : state.entities) {
        validate_text(entity.name, "entity name");
        if (!std::isfinite(entity.x) || !std::isfinite(entity.y)) {
            throw PersistenceError("entity position must be finite");
        }
    }
    if (state.autosave) validate_text(state.autosave->description, "autosave description");
}

template <typename Function>
auto translate_codec_error(Function&& function)
{
    try {
        return function();
    } catch (const PersistenceError&) {
        throw;
    } catch (const CodecError& error) {
        throw PersistenceError(std::string("invalid Linux save: ") + error.what());
    }
}

} // namespace

std::vector<UInt8> encode_save(const SaveState& state)
{
    validate_state(state);
    return translate_codec_error([&] {
        ByteWriter writer(maximum_save_bytes);
        writer.write_bytes({save_magic, sizeof(save_magic)});
        writer.write_u32_le(endian_marker);
        writer.write_u16_le(linux_save_version);
        writer.write_u16_le(0); // reserved; keeps the envelope extensible without host padding
        writer.write_utf16le(state.scenario, maximum_save_text_code_units);
        writer.write_u32_le(state.tick);
        writer.write_u64_le(state.random_state);
        writer.write_u32_le(static_cast<UInt32>(state.score));
        writer.write_u32_le(static_cast<UInt32>(state.entities.size()));
        for (const auto& entity : state.entities) {
            writer.write_u32_le(entity.id);
            writer.write_u8(entity.owner);
            writer.write_u32_le(static_cast<UInt32>(entity.health));
            writer.write_f32_le(entity.x);
            writer.write_f32_le(entity.y);
            writer.write_utf16le(entity.name, maximum_save_text_code_units);
        }
        writer.write_u8(state.autosave ? 1U : 0U);
        if (state.autosave) {
            writer.write_u32_le(state.autosave->sequence);
            writer.write_u64_le(state.autosave->unix_time_seconds);
            writer.write_utf16le(state.autosave->description, maximum_save_text_code_units);
        }
        return writer.bytes();
    });
}

void decode_save(ByteView bytes, SaveState& destination)
{
    if (bytes.size > maximum_save_bytes) throw PersistenceError("Linux save exceeds byte limit");
    SaveState decoded = translate_codec_error([&] {
        ByteReader reader(bytes);
        if (reader.read_bytes(sizeof(save_magic)) != std::vector<UInt8>(std::begin(save_magic), std::end(save_magic))) {
            throw PersistenceError("invalid Linux save magic");
        }
        if (reader.read_u32_le() != endian_marker) throw PersistenceError("unsupported Linux save endian marker");
        const UInt16 version = reader.read_u16_le();
        if (version != linux_save_version) {
            throw PersistenceError("unsupported Linux save version " + std::to_string(version));
        }
        if (reader.read_u16_le() != 0) throw PersistenceError("invalid Linux save reserved field");

        SaveState value;
        value.scenario = reader.read_utf16le(maximum_save_text_code_units);
        validate_text(value.scenario, "save scenario");
        value.tick = reader.read_u32_le();
        value.random_state = reader.read_u64_le();
        value.score = static_cast<Int32>(reader.read_u32_le());
        const UInt32 entity_count = reader.read_u32_le();
        if (entity_count > maximum_save_entities) throw PersistenceError("save entity count exceeds limit");
        // A valid entity needs at least fixed fields plus an empty UTF-16 length.
        constexpr std::size_t minimum_entity_bytes = 4 + 1 + 4 + 4 + 4 + 4;
        if (entity_count > reader.remaining() / minimum_entity_bytes) {
            throw PersistenceError("save entity count exceeds remaining document");
        }
        value.entities.reserve(entity_count);
        for (UInt32 index = 0; index < entity_count; ++index) {
            EntityState entity;
            entity.id = reader.read_u32_le();
            entity.owner = reader.read_u8();
            entity.health = static_cast<Int32>(reader.read_u32_le());
            entity.x = reader.read_f32_le();
            entity.y = reader.read_f32_le();
            entity.name = reader.read_utf16le(maximum_save_text_code_units);
            validate_text(entity.name, "entity name");
            if (!std::isfinite(entity.x) || !std::isfinite(entity.y)) {
                throw PersistenceError("entity position must be finite");
            }
            value.entities.push_back(std::move(entity));
        }
        const UInt8 autosave_flag = reader.read_u8();
        if (autosave_flag > 1) throw PersistenceError("invalid autosave flag");
        if (autosave_flag == 1) {
            AutosaveMetadata metadata;
            metadata.sequence = reader.read_u32_le();
            metadata.unix_time_seconds = reader.read_u64_le();
            metadata.description = reader.read_utf16le(maximum_save_text_code_units);
            validate_text(metadata.description, "autosave description");
            value.autosave = std::move(metadata);
        }
        if (reader.remaining() != 0) throw PersistenceError("Linux save has trailing bytes");
        return value;
    });
    destination = std::move(decoded);
}

} // namespace zh::persistence
