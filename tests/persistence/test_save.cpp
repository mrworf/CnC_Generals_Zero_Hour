#include "zh/persistence/save.h"

#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <string_view>

using namespace zh;

namespace {

int failures = 0;

void check(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

bool same(const persistence::SaveState& left, const persistence::SaveState& right)
{
    if (left.scenario != right.scenario || left.tick != right.tick ||
        left.random_state != right.random_state || left.score != right.score || left.entities.size() != right.entities.size() ||
        left.autosave.has_value() != right.autosave.has_value()) return false;
    for (std::size_t index = 0; index < left.entities.size(); ++index) {
        const auto& a = left.entities[index];
        const auto& b = right.entities[index];
        if (a.id != b.id || a.owner != b.owner || a.health != b.health ||
            a.x != b.x || a.y != b.y || a.name != b.name) return false;
    }
    return !left.autosave || (left.autosave->sequence == right.autosave->sequence &&
        left.autosave->unix_time_seconds == right.autosave->unix_time_seconds &&
        left.autosave->description == right.autosave->description);
}

persistence::SaveState sample(bool autosave = true)
{
    persistence::SaveState value;
    value.scenario = u"Alpine Assault \U0001f642";
    value.tick = 720;
    value.random_state = 0x123456789abcdef0ULL;
    value.score = -120;
    value.entities = {
        {17, 1, 350, 12.5F, -4.25F, u"Dozer"},
        {42, 2, -7, -0.0F, 88.0F, u"Tank \u03a9"},
    };
    if (autosave) value.autosave = persistence::AutosaveMetadata{3, 1'700'000'000ULL, u"Checkpoint \u4e00"};
    return value;
}

template <typename Mutator>
void rejects_without_mutation(const std::vector<foundation::UInt8>& valid, Mutator mutate, std::string_view expected)
{
    auto bytes = valid;
    mutate(bytes);
    auto destination = sample(false);
    const auto original = destination;
    try {
        persistence::decode_save({bytes.data(), bytes.size()}, destination);
        check(false, "malformed save accepted");
    } catch (const persistence::PersistenceError& error) {
        check(std::string_view(error.what()).find(expected) != std::string_view::npos, "save rejection diagnostic");
    }
    check(same(destination, original), "failed decode mutated destination");
}

} // namespace

int main()
{
    const auto state = sample();
    const auto bytes = persistence::encode_save(state);
    persistence::SaveState decoded;
    persistence::decode_save({bytes.data(), bytes.size()}, decoded);
    check(same(state, decoded), "autosave round trip");

    const auto manual = sample(false);
    const auto manual_bytes = persistence::encode_save(manual);
    persistence::decode_save({manual_bytes.data(), manual_bytes.size()}, decoded);
    check(same(manual, decoded), "manual save round trip");
    check(bytes[4] == 4 && bytes[5] == 3 && bytes[6] == 2 && bytes[7] == 1, "explicit little-endian marker");

    rejects_without_mutation(bytes, [](auto& data) { data[0] = 'X'; }, "magic");
    rejects_without_mutation(bytes, [](auto& data) { data[4] = 1; }, "endian");
    rejects_without_mutation(bytes, [](auto& data) { data[8] = 2; }, "version 2");
    rejects_without_mutation(bytes, [](auto& data) { data.resize(15); }, "truncated");
    rejects_without_mutation(bytes, [](auto& data) { data.push_back(0); }, "trailing");

    // The entity count follows the 12-byte header, length-prefixed scenario, tick, and RNG.
    const std::size_t entity_count_offset = 12 + 4 + state.scenario.size() * 2 + 4 + 8 + 4;
    rejects_without_mutation(bytes, [=](auto& data) {
        data[entity_count_offset] = 0x01; data[entity_count_offset + 1] = 0x10;
        data[entity_count_offset + 2] = 0; data[entity_count_offset + 3] = 0;
    }, "entity count exceeds limit");

    rejects_without_mutation(bytes, [](auto& data) {
        data[12] = 0xff; data[13] = 0xff; data[14] = 0xff; data[15] = 0x7f;
    }, "UTF-16");

    auto malformed_utf16 = state;
    malformed_utf16.scenario = std::u16string(1, static_cast<char16_t>(0xd800));
    try {
        (void)persistence::encode_save(malformed_utf16);
        check(false, "unpaired UTF-16 surrogate accepted");
    } catch (const persistence::PersistenceError& error) {
        check(std::string_view(error.what()).find("malformed UTF-16") != std::string_view::npos,
            "UTF-16 encode diagnostic");
    }

    auto non_finite = state;
    non_finite.entities[0].x = std::numeric_limits<float>::infinity();
    try {
        (void)persistence::encode_save(non_finite);
        check(false, "non-finite position accepted");
    } catch (const persistence::PersistenceError&) {
    }

    // Last byte before autosave metadata is the flag for a manual save.
    rejects_without_mutation(manual_bytes, [](auto& data) { data.back() = 2; }, "autosave flag");

    std::cout << "Linux save codec tests: " << (failures == 0 ? "ok" : "failed") << '\n';
    return failures == 0 ? 0 : 1;
}
