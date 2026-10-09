// SPDX-License-Identifier: GPL-3.0-or-later
// Actual generated catalog/request ownership. No dialog or GUI acceptance.
#include "PreRTS.h"
#include "AllocationFault.h"
#include "Common/GameMemory.h"
#include "Common/AsciiString.h"
#include "Common/FileSystem.h"
#include "Common/NativeWarningBox.h"
#include "GameClient/GameText.h"
#include "GameClient/GameClient.h"
#include "GameClient/TerrainVisual.h"
#include <filesystem>
#include <cstring>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <unistd.h>
#include <type_traits>
// Compile the real device contracts without instantiating fake presentation.
class InputFactoryContract:public GameClient {
  IMEManagerInterface* createIMEManager() override=0;
};
static_assert(std::is_abstract_v<InputFactoryContract>);
static_assert(std::is_same_v<decltype(&TerrainVisual::oversizeTerrain),void(TerrainVisual::*)(Int)>);
namespace {
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void little(std::string& bytes,std::uint32_t value){for(unsigned shift:{0,8,16,24})bytes.push_back(static_cast<char>(value>>shift));}
std::string catalog(){
  std::string bytes;for(auto value:{0x43534620u,3u,2u,2u,0u,0u})little(bytes,value);
  for(const char* label:{"TITLE","MESSAGE"}){
    little(bytes,0x4c424c20u);little(bytes,1);little(bytes,static_cast<std::uint32_t>(std::strlen(label)));bytes+=label;
    little(bytes,0x53545220u);
    const std::vector<std::uint16_t> text{u'G',u'e',u'n',u'e',u'r',u'a',u't',u'e',u'd',u' ',u'Ω',u' ',0xd83d,0xde00,u' ',u'l',u'o',u'c',u'a',u'l',u'i',u'z',u'e',u'd',u' ',u'w',u'a',u'r',u'n',u'i',u'n',u'g'};
    little(bytes,static_cast<std::uint32_t>(text.size()));
    for(auto unit:text){auto encoded=static_cast<std::uint16_t>(~unit);bytes.push_back(static_cast<char>(encoded));bytes.push_back(static_cast<char>(encoded>>8));}
  }return bytes;
}
struct Context {
  std::filesystem::path path;FileSystem files;
  std::unique_ptr<GameTextInterface> text;
  FileSystem* previous=TheFileSystem;
  AsciiString title{"TITLE"},message{"MESSAGE"};
  Context(){
    char pattern[]="/tmp/zh-warning-XXXXXX";const char* root=::mkdtemp(pattern);require(root,"generated catalog root");path=root;
    try {
      const auto file=path/"Data/English/Generals.csf";std::filesystem::create_directories(file.parent_path());
      const auto bytes=catalog();std::ofstream output(file,std::ios::binary);output.write(bytes.data(),bytes.size());output.close();require(bool(output),"generated CSF write");
      files.mountReadOnly({path.string()});TheFileSystem=&files;
      text.reset(CreateGameTextInterface());text->loadCSF("Data/English/Generals.csf");
    }catch(...){TheFileSystem=previous;std::error_code ignored;std::filesystem::remove_all(path,ignored);throw;}
  }
  ~Context(){text.reset();TheFileSystem=previous;std::error_code ignored;std::filesystem::remove_all(path,ignored);}
};
struct Observer {
  unsigned calls=0;OSDisplayButtonType answer=OSDBT_OK;Bool fail=FALSE;
  static OSDisplayButtonType present(const NativeWarningRequest& request,void* context){
    auto& owner=*static_cast<Observer*>(context);++owner.calls;
    require(request.title=="Generated Ω 😀 localized warning" && request.message==request.title,
        "actual UTF-16 catalog becomes complete UTF-8, not lossy ASCII");
    require(request.buttons && request.options==(OSDOF_SYSTEMMODAL|OSDOF_EXCLAMATIONICON),"source flags preserved");
    if(owner.fail)throw ERROR_BAD_ARG;return owner.answer;
  }
};
OSDisplayButtonType run(Context& context,Observer& view,UnsignedInt buttons=OSDBT_OK|OSDBT_CANCEL){
  return nativeDisplayWarning(context.text.get(),context.title,context.message,buttons,
      OSDOF_SYSTEMMODAL|OSDOF_EXCLAMATIONICON,Observer::present,&view);
}
void functional(Context& context){
  Observer observer;require(run(context,observer)==OSDBT_OK && observer.calls==1,"explicit localized confirmation");
  observer.answer=OSDBT_CANCEL;require(run(context,observer)==OSDBT_CANCEL,"explicit cancellation");
  require(run(context,observer,OSDBT_OK)==OSDBT_ERROR,"presenter cannot select unavailable button");
  observer.answer=OSDBT_ERROR;require(run(context,observer)==OSDBT_ERROR,"backend error never masquerades as cancellation/confirmation");
  require(nativeMusicRetryRequested(OSDBT_OK) && !nativeMusicRetryRequested(OSDBT_CANCEL) && !nativeMusicRetryRequested(OSDBT_ERROR),
      "missing music retry requires affirmative output, no unavailable-dialog loop");
  auto* previous=TheGameText;TheGameText=nullptr;
  require(OSDisplayWarningBox(context.title,context.message,OSDBT_OK,0)==OSDBT_ERROR,"actual public adapter rejects missing text without opening a dialog");
  TheGameText=previous;
}
void negative(Context& context){
  Observer observer;
  for(UnsignedInt buttons:{0u,4u,0x80000000u})require(run(context,observer,buttons)==OSDBT_ERROR,"invalid button request rejects before output");
  require(nativeDisplayWarning(nullptr,context.title,context.message,OSDBT_OK,0,Observer::present,&observer)==OSDBT_ERROR,"absent owner");
  require(nativeDisplayWarning(context.text.get(),context.title,context.message,OSDBT_OK,0,nullptr,&observer)==OSDBT_ERROR,"absent presenter");
  require(nativeDisplayWarning(context.text.get(),context.title,context.message,OSDBT_OK,0x20,Observer::present,&observer)==OSDBT_ERROR,"invalid option bits");
  require(observer.calls==0,"invalid requests never call observer");
  require(nativeDisplayWarning(context.text.get(),AsciiString("MISSING"),context.message,
      OSDBT_OK,0,Observer::present,&observer)==OSDBT_ERROR && observer.calls==0,
      "missing source label never displays fabricated localized output");
  observer.fail=TRUE;bool rejected=false;try{run(context,observer);}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}
  require(rejected,"synchronous callback failure propagates without retained capture");observer.fail=FALSE;
  require(run(context,observer)==OSDBT_OK,"same owner corrected retry after callback failure");
}
void faults(Context& context){
  Observer discovery;AllocationFault::arm(SIZE_MAX);try{run(context,discovery);}catch(...){AllocationFault::disarm();throw;}
  const auto census=AllocationFault::attempts();AllocationFault::disarm();require(census>0 && census<64,"complete warning allocation census");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){Observer observer;const auto live=AllocationFault::live();bool failed=false;
    AllocationFault::arm(ordinal);try{run(context,observer);}catch(const std::bad_alloc&){failed=true;}
    AllocationFault::disarm();require(failed==(ordinal<census) && failed==AllocationFault::triggered() && (failed || AllocationFault::attempts()==census),"all failure/retry pairs and exact terminal");
    require(AllocationFault::live()==live && observer.calls==(failed?0u:1u),"candidate strings retire and failed request does not call output");
    if(failed)require(run(context,observer)==OSDBT_OK && observer.calls==1,"same-owner corrected retry");
    require(AllocationFault::live()==live,"complete request retirement");
  }std::cout<<"warning ordinals [0,"<<census<<"); terminal "<<census<<'\n';
}
}
int main(int argc,char** argv){bool initialized=false;const auto initial=AllocationFault::live();
  try {require(argc==2,"warning family required");initMemoryManager();initialized=true;
    // File/catalog use warms legitimate original pool registry capacity. Retire
    // discovery before measuring live request/catalog owners, not during faults.
    {Context warm;}
    for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();
      {Context context;const std::string family=argv[1];if(family=="functional")functional(context);else if(family=="negative")negative(context);else if(family=="faults")faults(context);else throw std::runtime_error("unknown warning family");}
      require(AllocationFault::live()==live,"complete catalog/request owner retires exactly");
    }shutdownMemoryManager();initialized=false;
    require(AllocationFault::live()==initial,"whole memory owner retires warmed registry capacity exactly");
    std::cout<<"PASS actual warning ownership; physical dialog pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode){std::cerr<<"FAIL original warning boundary\n";}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
