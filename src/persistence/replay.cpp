#include "zh/persistence/replay.h"

#include "zh/foundation/byte_codec.h"
#include "zh/foundation/unicode.h"

#include <cmath>
#include <limits>
#include <string_view>

namespace zh::persistence {
namespace {

using namespace foundation;

constexpr UInt8 replay_magic[]{'Z', 'H', 'R', 'P'};
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

void validate_entity(const EntityState& entity)
{
    validate_text(entity.name, "snapshot entity name");
    if (!std::isfinite(entity.x) || !std::isfinite(entity.y)) {
        throw PersistenceError("snapshot entity position must be finite");
    }
}

bool valid_kind(ReplayCommandKind kind)
{
    return kind == ReplayCommandKind::move || kind == ReplayCommandKind::damage ||
        kind == ReplayCommandKind::grant_resource;
}

void validate_snapshot(const Snapshot& snapshot)
{
    if (snapshot.entities.size() > maximum_save_entities) {
        throw PersistenceError("snapshot entity count exceeds limit");
    }
    for (const auto& entity : snapshot.entities) validate_entity(entity);
}

void write_entity(ByteWriter& writer, const EntityState& entity)
{
    writer.write_u32_le(entity.id);
    writer.write_u8(entity.owner);
    writer.write_u32_le(static_cast<UInt32>(entity.health));
    writer.write_f32_le(entity.x);
    writer.write_f32_le(entity.y);
    writer.write_utf16le(entity.name, maximum_save_text_code_units);
}

EntityState read_entity(ByteReader& reader)
{
    EntityState entity;
    entity.id = reader.read_u32_le();
    entity.owner = reader.read_u8();
    entity.health = static_cast<Int32>(reader.read_u32_le());
    entity.x = reader.read_f32_le();
    entity.y = reader.read_f32_le();
    entity.name = reader.read_utf16le(maximum_save_text_code_units);
    validate_entity(entity);
    return entity;
}

Snapshot decode_snapshot(ByteView bytes)
{
    ByteReader reader(bytes);
    Snapshot snapshot;
    snapshot.tick = reader.read_u32_le();
    snapshot.random_state = reader.read_u64_le();
    snapshot.score = static_cast<Int32>(reader.read_u32_le());
    const UInt32 count = reader.read_u32_le();
    if (count > maximum_save_entities) throw PersistenceError("snapshot entity count exceeds limit");
    constexpr std::size_t minimum_entity_bytes = 4 + 1 + 4 + 4 + 4 + 4;
    if (count > reader.remaining() / minimum_entity_bytes) {
        throw PersistenceError("snapshot entity count exceeds remaining payload");
    }
    snapshot.entities.reserve(count);
    for (UInt32 index = 0; index < count; ++index) snapshot.entities.push_back(read_entity(reader));
    if (reader.remaining() != 0) throw PersistenceError("snapshot has trailing bytes");
    return snapshot;
}

template <typename Function>
auto translate_codec_error(Function&& function)
{
    try {
        return function();
    } catch (const PersistenceError&) {
        throw;
    } catch (const CodecError& error) {
        throw PersistenceError(std::string("invalid Linux replay: ") + error.what());
    }
}

void validate_replay(const Replay& replay)
{
    validate_text(replay.scenario, "replay scenario");
    validate_text(replay.configuration, "replay configuration");
    if (replay.commands.size() > maximum_replay_commands) throw PersistenceError("replay command count exceeds limit");
    if (replay.checkpoints.size() > maximum_replay_checkpoints) throw PersistenceError("replay checkpoint count exceeds limit");
    UInt32 prior_tick = 0;
    bool first = true;
    for (const auto& command : replay.commands) {
        if (!valid_kind(command.kind)) throw PersistenceError("unsupported replay command kind");
        validate_text(command.text, "replay command text");
        if (!first && command.tick < prior_tick) throw PersistenceError("replay command ticks are decreasing");
        first = false;
        prior_tick = command.tick;
    }
    first = true;
    for (const auto& checkpoint : replay.checkpoints) {
        validate_snapshot(checkpoint.snapshot);
        if (checkpoint.tick != checkpoint.snapshot.tick) throw PersistenceError("checkpoint tick does not match snapshot");
        if (!first && checkpoint.tick <= prior_tick) throw PersistenceError("replay checkpoint ticks are not increasing");
        if (checkpoint.crc32 != snapshot_crc32(checkpoint.snapshot)) {
            throw PersistenceError("checkpoint CRC mismatch at tick " + std::to_string(checkpoint.tick));
        }
        first = false;
        prior_tick = checkpoint.tick;
    }
}

} // namespace

std::vector<UInt8> canonical_snapshot_bytes(const Snapshot& snapshot)
{
    validate_snapshot(snapshot);
    return translate_codec_error([&] {
        ByteWriter writer(maximum_save_bytes);
        writer.write_u32_le(snapshot.tick);
        writer.write_u64_le(snapshot.random_state);
        writer.write_u32_le(static_cast<UInt32>(snapshot.score));
        writer.write_u32_le(static_cast<UInt32>(snapshot.entities.size()));
        for (const auto& entity : snapshot.entities) write_entity(writer, entity);
        return writer.bytes();
    });
}

UInt32 snapshot_crc32(const Snapshot& snapshot)
{
    const auto bytes = canonical_snapshot_bytes(snapshot);
    UInt32 crc = 0xffffffffU;
    for (const UInt8 byte : bytes) {
        crc ^= byte;
        for (unsigned bit = 0; bit < 8; ++bit) {
            const UInt32 mask = 0U - (crc & 1U);
            crc = (crc >> 1U) ^ (0xedb88320U & mask);
        }
    }
    return ~crc;
}

ReplayCheckpoint make_checkpoint(const Snapshot& snapshot)
{
    return {snapshot.tick, snapshot_crc32(snapshot), snapshot};
}

std::vector<UInt8> encode_replay(const Replay& replay)
{
    validate_replay(replay);
    return translate_codec_error([&] {
        ByteWriter writer(maximum_replay_bytes);
        writer.write_bytes({replay_magic, sizeof(replay_magic)});
        writer.write_u32_le(endian_marker);
        writer.write_u16_le(linux_replay_version);
        writer.write_u16_le(0);
        writer.write_utf16le(replay.scenario, maximum_save_text_code_units);
        writer.write_utf16le(replay.configuration, maximum_save_text_code_units);
        writer.write_u32_le(static_cast<UInt32>(replay.commands.size()));
        for (const auto& command : replay.commands) {
            writer.write_u32_le(command.tick);
            writer.write_u8(static_cast<UInt8>(command.kind));
            writer.write_u8(command.player);
            writer.write_u32_le(command.target);
            writer.write_u32_le(static_cast<UInt32>(command.value));
            writer.write_utf16le(command.text, maximum_save_text_code_units);
        }
        writer.write_u32_le(static_cast<UInt32>(replay.checkpoints.size()));
        for (const auto& checkpoint : replay.checkpoints) {
            const auto snapshot_bytes = canonical_snapshot_bytes(checkpoint.snapshot);
            writer.write_u32_le(checkpoint.tick);
            writer.write_u32_le(checkpoint.crc32);
            writer.write_u32_le(static_cast<UInt32>(snapshot_bytes.size()));
            writer.write_bytes({snapshot_bytes.data(), snapshot_bytes.size()});
        }
        return writer.bytes();
    });
}

void decode_replay(ByteView bytes, Replay& destination)
{
    if (bytes.size > maximum_replay_bytes) throw PersistenceError("Linux replay exceeds byte limit");
    Replay decoded = translate_codec_error([&] {
        ByteReader reader(bytes);
        if (reader.read_bytes(sizeof(replay_magic)) != std::vector<UInt8>(std::begin(replay_magic), std::end(replay_magic))) {
            throw PersistenceError("invalid Linux replay magic");
        }
        if (reader.read_u32_le() != endian_marker) throw PersistenceError("unsupported Linux replay endian marker");
        const UInt16 version = reader.read_u16_le();
        if (version != linux_replay_version) {
            throw PersistenceError("unsupported Linux replay version " + std::to_string(version));
        }
        if (reader.read_u16_le() != 0) throw PersistenceError("invalid Linux replay reserved field");

        Replay value;
        value.scenario = reader.read_utf16le(maximum_save_text_code_units);
        validate_text(value.scenario, "replay scenario");
        value.configuration = reader.read_utf16le(maximum_save_text_code_units);
        validate_text(value.configuration, "replay configuration");
        const UInt32 command_count = reader.read_u32_le();
        if (command_count > maximum_replay_commands) throw PersistenceError("replay command count exceeds limit");
        constexpr std::size_t minimum_command_bytes = 4 + 1 + 1 + 4 + 4 + 4;
        if (command_count > reader.remaining() / minimum_command_bytes) {
            throw PersistenceError("replay command count exceeds remaining document");
        }
        value.commands.reserve(command_count);
        UInt32 prior_tick = 0;
        for (UInt32 index = 0; index < command_count; ++index) {
            ReplayCommand command;
            command.tick = reader.read_u32_le();
            command.kind = static_cast<ReplayCommandKind>(reader.read_u8());
            command.player = reader.read_u8();
            command.target = reader.read_u32_le();
            command.value = static_cast<Int32>(reader.read_u32_le());
            command.text = reader.read_utf16le(maximum_save_text_code_units);
            if (!valid_kind(command.kind)) throw PersistenceError("unsupported replay command kind");
            validate_text(command.text, "replay command text");
            if (index != 0 && command.tick < prior_tick) throw PersistenceError("replay command ticks are decreasing");
            prior_tick = command.tick;
            value.commands.push_back(std::move(command));
        }

        const UInt32 checkpoint_count = reader.read_u32_le();
        if (checkpoint_count > maximum_replay_checkpoints) throw PersistenceError("replay checkpoint count exceeds limit");
        constexpr std::size_t minimum_checkpoint_bytes = 4 + 4 + 4 + 20;
        if (checkpoint_count > reader.remaining() / minimum_checkpoint_bytes) {
            throw PersistenceError("replay checkpoint count exceeds remaining document");
        }
        value.checkpoints.reserve(checkpoint_count);
        prior_tick = 0;
        for (UInt32 index = 0; index < checkpoint_count; ++index) {
            ReplayCheckpoint checkpoint;
            checkpoint.tick = reader.read_u32_le();
            checkpoint.crc32 = reader.read_u32_le();
            const UInt32 snapshot_size = reader.read_u32_le();
            if (snapshot_size > maximum_save_bytes || snapshot_size > reader.remaining()) {
                throw PersistenceError("replay snapshot length exceeds limit or remaining document");
            }
            const auto snapshot_bytes = reader.read_bytes(snapshot_size);
            checkpoint.snapshot = decode_snapshot({snapshot_bytes.data(), snapshot_bytes.size()});
            if (checkpoint.tick != checkpoint.snapshot.tick) throw PersistenceError("checkpoint tick does not match snapshot");
            if (index != 0 && checkpoint.tick <= prior_tick) throw PersistenceError("replay checkpoint ticks are not increasing");
            const UInt32 actual_crc = snapshot_crc32(checkpoint.snapshot);
            if (checkpoint.crc32 != actual_crc) {
                throw PersistenceError("checkpoint CRC mismatch at tick " + std::to_string(checkpoint.tick) +
                    ": expected " + std::to_string(checkpoint.crc32) + " actual " + std::to_string(actual_crc));
            }
            prior_tick = checkpoint.tick;
            value.checkpoints.push_back(std::move(checkpoint));
        }
        if (reader.remaining() != 0) throw PersistenceError("Linux replay has trailing bytes");
        return value;
    });
    destination = std::move(decoded);
}

} // namespace zh::persistence
