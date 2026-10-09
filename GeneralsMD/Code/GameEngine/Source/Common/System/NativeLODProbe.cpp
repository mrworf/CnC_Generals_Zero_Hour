// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "Common/GameLOD.h"
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <limits>
namespace {
std::optional<Int> frequencyMHz() {
  const int descriptor=::open("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq",O_RDONLY|O_CLOEXEC);
  if(descriptor<0)return std::nullopt;
  struct Retire {int descriptor;~Retire(){::close(descriptor);}} retire{descriptor};
  char bytes[64];ssize_t count;
  do {count=::read(descriptor,bytes,sizeof(bytes));}while(count<0 && errno==EINTR);
  if(count<=0 || count==sizeof(bytes))return std::nullopt;
  std::uint64_t khz=0;bool digit=false,trailing=false;
  for(ssize_t index=0;index<count;++index){const char value=bytes[index];
    if(value=='\n' || value==' ' || value=='\t' || value=='\r'){if(digit)trailing=true;continue;}
    if(trailing || value<'0' || value>'9' || khz>(std::numeric_limits<std::uint64_t>::max()-9)/10)return std::nullopt;
    digit=true;khz=khz*10+static_cast<unsigned>(value-'0');
  }
  const auto mhz=khz/1000;
  if(!digit || !mhz || mhz>std::uint64_t(std::numeric_limits<Int>::max()))return std::nullopt;
  return static_cast<Int>(mhz);
}
class NativeProbe final:public NativeLODProbe {
public:
  NativeLODHardware hardware() override {
    const long pages=::sysconf(_SC_PHYS_PAGES),size=::sysconf(_SC_PAGESIZE);
    if(pages<=0 || size<=0 || std::uint64_t(pages)>std::numeric_limits<std::uint64_t>::max()/std::uint64_t(size))throw ERROR_BAD_ARG;
    const auto frequency=frequencyMHz();
    // A modern vendor/frequency does not establish an old benchmark CPU class.
    return {std::uint64_t(pages)*std::uint64_t(size),XX,frequency.value_or(0),frequency.has_value()};
  }
  std::optional<NativeLODLegacyScores> legacyScores() override {return std::nullopt;}
  std::optional<ChipsetType> chipset() override {return std::nullopt;}
};
}
NativeLODProbe& nativeLODProbe(){static NativeProbe owner;return owner;}
