#include "zh/original_support/ww_support.h"

#include "zh/foundation/numeric.h"
#include "zh/foundation/unicode.h"

#include "FastAllocator.h"
#include "RAMFILE.H"
#include "chunkio.h"
#include "gcd_lcm.h"
#include "nstrdup.h"
#include "pointerremap.h"
#include "tri.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace zh::original_support {
namespace {

void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

void exercise_chunk_io()
{
    std::array<char, 128> storage{};
    RAMFileClass output(storage.data(), static_cast<int>(storage.size()));
    require(output.Open(FileClass::WRITE), "original RAM file did not open for writing");
    ChunkSaveClass save(&output);
    constexpr uint32 chunk_id = 0x11223344U;
    constexpr uint32 payload = 0x55667788U;
    require(save.Begin_Chunk(chunk_id), "original chunk writer did not begin chunk");
    require(save.Write(&payload, sizeof(payload)) == sizeof(payload), "original chunk writer truncated payload");
    require(save.End_Chunk(), "original chunk writer did not close chunk");
    const int written = output.Size();
    output.Close();

    if (written != 12) throw std::runtime_error("original chunk size changed: " + std::to_string(written));
    const std::array<unsigned char, 8> expected_header{0x44, 0x33, 0x22, 0x11, 0x04, 0x00, 0x00, 0x00};
    require(std::memcmp(storage.data(), expected_header.data(), expected_header.size()) == 0,
        "original chunk header is not fixed little-endian x86 layout");

    RAMFileClass input(storage.data(), written);
    require(input.Open(FileClass::READ), "original RAM file did not open for reading");
    ChunkLoadClass load(&input);
    require(load.Open_Chunk() && load.Cur_Chunk_ID() == chunk_id && load.Cur_Chunk_Length() == sizeof(payload),
        "original chunk reader changed header fields");
    uint32 decoded = 0;
    require(load.Read(&decoded, sizeof(decoded)) == sizeof(decoded) && decoded == payload,
        "original chunk reader changed payload");
    require(load.Close_Chunk(), "original chunk reader did not close chunk");
}

void exercise_pointer_remap()
{
    int old_value = 1;
    int new_value = 2;
    int other_old = 3;
    int other_new = 4;
    void* pointer = &old_value;
    void* missing = reinterpret_cast<void*>(static_cast<uintptr_t>(1));
    PointerRemapClass remap;
    remap.Register_Pointer(&old_value, &new_value);
    remap.Register_Pointer(&other_old, &other_new);
    remap.Request_Pointer_Remap(&pointer);
    remap.Request_Pointer_Remap(&missing);
    remap.Process();
    require(pointer == &new_value, "original pointer remap did not apply registered mapping");
    require(missing == nullptr, "original pointer remap did not null an unknown mapping");
}

} // namespace

bool rejects_truncated_chunk()
{
    std::array<char, 4> truncated{};
    RAMFileClass input(truncated.data(), static_cast<int>(truncated.size()));
    if (!input.Open(FileClass::READ)) return false;
    ChunkLoadClass load(&input);
    return !load.Open_Chunk() && load.Cur_Chunk_Depth() == 0;
}

std::string run_ww_support_probe()
{
    require(Greatest_Common_Divisor(84, 30) == 6, "original WWLib GCD changed");
    require(Least_Common_Multiple(21, 6) == 42, "original WWLib LCM changed");

    char* duplicated = nstrdup("Zero Hour");
    require(duplicated != nullptr && std::strcmp(duplicated, "Zero Hour") == 0, "original WWLib string duplication changed");
    delete[] duplicated;

    FastAllocatorGeneral allocator;
    void* allocation = allocator.Alloc(37);
    require(allocation != nullptr && allocator.Get_Total_Allocation_Count() == 1, "original allocator did not track allocation");
    allocator.Free(allocation);
    require(allocator.Get_Total_Allocation_Count() == 0 && allocator.Get_Total_Allocated_Size() == 0,
        "original allocator teardown retained live allocation state");

    const Vector3 points[3]{Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)};
    const Vector3 normal(0.0f, 0.0f, 1.0f);
    TriClass triangle{};
    triangle.N = &normal;
    triangle.V[0] = &points[0]; triangle.V[1] = &points[1]; triangle.V[2] = &points[2];
    require(triangle.Contains_Point(Vector3(0.25f, 0.25f, 0.0f)), "original WWMath triangle rejected interior point");
    require(!triangle.Contains_Point(Vector3(1.25f, 1.25f, 0.0f)), "original WWMath triangle accepted exterior point");

    exercise_chunk_io();
    require(rejects_truncated_chunk(), "original chunk reader accepted truncated header");
    exercise_pointer_remap();

    zh::foundation::ScopedRoundToNearest rounding;
    require(zh::foundation::checked_float_to_i32(2.5f, zh::foundation::IntegralRounding::nearest) == 2,
        "round-to-nearest-even boundary changed");
    const std::string utf8 = "ZH \xF0\x9F\x8C\x90";
    const auto utf16 = zh::foundation::utf8_to_utf16(utf8);
    require(sizeof(zh::foundation::WideChar) == 2 && zh::foundation::utf16_to_utf8(utf16) == utf8,
        "fixed-width UTF-16 boundary changed");

    return "original-support runtime provider=GeneralsMD WWLib/WWMath/WWSaveLoad chunk=ok allocator=clean utf16=char16_t";
}

} // namespace zh::original_support

const char* zh_wwsupport_bootstrap_component() noexcept
{
    return "zh_wwsupport:original-ww-support";
}
