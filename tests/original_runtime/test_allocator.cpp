#include "PreRTS.h"
#include "zh/original_process.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

extern "C" void* m26_allocate_across_target(std::size_t size);
extern "C" void m26_free_across_target(void* memory);
extern "C" void* m26_allocate_aligned(std::size_t size, std::size_t alignment);
extern "C" void m26_free_aligned(void* memory, std::size_t alignment);

namespace {

void* pre_main_allocation = m26_allocate_across_target(4097);

int fail(const char* message)
{
  std::fprintf(stderr, "m26 allocator failure: %s\n", message);
  return 1;
}

}  // namespace

int main(int argc, char** argv)
{
  if (!pre_main_allocation || !TheDynamicMemoryAllocator || !TheMemoryPoolFactory)
    return fail("pre-main original allocator did not initialize");

  initMemoryManager();
  if (!isMemoryManagerOfficiallyInited())
    return fail("official initialization/linkage check did not run");

  const auto status = zh::original_process::pool_config_status();
  const char* expected = argc > 1 ? argv[1] : "defaults";
  const auto expect_code = [&]() {
    using Code = zh::original_process::PoolConfigCode;
    if (std::strcmp(expected, "applied") == 0) return Code::applied;
    if (std::strcmp(expected, "malformed") == 0) return Code::malformed;
    if (std::strcmp(expected, "oversized") == 0) return Code::oversized;
    if (std::strcmp(expected, "invalid_count") == 0) return Code::invalid_count;
    if (std::strcmp(expected, "duplicate") == 0) return Code::duplicate;
    if (std::strcmp(expected, "io_error") == 0) return Code::io_error;
    return Code::defaults;
  }();
  if (status.code != expect_code)
    return fail("unexpected pool configuration result");

  Int initial = -1;
  Int overflow = -1;
  userMemoryAdjustPoolSize("PartitionContactListNode", initial, overflow);
  if (expect_code == zh::original_process::PoolConfigCode::applied) {
    if (initial != 8 || overflow != 4 || status.recognized != 1)
      return fail("valid override was not rounded/applied atomically");
  } else if (initial != 2048 || overflow != 512) {
    return fail("invalid or absent configuration changed compiled defaults");
  }

  const std::size_t raw_before = zh::original_process::live_raw_allocations();
  void* cross_target = m26_allocate_across_target(4097);
  if (zh::original_process::live_raw_allocations() != raw_before + 1)
    return fail("large cross-target allocation did not use original raw path");
  m26_free_across_target(cross_target);
  if (zh::original_process::live_raw_allocations() != raw_before)
    return fail("cross-target free did not restore raw allocation count");

  void* aligned = m26_allocate_aligned(37, 64);
  if ((reinterpret_cast<std::uintptr_t>(aligned) & 63U) != 0)
    return fail("aligned allocation did not meet requested alignment");
  m26_free_aligned(aligned, 64);

  if (!zh::original_process::test_pool_config_reentry_guard())
    return fail("pool configuration allocation re-entry was not detected");

  m26_free_across_target(pre_main_allocation);
  pre_main_allocation = nullptr;
  if (zh::original_process::live_raw_allocations() + 1 != raw_before)
    return fail("pre-main raw allocation was not released exactly once");

  userMemoryManagerInitPools();
  if (zh::original_process::pool_config_status().code != zh::original_process::PoolConfigCode::too_late)
    return fail("late pool retune was not rejected");

  std::printf("original-process allocator: ok provider=GameMemory.cpp config=%s raw=%zu\n",
              expected, zh::original_process::live_raw_allocations());
  return 0;
}
