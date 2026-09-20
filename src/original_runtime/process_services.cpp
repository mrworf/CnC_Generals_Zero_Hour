#include "PreRTS.h"
#include "Common/CriticalSection.h"
#include "Common/version.h"
#include "zh/original_process.h"

#include <cstdio>

namespace zh::original_process {
namespace {

ServiceCounts counts{};
bool initialized = false;

void diagnostic(char* output, std::size_t size, const char* value) noexcept
{
  if (!output || !size) return;
  std::snprintf(output, size, "%s", value);
}

}  // namespace

bool initialize_services(int fail_after_stage, char* output, std::size_t size) noexcept
{
  if (initialized) {
    diagnostic(output, size, "already initialized");
    return true;
  }
  initMemoryManager();
  try {
    TheUnicodeStringCriticalSection = new CriticalSection;
    TheDmaCriticalSection = new CriticalSection;
    TheMemoryPoolCriticalSection = new CriticalSection;
    TheDebugLogCriticalSection = new CriticalSection;
    counts.synchronization = 4;
    if (fail_after_stage == 1) throw ERROR_OUT_OF_MEMORY;

    counts.logging = 1;
    if (fail_after_stage == 2) throw ERROR_OUT_OF_MEMORY;

    TheVersion = new Version;
    counts.version = 1;
    if (fail_after_stage == 3) throw ERROR_OUT_OF_MEMORY;

    initialized = true;
    diagnostic(output, size, "ok");
    return true;
  } catch (...) {
    diagnostic(output, size, "process service initialization failed");
    shutdown_services();
    return false;
  }
}

void shutdown_services() noexcept
{
  Version* version = TheVersion;
  TheVersion = nullptr;
  delete version;
  counts.version = 0;
  counts.logging = 0;

  CriticalSection* debug = TheDebugLogCriticalSection;
  CriticalSection* memory = TheMemoryPoolCriticalSection;
  CriticalSection* dma = TheDmaCriticalSection;
  CriticalSection* unicode = TheUnicodeStringCriticalSection;
  TheDebugLogCriticalSection = nullptr;
  TheMemoryPoolCriticalSection = nullptr;
  TheDmaCriticalSection = nullptr;
  TheUnicodeStringCriticalSection = nullptr;
  delete debug;
  delete memory;
  delete dma;
  delete unicode;
  counts.synchronization = 0;
  counts.workers = 0;
  initialized = false;
}

ServiceCounts service_counts() noexcept { return counts; }

}  // namespace zh::original_process
