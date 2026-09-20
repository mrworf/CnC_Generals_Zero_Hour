#pragma once

#include <cstddef>

namespace zh::original_process {

enum class PoolConfigCode : int {
  defaults = 0,
  applied = 1,
  malformed = 2,
  oversized = 3,
  invalid_count = 4,
  duplicate = 5,
  reentrant = 6,
  too_late = 7,
  io_error = 8,
};

struct PoolConfigStatus {
  PoolConfigCode code;
  int recognized;
  int ignored;
  const char* source;
};

struct ServiceCounts {
  int synchronization;
  int logging;
  int version;
  int workers;
};

PoolConfigStatus pool_config_status() noexcept;
std::size_t live_raw_allocations() noexcept;
std::size_t live_pool_allocations() noexcept;
bool initialize_services(int fail_after_stage, char* diagnostic, std::size_t diagnostic_size) noexcept;
void shutdown_services() noexcept;
ServiceCounts service_counts() noexcept;
#if defined(ZH_ORIGINAL_RUNTIME_TEST_HOOKS)
bool test_pool_config_reentry_guard() noexcept;
#endif

}  // namespace zh::original_process
