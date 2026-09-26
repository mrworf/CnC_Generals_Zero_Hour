#include "PreRTS.h"
#include "zh/original_process.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <new>
#include <limits>
#include <initializer_list>

extern "C" void* m26_allocate_across_target(std::size_t size);
extern "C" void m26_free_across_target(void* memory);
extern "C" void* m26_allocate_aligned(std::size_t size, std::size_t alignment);
extern "C" void m26_free_aligned(void* memory, std::size_t alignment);
extern "C" void* m22_allocate_nothrow_across_target(std::size_t size, bool array);
extern "C" void m22_free_nothrow_across_target(void* memory, bool array, unsigned route);
extern "C" void* __real_malloc(std::size_t size) noexcept;

namespace { bool fail_next_raw_malloc = false; unsigned raw_failures = 0; }
extern "C" void* __wrap_malloc(std::size_t size) noexcept
{
  if (fail_next_raw_malloc) {
    fail_next_raw_malloc = false;
    ++raw_failures;
    return nullptr;
  }
  return __real_malloc(size);
}

namespace {

void* pre_main_allocation = m26_allocate_across_target(4097);

int fail(const char* message)
{
  std::fprintf(stderr, "m26 allocator failure: %s\n", message);
  return 1;
}

struct ThrowingScalar {
  char bytes[4097];
  ThrowingScalar() { throw 17; }
};
struct ThrowingArray {
  char bytes[4097];
  static unsigned constructed, destroyed;
  ThrowingArray() { if (++constructed == 2) throw 19; }
  ~ThrowingArray() { ++destroyed; }
};
unsigned ThrowingArray::constructed = 0;
unsigned ThrowingArray::destroyed = 0;

int nothrow_pairing()
{
  for (bool array : {false, true}) {
    for (unsigned route = 0; route != 3; ++route) {
      const auto raw = zh::original_process::live_raw_allocations();
      const auto pools = zh::original_process::live_pool_allocations();
      void* memory = m22_allocate_nothrow_across_target(4097, array);
      if (!memory || zh::original_process::live_raw_allocations() != raw + 1)
        return fail("ordinary nothrow cross-target raw owner missing");
      m22_free_nothrow_across_target(memory, array, route);
      if (zh::original_process::live_raw_allocations() != raw ||
          zh::original_process::live_pool_allocations() != pools)
        return fail("ordinary/sized/placement nothrow deletion changed counts");
    }
    const auto raw = zh::original_process::live_raw_allocations();
    const auto pools = zh::original_process::live_pool_allocations();
    void* small = m22_allocate_nothrow_across_target(37, array);
    if (!small || zh::original_process::live_pool_allocations() != pools + 1)
      return fail("small ordinary nothrow allocation did not acquire one pool unit");
    m22_free_nothrow_across_target(small, array, 0);
    void* zero = m22_allocate_nothrow_across_target(0, array);
    if (!zero) return fail("zero-size nothrow allocation failed");
    m22_free_nothrow_across_target(zero, array, 2);
    if (m22_allocate_nothrow_across_target((std::numeric_limits<std::size_t>::max)(), array))
      return fail("unrepresentable nothrow size admitted");
    if (m22_allocate_nothrow_across_target((std::numeric_limits<Int>::max)(), array))
      return fail("nothrow private-header overflow admitted");
    if (zh::original_process::live_raw_allocations() != raw ||
        zh::original_process::live_pool_allocations() != pools)
      return fail("zero/unrepresentable nothrow size changed counts");
    const unsigned failures = raw_failures;
    fail_next_raw_malloc = true;
    if (m22_allocate_nothrow_across_target(4097, array) || fail_next_raw_malloc ||
        raw_failures != failures + 1)
      return fail("actual nothrow allocation failure did not return null once");
    if (zh::original_process::live_raw_allocations() != raw ||
        zh::original_process::live_pool_allocations() != pools)
      return fail("failed nothrow raw request changed counts");
    void* retry = m22_allocate_nothrow_across_target(4097, array);
    if (!retry) return fail("nothrow allocation retry failed");
    m22_free_nothrow_across_target(retry, array, 0);
    m22_free_nothrow_across_target(nullptr, array, 2);
  }
  const auto raw = zh::original_process::live_raw_allocations();
  const auto pools = zh::original_process::live_pool_allocations();
  bool scalar_threw = false, array_threw = false;
  try { auto* value = new (std::nothrow) ThrowingScalar; delete value; }
  catch (int value) { scalar_threw = value == 17; }
  try { auto* value = new (std::nothrow) ThrowingArray[2]; delete[] value; }
  catch (int value) { array_threw = value == 19; }
  if (!scalar_threw || !array_threw || ThrowingArray::constructed != 2 ||
      ThrowingArray::destroyed != 1 || zh::original_process::live_raw_allocations() != raw ||
      zh::original_process::live_pool_allocations() != pools)
    return fail("nothrow constructor unwind failed exact placement cleanup");
  // The correction must not turn the original throwing allocation into null.
  fail_next_raw_malloc = true;
  bool throwing_failed = false;
  try { auto* value = m26_allocate_across_target(4097); m26_free_across_target(value); }
  catch (...) { throwing_failed = true; }
  if (!throwing_failed || fail_next_raw_malloc ||
      zh::original_process::live_raw_allocations() != raw)
    return fail("original throwing allocation failure semantics changed");
  return 0;
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

  if (nothrow_pairing()) return 1;

  void* aligned = m26_allocate_aligned(37, 64);
  if ((reinterpret_cast<std::uintptr_t>(aligned) & 63U) != 0)
    return fail("aligned allocation did not meet requested alignment");
  if (zh::original_process::live_raw_allocations() != raw_before)
    return fail("aligned allocation unexpectedly entered original raw allocator");
  m26_free_aligned(aligned, 64);
  if (zh::original_process::live_raw_allocations() != raw_before)
    return fail("aligned cross-target free changed original raw allocation count");

  // This overload is used by third-party DSOs and can pair with the process's
  // aligned delete. It must not be interpreted as an original private header.
  void* foreign_style = ::operator new(1536, std::align_val_t{64}, std::nothrow);
  if (!foreign_style || (reinterpret_cast<std::uintptr_t>(foreign_style) & 63U) != 0)
    return fail("aligned nothrow allocation did not meet requested alignment");
  ::operator delete(foreign_style, std::align_val_t{64});
  if (zh::original_process::live_raw_allocations() != raw_before)
    return fail("aligned nothrow pair changed original raw allocation count");

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
