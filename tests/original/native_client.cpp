// SPDX-License-Identifier: GPL-3.0-or-later
// Actual client/Drawable owners, not a renderer or full startup acceptance.
#include "PreRTS.h"
#include "AllocationFault.h"
#include "Common/GameMemory.h"
#include "Common/GlobalData.h"
#include "Common/FileSystem.h"
#include "Common/MessageStream.h"
#include "Common/ThingTemplate.h"
#include "GameClient/NativeHeadlessGameClient.h"
#include "GameClient/Drawable.h"
#include "GameClient/RayEffect.h"
#include "GameClient/DrawGroupInfo.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/GhostObject.h"
#include <memory>
#include <iostream>
#include <limits>
namespace {
// Exposes destruction of the actual source template without replacing data,
// parsing, modules or gameplay behavior.
struct Definition:ThingTemplate {~Definition() override=default;};
struct InspectableClient:NativeHeadlessGameClient {using GameClient::updateDrawableState;};
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
template<class F>void rejects(F action){bool rejected=false;try{action();}catch(ErrorCode){rejected=true;}require(rejected,"invalid native client admission rejects");}
struct Context {
  FileSystem files;MessageStream messages;
  std::unique_ptr<GlobalData> data;
  FileSystem* previousFiles=TheFileSystem;GlobalData* previousData=TheWritableGlobalData;
  MessageStream* previousMessages=TheMessageStream;
  Context(){TheFileSystem=&files;TheMessageStream=&messages;
    try{data=std::make_unique<GlobalData>();TheWritableGlobalData=data.get();}
    catch(...){TheFileSystem=previousFiles;TheMessageStream=previousMessages;throw;}}
  ~Context(){TheWritableGlobalData=data.get();data.reset();TheWritableGlobalData=previousData;TheMessageStream=previousMessages;TheFileSystem=previousFiles;}
};
struct Client {
  GameClient* previous=TheGameClient;
  std::unique_ptr<InspectableClient> owner;
  Client(){owner=std::make_unique<InspectableClient>();TheGameClient=owner.get();}
  ~Client(){TheGameClient=owner.get();owner.reset();TheGameClient=previous;}
  void init(){owner->init();}
};
void functional(){Context context;Definition definition;Client client;client.init();
  require(client.owner->isHeadless() && TheRayEffects,"explicit absent output retains actual ray-effect owner");
  Drawable* first=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);
  Drawable* second=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);
  const auto firstID=first->getID(),secondID=second->getID();
  require(firstID!=INVALID_DRAWABLE_ID && secondID!=firstID && client.owner->findDrawableByID(firstID)==first &&
    client.owner->findDrawableByID(secondID)==second && client.owner->firstDrawable()==second,"actual source registration/list/lookup distinct identity");
  rejects([&]{first->setID(secondID);});
  rejects([&]{client.owner->registerDrawable(first);});
  require(first->getID()==firstID && client.owner->findDrawableByID(firstID)==first && client.owner->findDrawableByID(secondID)==second,
    "duplicate ID admission retains both actual owners");
  require(!client.owner->findDrawableByID(static_cast<DrawableID>(std::numeric_limits<UnsignedInt>::max())),"raw UInt32 lookup never narrows to a negative index");
  rejects([&]{client.owner->init();});rejects([&]{client.owner->update();});
  require(client.owner->firstDrawable()==second,"incomplete simulation/update rejection never discards logical state");
  client.owner->destroyDrawable(first);require(!client.owner->findDrawableByID(firstID) && client.owner->findDrawableByID(secondID)==second,"exact source registry withdrawal");
  AllocationFault::arm(0);bool failed=false;try{client.owner->reset();}catch(const std::bad_alloc&){failed=true;}AllocationFault::disarm();
  require(failed && client.owner->findDrawableByID(secondID)==second && client.owner->firstDrawable()==second,"failed reset preparation preserves accepted drawable and lookup");
  client.owner->reset();require(!client.owner->firstDrawable() && !client.owner->findDrawableByID(secondID),"corrected reset retires original drawable graph");
  const Coord3D start{0,2,4},end{4,6,8};client.owner->createRayEffectByTemplate(&start,&end,&definition);
  Drawable* ray=client.owner->firstDrawable();RayEffectData captured{};client.owner->getRayEffectData(ray,&captured);
  require(ray && captured.draw==ray && captured.startLoc.x==0 && captured.endLoc.z==8 && ray->getPosition()->x==2,
    "actual source ray endpoints and midpoint retained without a synthetic renderer");
  require(!client.owner->friend_createDrawable(nullptr,DRAWABLE_STATUS_NONE),"original null-template factory contract");
}
void frames(){Context context;Definition definition;GameLogic logic;GhostObjectManager ghosts;
  const auto previousLogic=TheGameLogic;const auto previousGhosts=TheGhostObjectManager;
  struct Restore {GameLogic* logic;GhostObjectManager* ghosts;~Restore(){TheGameLogic=logic;TheGhostObjectManager=ghosts;}} restore{previousLogic,previousGhosts};
  TheGameLogic=&logic;TheGhostObjectManager=&ghosts;
  {Client client;client.init();auto* draw=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);draw->fadeOut(2);
    client.owner->setFrame(0);client.owner->updateDrawableState(FALSE,0);client.owner->updateDrawableState(FALSE,0);
    require(draw->getEffectiveOpacity()==1.0f,"source repeated frame does not advance drawable fade twice");
    client.owner->setFrame(1);client.owner->updateDrawableState(FALSE,0);require(draw->getEffectiveOpacity()==0.5f,"original drawable fade transition");
    client.owner->setFrame(2);client.owner->updateDrawableState(TRUE,0);require(draw->getEffectiveOpacity()==0.5f,"source frozen frame retains drawable state");
    client.owner->setFrame(3);client.owner->updateDrawableState(FALSE,0);require(draw->getEffectiveOpacity()==0.0f,"source unfreezing resumes actual drawable update");
    client.owner->setFrame(0);client.owner->updateDrawableState(FALSE,0);}
  {Client replacement;replacement.init();auto* draw=replacement.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);draw->fadeOut(1);
    replacement.owner->setFrame(0);replacement.owner->updateDrawableState(FALSE,0);
    replacement.owner->setFrame(1);replacement.owner->updateDrawableState(FALSE,0);
    require(draw->getEffectiveOpacity()==0.0f,"replaced source owner does not inherit previous owner's last-frame suppression");}
}
void drawFaults(){Context context;Definition definition;Client client;client.init();
  Drawable* accepted=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);const auto acceptedID=accepted->getID();
  std::size_t census;
  {AllocationFault::arm(SIZE_MAX);Drawable* discovery=nullptr;try{discovery=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);}catch(...){AllocationFault::disarm();throw;}
    census=AllocationFault::attempts();AllocationFault::disarm();client.owner->destroyDrawable(discovery);}
  auto* pool=TheMemoryPoolFactory->findMemoryPool("Drawable");require(pool && census>0 && census<128,"complete bounded source drawable acquisition manifest");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){const auto live=AllocationFault::live();const auto units=pool->getUsedBlockCount();
    const auto next=client.owner->getDrawableIDCounter();Drawable* candidate=nullptr;bool failed=false;
    AllocationFault::arm(ordinal);try{candidate=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);}catch(const std::bad_alloc&){failed=true;}AllocationFault::disarm();
    require(failed==(ordinal<census) && failed==AllocationFault::triggered() && (failed || AllocationFault::attempts()==census),"each actual drawable allocation prefix and exact terminal");
    if(failed){require(client.owner->getDrawableIDCounter()==next && client.owner->firstDrawable()==accepted &&
        pool->getUsedBlockCount()==units && AllocationFault::live()==live,"failed source constructor withdraws identity, arrays and pool unit exactly");
      candidate=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);}
    require(client.owner->findDrawableByID(acceptedID)==accepted && client.owner->findDrawableByID(candidate->getID())==candidate,"accepted source drawable survives candidate/retry");
    client.owner->destroyDrawable(candidate);require(pool->getUsedBlockCount()==units && AllocationFault::live()==live,"complete corrected constructor owner retirement");
  }std::cout<<"drawable prefixes [0,"<<census<<"); terminal "<<census<<'\n';
}
void clientFaults(){Context context;
  auto operation=[] {Client client;client.init();};std::size_t census;
  {AllocationFault::arm(SIZE_MAX);try{operation();}catch(...){AllocationFault::disarm();throw;}census=AllocationFault::attempts();AllocationFault::disarm();}
  require(census>0 && census<128,"complete bounded actual client acquisition manifest");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){const auto live=AllocationFault::live();bool failed=false;
    AllocationFault::arm(ordinal);try{operation();}catch(const std::bad_alloc&){failed=true;}AllocationFault::disarm();
    require(failed==(ordinal<census) && failed==AllocationFault::triggered() && (failed || AllocationFault::attempts()==census),"every actual client acquisition failure and exact terminal");
    require(AllocationFault::live()==live && !TheGameClient && !TheRayEffects && !TheDrawGroupInfo,"partial client retires acquired backing/publications");
    if(failed)operation();require(AllocationFault::live()==live,"corrected complete client retry retires exactly");
  }std::cout<<"client prefixes [0,"<<census<<"); terminal "<<census<<'\n';
}
}
int main(int argc,char** argv){const auto initial=AllocationFault::live();bool initialized=false;
  try{require(argc==2,"native client family required");initMemoryManager();initialized=true;
    {Context context;Definition definition;Client client;client.init();auto* drawable=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);client.owner->destroyDrawable(drawable);}
    for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();const std::string family=argv[1];
      if(family=="functional")functional();else if(family=="frames")frames();else if(family=="draw-faults")drawFaults();else if(family=="client-faults")clientFaults();else throw std::runtime_error("unknown native client family");
      require(AllocationFault::live()==live && !TheGameClient && !TheRayEffects && !TheWritableGlobalData && !TheFileSystem && !TheMessageStream,"complete source client/context retirement");}
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole original metadata retirement");
    std::cout<<"PASS actual headless client/Drawable owner; full startup/modules/output pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode){std::cerr<<"FAIL native source client admission\n";}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
