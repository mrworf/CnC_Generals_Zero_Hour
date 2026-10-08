// SPDX-License-Identifier: GPL-3.0-or-later
// Actual template AudioArray owner; not full template/module gameplay acceptance.
#include "AllocationFault.h"
#include "Common/ThingTemplate.h"
#include "Common/GlobalData.h"
#include "Common/NativeUserStorage.h"
#include <filesystem>
#include <array>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

namespace {
void require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
constexpr std::array<Int,3> slots{0, TTAUDIO_COUNT / 2, TTAUDIO_COUNT - 1};
struct AudioProbe : AudioEventRTS {
  using AudioEventRTS::AudioEventRTS;
  bool initialized() const {
    return m_objectID==INVALID_ID && m_positionOfAudio.x==0 && m_positionOfAudio.y==0 &&
      m_positionOfAudio.z==0 && m_portionToPlayNext==PP_Attack;
  }
};
void seed(AudioArray& value) {
  for (Int slot : slots) {
    value.m_audio[slot] = newInstance(DynamicAudioEventRTS);
    value.m_audio[slot]->m_event.setEventName("generated-template-audio");
  }
}
void copied(const AudioArray& source, const AudioArray& value) {
  for (Int slot = 0; slot < TTAUDIO_COUNT; ++slot) {
    if (source.m_audio[slot]) {
      require(value.m_audio[slot] && value.m_audio[slot] != source.m_audio[slot] &&
        value.m_audio[slot]->m_event.getEventName() == source.m_audio[slot]->m_event.getEventName(),
        "complete distinct cloned audio ownership");
    } else require(!value.m_audio[slot], "entire inactive array tail initialized");
  }
}
void values() {
  alignas(AudioProbe) std::array<unsigned char,sizeof(AudioProbe)> eventBacking;
  eventBacking.fill(0xa5);
  auto* event = new(eventBacking.data()) AudioProbe;
  require(event->initialized(), "inactive audio aggregate/phase initialized from nonzero backing");
  AudioProbe eventCopy(*event);
  require(eventCopy.initialized(), "copy does not propagate indeterminate inactive fields");
  event->~AudioProbe();
  alignas(AudioArray) std::array<unsigned char,sizeof(AudioArray)> backing;
  backing.fill(0xa5);
  auto* empty = new(backing.data()) AudioArray;
  for (auto* event : empty->m_audio) require(!event, "fresh nonzero backing initialized");
  empty->~AudioArray();
  auto* pool = TheMemoryPoolFactory->createMemoryPool("DynamicAudioEventRTS",sizeof(DynamicAudioEventRTS),16,0);
  {
    AudioArray source; seed(source);
    AudioArray clone(source); copied(source,clone);
    clone.m_audio[0]->m_event.setEventName("independent-clone");
    require(source.m_audio[0]->m_event.getEventName()=="generated-template-audio", "cloned COW event identity");
    AllocationFault::arm(0); clone=clone; AllocationFault::disarm();
    require(!AllocationFault::triggered() && clone.m_audio[0]->m_event.getEventName()=="independent-clone",
      "self-assignment preserves owner without allocation");
    AudioArray target; target=source; copied(source,target);
    AudioArray cleared;
    target=cleared;
    for (auto* event : target.m_audio) require(!event, "assignment retires previously occupied slots");
    require(pool->getUsedBlockCount()==6, "null-source assignment does not lose owned audio");
    AllocationFault::arm(0); source.swap(clone); AllocationFault::disarm();
    require(!AllocationFault::triggered(), "complete audio publication allocation-free");
  }
  require(pool->getUsedBlockCount()==0, "complete audio lifecycle retirement");
  TheMemoryPoolFactory->destroyMemoryPool(pool);
}
void faults(bool assignment) {
  // The pool has a fixed nine-unit physical backing. Holding raw pool units
  // leaves exactly ordinal slots for a three-acquisition copy; no production
  // fault hook, fake event, arena shrink or timing-dependent allocator is used.
  for (Int ordinal=0; ordinal<=3; ++ordinal) {
    auto* pool = TheMemoryPoolFactory->createMemoryPool("DynamicAudioEventRTS",sizeof(DynamicAudioEventRTS),9,0);
    {
      AudioArray source; seed(source);
      AudioArray target;
      target.m_audio[1]=newInstance(DynamicAudioEventRTS);
      target.m_audio[1]->m_event.setEventName("accepted-audio");
      auto* accepted=target.m_audio[1];
      std::array<void*,5> held{};
      for (Int i=0; i<5-ordinal; ++i) held[i]=pool->allocateBlock("generated capacity owner");
      const auto live=AllocationFault::live();
      const auto used=pool->getUsedBlockCount();
      bool rejected=false;
      try {
        if (assignment) target=source;
        else { AudioArray candidate(source); copied(source,candidate); }
      } catch (ErrorCode error) {
        if (error!=ERROR_OUT_OF_MEMORY) throw;
        rejected=true;
      }
      if (ordinal<3) {
        require(rejected && pool->getUsedBlockCount()==used && AllocationFault::live()==live &&
          target.m_audio[1]==accepted && accepted->m_event.getEventName()=="accepted-audio",
          "every copied prefix retires before failed constructor/assignment returns");
        for (Int slot : slots) require(source.m_audio[slot] &&
          source.m_audio[slot]->m_event.getEventName()=="generated-template-audio", "source survives rejected clone");
      } else require(!rejected, "exact three-acquisition terminal succeeds");
      for (void* unit : held) if (unit) pool->freeBlock(unit);
      if (assignment) { target=source; copied(source,target); }
      else { AudioArray retry(source); copied(source,retry); }
    }
    require(pool->getUsedBlockCount()==0, "pool and source/accepted/clone units all retired");
    TheMemoryPoolFactory->destroyMemoryPool(pool);
  }
  std::cout<<(assignment ? "audio assignment" : "audio constructor")
    <<" acquisition ordinals [0,3) complete; terminal 3\n";
}
struct TemplateProbe : ThingTemplate {
  ~TemplateProbe() override = default; // exposes stack lifetime, not replacement behavior
};
struct DataParents {
  std::filesystem::path root;
  FileSystem files;
  std::unique_ptr<NativeUserStorage> storage;
  DataParents() {
    char pattern[]="/tmp/zh-template-parents-XXXXXX";
    const auto* directory=::mkdtemp(pattern);
    require(directory,"generated template parent root"); root=directory;
    try {
      std::filesystem::create_directory(root/"assets");
      files.mountReadOnly({(root/"assets").string()});
      storage=std::make_unique<NativeUserStorage>(
        NativeUserPaths::resolve(root.string(),(root/"data").string(),(root/"cache").string()),files);
      TheFileSystem=&files; TheNativeUserStorage=storage.get();
    } catch(...) {
      std::error_code ignored;std::filesystem::remove_all(root,ignored);throw;
    }
  }
  ~DataParents() {
    TheFileSystem=nullptr; TheNativeUserStorage=nullptr;
    std::error_code ignored;std::filesystem::remove_all(root,ignored);
  }
};
void templateOwner(bool fault) {
  DataParents parents;
  GlobalData globals;
  globals.m_defaultOcclusionDelay=37;
  TheWritableGlobalData=&globals;
  struct Withdraw { ~Withdraw(){TheWritableGlobalData=nullptr;} } withdraw;
  auto* pool=TheMemoryPoolFactory->createMemoryPool("ThingTemplatePool",sizeof(ThingTemplate),16,0);
  {
    alignas(TemplateProbe) std::array<unsigned char,sizeof(TemplateProbe)> bytes;
    bytes.fill(0xa5);
    auto* fresh=::new(bytes.data()) TemplateProbe;
    require(fresh->getAssetScale()==0 && fresh->getDisplayColor()==0 && !fresh->isBridge() &&
      fresh->getTemplateID()==0 && fresh->friend_getNextTemplate()==nullptr &&
      fresh->friend_getNextOverride()==nullptr,"source zeroed-pool defaults explicit on nonzero backing");
    fresh->~TemplateProbe();
    TemplateProbe source, next;
    source.friend_setTemplateName("accepted-source");
    source.friend_setTemplateID(7);
    source.friend_setNextTemplate(&next);
    auto& variants=const_cast<std::vector<AsciiString>&>(source.getBuildVariations());
    variants.emplace_back("generated-variant-one"); variants.emplace_back("generated-variant-two");
    auto* child=newInstance(ThingTemplate);
    child->friend_setTemplateName("accepted-override");
    source.setNextOverride(child);
    auto action=[&] {
      TemplateProbe candidate;
      candidate.friend_setTemplateName("candidate-identity");
      candidate.friend_setTemplateID(11);
      candidate.friend_setNextTemplate(&next);
      candidate.markAsOverride();
      candidate.copyFrom(&source);
      require(candidate.getName()=="candidate-identity" && candidate.getTemplateID()==11 &&
        candidate.friend_getNextTemplate()==&next && candidate.friend_getNextOverride()==nullptr && candidate.friend_isOverride() &&
        candidate.getBuildVariations()==source.getBuildVariations(),
        "actual clone preserves destination identity and does not acquire source override ownership");
    };
    action();
    const auto live=AllocationFault::live(); const auto used=pool->getUsedBlockCount();
    auto accepted=[&] {
      require(source.friend_getNextOverride()==child && child->getName()=="accepted-override" &&
        source.getName()=="accepted-source" && source.getTemplateID()==7 &&
        source.friend_getNextTemplate()==&next && source.getBuildVariations().size()==2 &&
        pool->getUsedBlockCount()==used && AllocationFault::live()==live,
        "clone retirement preserves accepted source graph and exact resources");
    };
    accepted();
    if (fault) {
      AllocationFault::arm(std::numeric_limits<std::size_t>::max()); action();
      const auto census=AllocationFault::attempts(); AllocationFault::disarm(); accepted();
      require(census>0 && census<64,"independent actual template clone census");
      for (std::size_t ordinal=0; ordinal<=census; ++ordinal) {
        bool rejected=false; AllocationFault::arm(ordinal);
        try { action(); } catch(const std::bad_alloc&){rejected=true;}
        catch(...){AllocationFault::disarm();throw;}
        AllocationFault::disarm(); accepted();
        if (ordinal<census) {
          require(rejected && AllocationFault::triggered(),"every template clone ordinal rejects");
          action(); accepted();
        } else require(!rejected && !AllocationFault::triggered() && AllocationFault::attempts()==census,
          "actual clone exact terminal boundary");
      }
      std::cout<<"template clone ordinals [0,"<<census<<") complete; terminal "<<census<<'\n';
    }
  }
  require(pool->getUsedBlockCount()==0,"source retires its own override exactly once");
  TheMemoryPoolFactory->destroyMemoryPool(pool);
  const auto live=AllocationFault::live();
  TheWritableGlobalData=nullptr;
  bool rejected=false;
  try { (void)newInstance(ThingTemplate); } catch(ErrorCode){rejected=true;}
  auto* failedPool=TheMemoryPoolFactory->findMemoryPool("ThingTemplatePool");
  require(rejected && failedPool && failedPool->getUsedBlockCount()==0,"missing constructor parent returns acquired pool slot");
  TheMemoryPoolFactory->destroyMemoryPool(failedPool);
  require(AllocationFault::live()==live,"missing-parent constructor complete backing retirement");
}
}
int main(int argc,char** argv) {
  bool initialized=false;
  try {
    require(argc==2,"template backing family");
    for (int repeat=0; repeat<3; ++repeat) {
      initMemoryManager(); initialized=true;
      const std::string_view family(argv[1]);
      if (family=="values") values();
      else if (family=="fault-copy") faults(false);
      else if (family=="fault-assign") faults(true);
      else if (family=="template-values") templateOwner(false);
      else if (family=="template-faults") templateOwner(true);
      else throw std::runtime_error("unknown backing family");
      shutdownMemoryManager(); initialized=false;
    }
    std::cout<<"PASS: actual template audio backing (whole definition acceptance pending)\n"; return 0;
  } catch(ErrorCode error) { std::cerr<<"FAIL: source error "<<unsigned(error)<<'\n'; }
  catch(const std::exception& error) { std::cerr<<"FAIL: "<<error.what()<<'\n'; }
  AllocationFault::disarm(); if(initialized)shutdownMemoryManager(); return 1;
}
