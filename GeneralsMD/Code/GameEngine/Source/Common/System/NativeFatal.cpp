// SPDX-License-Identifier: GPL-3.0-or-later
// Fatal reporting is independent of game/storage/presentation owner lifetimes.
#include "Common/Debug.h"
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>

namespace {
[[noreturn]] void fatal() noexcept {
  // Reasons/localized identifiers may contain user paths or depend on retired
  // owners. Do not read them, allocate a report, or call game/UI services here.
  constexpr char report[]=
      "Zero Hour: fatal engine error. Runtime state is invalid; restart the game. "
      "Private error details withheld.\n";
  // A broken stderr pipe must not replace the defined failure exit with SIGPIPE.
  sigset_t blocked;
  if(::sigemptyset(&blocked)==0 && ::sigaddset(&blocked,SIGPIPE)==0)
    ::pthread_sigmask(SIG_BLOCK,&blocked,nullptr);
  std::size_t written=0;
  while(written<sizeof(report)-1) {
    const auto count=::write(STDERR_FILENO,report+written,sizeof(report)-1-written);
    if(count>0) written+=static_cast<std::size_t>(count);
    else if(count<0 && errno==EINTR) continue;
    else break;
  }
  // Match the source's immediate exit: no atexit callbacks into corrupted or
  // already-retired simulation owners, nor destructor-driven asset writes.
  ::_exit(EXIT_FAILURE);
}
}
extern "C" void ReleaseCrash(const char*) {fatal();}
extern "C" void ReleaseCrashLocalized(const AsciiString&,const AsciiString&) {fatal();}
