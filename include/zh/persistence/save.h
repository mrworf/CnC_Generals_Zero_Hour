#pragma once

#include "zh/foundation/types.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace zh::persistence {

class PersistenceError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct EntityState {
    foundation::UInt32 id = 0;
    foundation::UInt8 owner = 0;
    foundation::Int32 health = 0;
    float x = 0.0F;
    float y = 0.0F;
    std::u16string name;
};

struct AutosaveMetadata {
    foundation::UInt32 sequence = 0;
    foundation::UInt64 unix_time_seconds = 0;
    std::u16string description;
};

struct SaveState {
    std::u16string scenario;
    foundation::UInt32 tick = 0;
    foundation::UInt64 random_state = 0;
    std::vector<EntityState> entities;
    std::optional<AutosaveMetadata> autosave;
};

inline constexpr foundation::UInt16 linux_save_version = 1;
inline constexpr std::size_t maximum_save_bytes = 1024U * 1024U;
inline constexpr foundation::UInt32 maximum_save_entities = 4096;
inline constexpr std::size_t maximum_save_text_code_units = 256;

std::vector<foundation::UInt8> encode_save(const SaveState& state);
void decode_save(foundation::ByteView bytes, SaveState& destination);

} // namespace zh::persistence
