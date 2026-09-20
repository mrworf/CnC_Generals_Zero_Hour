#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace zh::foundation {

using Int8 = std::int8_t;
using UInt8 = std::uint8_t;
using Int16 = std::int16_t;
using UInt16 = std::uint16_t;
using Int32 = std::int32_t;
using UInt32 = std::uint32_t;
using Int64 = std::int64_t;
using UInt64 = std::uint64_t;
using IntPtr = std::intptr_t;
using UIntPtr = std::uintptr_t;
using WideChar = char16_t;
using Bool = bool;

static_assert(sizeof(Int8) == 1 && sizeof(UInt8) == 1);
static_assert(sizeof(Int16) == 2 && sizeof(UInt16) == 2);
static_assert(sizeof(Int32) == 4 && sizeof(UInt32) == 4);
static_assert(sizeof(Int64) == 8 && sizeof(UInt64) == 8);
static_assert(sizeof(WideChar) == 2);
static_assert(sizeof(IntPtr) == sizeof(void*));
static_assert(std::numeric_limits<float>::is_iec559 && sizeof(float) == 4);

struct ByteView {
    const UInt8* data = nullptr;
    std::size_t size = 0;
};

} // namespace zh::foundation
