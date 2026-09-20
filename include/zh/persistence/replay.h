#pragma once

#include "zh/persistence/save.h"

#include <string>
#include <vector>

namespace zh::persistence {

enum class ReplayCommandKind : foundation::UInt8 {
    move = 1,
    damage = 2,
    grant_resource = 3,
};

struct ReplayCommand {
    foundation::UInt32 tick = 0;
    ReplayCommandKind kind = ReplayCommandKind::move;
    foundation::UInt8 player = 0;
    foundation::UInt32 target = 0;
    foundation::Int32 value = 0;
    std::u16string text;
};

struct Snapshot {
    foundation::UInt32 tick = 0;
    foundation::UInt64 random_state = 0;
    foundation::Int32 score = 0;
    std::vector<EntityState> entities;
};

struct ReplayCheckpoint {
    foundation::UInt32 tick = 0;
    foundation::UInt32 crc32 = 0;
    Snapshot snapshot;
};

struct Replay {
    std::u16string scenario;
    std::u16string configuration;
    std::vector<ReplayCommand> commands;
    std::vector<ReplayCheckpoint> checkpoints;
};

inline constexpr foundation::UInt16 linux_replay_version = 1;
inline constexpr std::size_t maximum_replay_bytes = 8U * 1024U * 1024U;
inline constexpr foundation::UInt32 maximum_replay_commands = 100'000;
inline constexpr foundation::UInt32 maximum_replay_checkpoints = 10'000;

std::vector<foundation::UInt8> canonical_snapshot_bytes(const Snapshot& snapshot);
foundation::UInt32 snapshot_crc32(const Snapshot& snapshot);
ReplayCheckpoint make_checkpoint(const Snapshot& snapshot);

std::vector<foundation::UInt8> encode_replay(const Replay& replay);
void decode_replay(foundation::ByteView bytes, Replay& destination);

} // namespace zh::persistence
