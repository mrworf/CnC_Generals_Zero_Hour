// SPDX-License-Identifier: GPL-3.0-or-later
// Actual original helper constructor ownership, not window animation parity.
#include "AllocationFault.h"
#include "Common/GameMemory.h"
#include "GameClient/AnimateWindowManager.h"
class Display;
extern Display* TheDisplay;
#include <iostream>
#include <stdexcept>
namespace {
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void run(){AnimateWindowManager owner(1024);owner.init();owner.reset();owner.update();}
void faults(){
  std::size_t census=0;
  {AllocationFault::arm(SIZE_MAX);try{run();}catch(...){AllocationFault::disarm();throw;}
    census=AllocationFault::attempts();AllocationFault::disarm();}
  require(census==8,"all eight actual source process helpers");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){const auto live=AllocationFault::live();
    bool failed=false;AllocationFault::arm(ordinal);try{run();}catch(const std::bad_alloc&){failed=true;}
    AllocationFault::disarm();require(failed==(ordinal<census) && failed==AllocationFault::triggered() &&
      (failed || AllocationFault::attempts()==census),"every constructor failure/retry and exact terminal");
    require(AllocationFault::live()==live,"constructor unwinds all acquired helper units");
    if(failed)run();require(AllocationFault::live()==live,"corrected complete owner retires exactly");
  }std::cout<<"animation helper ordinals [0,"<<census<<"); terminal "<<census<<'\n';
}
}
int main(){bool initialized=false;
  try{initMemoryManager();initialized=true;require(!TheDisplay,"no synthetic display provider");
    bool rejected=false;try{AnimateWindowManager invalid;}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}
    require(rejected,"default owner requires actual display metadata");
    for(unsigned repeat=0;repeat<3;++repeat)faults();shutdownMemoryManager();initialized=false;
    std::cout<<"PASS actual animation constructor; registered-window lifecycle pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode){std::cerr<<"FAIL actual animation ownership\n";}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
