#include "PreRTS.h"

#include <cstdio>

extern "C" void ReleaseCrash(const char *reason)
{
  if (reason) std::fprintf(stderr, "Fatal Zero Hour error: %s\n", reason);
  std::abort();
}
