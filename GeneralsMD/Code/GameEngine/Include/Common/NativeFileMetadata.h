// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <chrono>
#include <cstdint>
#include <sys/stat.h>

// Original FileInfo timestamp protocol: 100ns ticks since 1601, not native
// time_t bytes. Shared by protected user files and immutable asset providers.
inline bool nativeFileTimestamp(const struct stat& info,std::uint64_t& result) noexcept {
  using namespace std::chrono;
  constexpr auto epoch=duration_cast<seconds>(sys_days{year{1970}/January/1}-
      sys_days{year{1601}/January/1}).count();
  const auto seconds=info.st_mtim.tv_sec,nanos=info.st_mtim.tv_nsec;
  if(seconds < -epoch || nanos<0 || nanos>=1000000000) return false;
  const auto shifted=seconds<0?std::uint64_t(epoch+seconds):
      std::uint64_t(seconds)+std::uint64_t(epoch);
  const auto tail=std::uint64_t(nanos)/100;
  if(shifted>(UINT64_MAX-tail)/10000000) return false;
  result=shifted*10000000+tail;
  return true;
}
