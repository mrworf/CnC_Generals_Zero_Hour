// SPDX-License-Identifier: GPL-3.0-or-later
// Native original-engine startup. No physical output or simulation tick is
// claimed by this entry; later N2 slices extend the same process boundary.
#include "PreRTS.h"
#include "Common/NativeHeadlessGameEngine.h"
#include "Common/GameMemory.h"
#include <cstdio>
#include <exception>
#include <memory>
#include <string_view>

namespace {
void usage() {
  std::puts("zero_hour_headless --help");
  std::puts("zero_hour_headless --startup --data-root PATH [--data-root PATH ...] --user-data PATH --user-cache PATH");
  std::puts("Startup reads mounted game data and writes only to explicit user storage.");
}
int start(NativeHeadlessConfiguration config) {
  initMemoryManager();
  try {
    auto engine=std::make_unique<NativeHeadlessGameEngine>(std::move(config));
    TheGameEngine=engine.get();
    try { engine->init(0,nullptr); }
    catch (...) { engine.reset();TheGameEngine=nullptr;throw; }
    const bool ready=!engine->getQuitting();
    engine.reset();
    TheGameEngine=nullptr;
    shutdownMemoryManager();
    if(!ready) { std::fputs("original startup requested quit\n",stderr);return 1; }
    std::puts("original startup and shutdown complete");
    return 0;
  } catch (...) {
    shutdownMemoryManager();
    throw;
  }
}
}
int main(int argc,char** argv) {
  if(argc==2 && std::string_view(argv[1])=="--help") { usage();return 0; }
  NativeHeadlessConfiguration config;
  bool startup=false;
  for(int n=1;n<argc;++n) {
    const std::string_view flag(argv[n]);
    if(flag=="--startup" && !startup) { startup=true;continue; }
    if(n+1>=argc) {usage();return 2;}
    if(flag=="--data-root") config.dataRoots.emplace_back(argv[++n]);
    else if(flag=="--user-data") config.userPaths.data=argv[++n];
    else if(flag=="--user-cache") config.userPaths.cache=argv[++n];
    else {usage();return 2;}
  }
  if(!startup || config.dataRoots.empty() || config.userPaths.data.empty() || config.userPaths.cache.empty()) {
    usage();return 2;
  }
  try { return start(std::move(config)); }
  catch (...) { std::fputs("original startup rejected supplied configuration or data\n",stderr);return 1; }
}
