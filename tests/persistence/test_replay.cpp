#include "zh/foundation/byte_codec.h"
#include "zh/persistence/replay.h"

#include <iostream>
#include <string_view>

using namespace zh;

namespace {

int failures = 0;

void check(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

persistence::Snapshot snapshot(foundation::UInt32 tick, foundation::Int32 score)
{
    return {tick, 0x1020304050607080ULL + tick, score,
        {{7, 1, 100, 1.5F, -2.0F, u"Unit \u03a9"}}};
}

persistence::Replay sample()
{
    persistence::Replay replay;
    replay.scenario = u"Determinism";
    replay.configuration = u"seed=42;locale=English";
    replay.commands = {
        {1, persistence::ReplayCommandKind::move, 0, 7, 25, u"north"},
        {2, persistence::ReplayCommandKind::damage, 1, 7, -10, u"hit \u4e00"},
    };
    replay.checkpoints = {
        persistence::make_checkpoint(snapshot(1, 25)),
        persistence::make_checkpoint(snapshot(2, 15)),
    };
    return replay;
}

bool same(const persistence::Replay& a, const persistence::Replay& b)
{
    if (a.scenario != b.scenario || a.configuration != b.configuration ||
        a.commands.size() != b.commands.size() || a.checkpoints.size() != b.checkpoints.size()) return false;
    for (std::size_t i = 0; i < a.commands.size(); ++i) {
        if (a.commands[i].tick != b.commands[i].tick || a.commands[i].kind != b.commands[i].kind ||
            a.commands[i].player != b.commands[i].player || a.commands[i].target != b.commands[i].target ||
            a.commands[i].value != b.commands[i].value || a.commands[i].text != b.commands[i].text) return false;
    }
    for (std::size_t i = 0; i < a.checkpoints.size(); ++i) {
        if (a.checkpoints[i].tick != b.checkpoints[i].tick || a.checkpoints[i].crc32 != b.checkpoints[i].crc32 ||
            persistence::canonical_snapshot_bytes(a.checkpoints[i].snapshot) !=
                persistence::canonical_snapshot_bytes(b.checkpoints[i].snapshot)) return false;
    }
    return true;
}

struct Offsets {
    std::size_t command_count;
    std::size_t first_kind;
    std::size_t second_tick;
    std::size_t first_text_length;
    std::size_t checkpoint_count;
    std::size_t first_crc;
    std::size_t first_snapshot_length;
};

Offsets offsets(const std::vector<foundation::UInt8>& bytes)
{
    foundation::ByteReader reader({bytes.data(), bytes.size()});
    reader.read_bytes(12);
    reader.read_utf16le(persistence::maximum_save_text_code_units);
    reader.read_utf16le(persistence::maximum_save_text_code_units);
    Offsets result{};
    result.command_count = reader.position();
    const auto count = reader.read_u32_le();
    for (foundation::UInt32 index = 0; index < count; ++index) {
        if (index == 1) result.second_tick = reader.position();
        reader.read_u32_le();
        if (index == 0) result.first_kind = reader.position();
        reader.read_u8();
        reader.read_u8();
        reader.read_u32_le();
        reader.read_u32_le();
        if (index == 0) result.first_text_length = reader.position();
        reader.read_utf16le(persistence::maximum_save_text_code_units);
    }
    result.checkpoint_count = reader.position();
    reader.read_u32_le();
    reader.read_u32_le();
    result.first_crc = reader.position();
    reader.read_u32_le();
    result.first_snapshot_length = reader.position();
    return result;
}

void put_u32(std::vector<foundation::UInt8>& bytes, std::size_t offset, foundation::UInt32 value)
{
    for (unsigned index = 0; index < 4; ++index) bytes[offset + index] = static_cast<foundation::UInt8>(value >> (index * 8U));
}

template <typename Mutator>
void rejects(const std::vector<foundation::UInt8>& valid, Mutator mutator, std::string_view diagnostic)
{
    auto bytes = valid;
    mutator(bytes);
    auto destination = sample();
    destination.scenario = u"unchanged";
    try {
        persistence::decode_replay({bytes.data(), bytes.size()}, destination);
        check(false, "malformed replay accepted");
    } catch (const persistence::PersistenceError& error) {
        check(std::string_view(error.what()).find(diagnostic) != std::string_view::npos, "replay rejection diagnostic");
    }
    check(destination.scenario == u"unchanged", "failed replay decode mutated destination");
}

} // namespace

int main()
{
    const auto replay = sample();
    const auto bytes = persistence::encode_replay(replay);
    persistence::Replay decoded;
    persistence::decode_replay({bytes.data(), bytes.size()}, decoded);
    check(same(replay, decoded), "replay round trip");

    const auto canonical = persistence::canonical_snapshot_bytes(replay.checkpoints[0].snapshot);
    check(foundation::bytes_to_hex({canonical.data(), canonical.size()}) ==
        "01000000817060504030201019000000010000000700000001640000000000c03f000000c00600000055006e00690074002000a903",
        "canonical snapshot bytes changed");
    check(persistence::snapshot_crc32(replay.checkpoints[0].snapshot) == 0xd06f166eU,
        "canonical snapshot CRC changed");

    rejects(bytes, [](auto& data) { data[0] = 0; }, "magic");
    rejects(bytes, [](auto& data) { data[4] = 0; }, "endian");
    rejects(bytes, [](auto& data) { data[8] = 9; }, "version 9");
    rejects(bytes, [](auto& data) { data.resize(20); }, "UTF-16");
    rejects(bytes, [](auto& data) { data.push_back(0); }, "trailing");

    const auto layout = offsets(bytes);
    rejects(bytes, [&](auto& data) { put_u32(data, layout.command_count, persistence::maximum_replay_commands + 1); },
        "command count exceeds limit");
    rejects(bytes, [&](auto& data) { data[layout.first_kind] = 99; }, "command kind");
    rejects(bytes, [&](auto& data) { put_u32(data, layout.second_tick, 0); }, "ticks are decreasing");
    rejects(bytes, [&](auto& data) { put_u32(data, layout.first_text_length, 0xffffffffU); }, "UTF-16");
    rejects(bytes, [&](auto& data) {
        data[layout.first_text_length + 4] = 0x00;
        data[layout.first_text_length + 5] = 0xd8;
    }, "malformed UTF-16");
    rejects(bytes, [&](auto& data) { put_u32(data, layout.checkpoint_count, persistence::maximum_replay_checkpoints + 1); },
        "checkpoint count exceeds limit");
    rejects(bytes, [&](auto& data) { data[layout.first_crc] ^= 1; }, "CRC mismatch at tick 1");
    rejects(bytes, [&](auto& data) { put_u32(data, layout.first_snapshot_length, 0xffffffffU); }, "snapshot length");

    auto bad_kind = replay;
    bad_kind.commands[0].kind = static_cast<persistence::ReplayCommandKind>(99);
    try { (void)persistence::encode_replay(bad_kind); check(false, "bad command kind accepted"); }
    catch (const persistence::PersistenceError&) {}

    auto bad_order = replay;
    bad_order.commands[0].tick = 3;
    try { (void)persistence::encode_replay(bad_order); check(false, "decreasing commands accepted"); }
    catch (const persistence::PersistenceError&) {}

    auto bad_utf16 = replay;
    bad_utf16.commands[0].text = std::u16string(1, static_cast<char16_t>(0xdc00));
    try { (void)persistence::encode_replay(bad_utf16); check(false, "bad replay UTF-16 accepted"); }
    catch (const persistence::PersistenceError&) {}

    auto bad_crc = replay;
    ++bad_crc.checkpoints[0].crc32;
    try { (void)persistence::encode_replay(bad_crc); check(false, "bad checkpoint CRC accepted"); }
    catch (const persistence::PersistenceError& error) {
        check(std::string_view(error.what()).find("tick 1") != std::string_view::npos, "CRC diagnostic has checkpoint");
    }

    auto excessive = replay;
    excessive.commands.resize(static_cast<std::size_t>(persistence::maximum_replay_commands) + 1);
    try { (void)persistence::encode_replay(excessive); check(false, "excessive command count accepted"); }
    catch (const persistence::PersistenceError&) {}

    std::cout << "Linux replay codec tests: " << (failures == 0 ? "ok" : "failed") << '\n';
    return failures == 0 ? 0 : 1;
}
