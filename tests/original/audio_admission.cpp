// SPDX-License-Identifier: GPL-3.0-or-later
// Generated original admission/queue ownership; never physical playback acceptance.
#include "PreRTS.h"
#include "AllocationFault.h"
#include "AbsentAudioFixture.h"
#include "Common/AudioEventInfo.h"
#include "Common/AudioEventRTS.h"
#include "Common/AudioHandleSpecialValues.h"
#include "Common/AudioAffect.h"
#include "Common/AudioRequest.h"
#include "Common/AudioSettings.h"
#include "Common/AudioRandomValue.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "Common/INI.h"
#include "Common/NativeAudioAdmission.h"
#include "Common/NativeUserStorage.h"
#include "Common/RandomValue.h"
#include "Common/version.h"
#include "Common/GameMusic.h"
#include "Common/GameSounds.h"
#include <filesystem>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <vector>
#include <unistd.h>
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
struct Tree {
  std::filesystem::path root;
  Tree(){char pattern[]="/tmp/zh-audio-admission-XXXXXX";auto* value=::mkdtemp(pattern);require(value,"generated audio root");root=value;}
  ~Tree(){std::error_code error;std::filesystem::remove_all(root,error);}
  void put(const char* leaf,const char* bytes){const auto path=root/leaf;std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path,std::ios::binary);out<<bytes;out.close();require(bool(out),"complete generated audio file");}
};
struct QueueAudio final:AbsentAudioFixture {
  bool failLimit=false;
  UnsignedInt getNum2DSamples()const override{return 0;}
  UnsignedInt getNum3DSamples()const override{return 0;}
  Bool doesViolateLimit(AudioEventRTS*)const override{if(failLimit)throw ERROR_BAD_ARG;return FALSE;}
  Bool isPlayingLowerPriority(AudioEventRTS*)const override{return FALSE;}
  Bool isPlayingAlready(AudioEventRTS*)const override{return FALSE;}
  Bool isObjectPlayingVoice(UnsignedInt)const override{return FALSE;}
  void setPreferredProvider(AsciiString value)override{m_audioSettings->m_preferred3DProvider[MAX_HW_PROVIDERS]=std::move(value);}
  void setPreferredSpeaker(AsciiString value)override{m_audioSettings->m_defaultSpeakerType2D=translateSpeakerTypeToUnsignedInt(value);}
  std::size_t pending()const{return m_audioRequests.size();}
  AudioHandle cursor()const{return theAudioHandlePool;}
  AudioRequest* first(){return m_audioRequests.empty()?nullptr:m_audioRequests.front();}
  void drain(){removeAllAudioRequests();}
  MusicManager& music(){return *m_music;}
  SoundManager& sound(){return *m_sound;}
};
struct InspectEvent:AudioEventRTS {
  using AudioEventRTS::AudioEventRTS;
  const AudioEventInfo* rawInfo()const{return m_eventInfo;}
};
struct Context {
  Tree assets,user;FileSystem files;
  NativeUserStorage storage;
  Version version;
  FileSystem* previousFiles=TheFileSystem;
  NativeUserStorage* previousStorage=TheNativeUserStorage;
  Version* previousVersion=TheVersion;
  AudioManager* previousAudio=TheAudio;
  GlobalData* previousData=TheWritableGlobalData;
  std::unique_ptr<GlobalData> data;
  std::unique_ptr<QueueAudio> audio;
  Context():storage(NativeUserPaths::resolve(user.root.string(),(user.root/"data").string(),(user.root/"cache").string()),files){
    assets.put("Data/Scripts/SkirmishScripts.scb","generated");
    assets.put("Data/Scripts/MultiplayerScripts.scb","generated");
    assets.put("game.ini","GameData\n XResolution = 640\n YResolution = 480\n End\n");
    assets.put("Data/INI/AudioSettings.ini","AudioSettings\n AudioRoot = Data/Audio\n MusicFolder = Tracks\n SoundsFolder = Sounds\n StreamingFolder = Speech\n SoundsExtension = wav\n"
      " Preferred3DSW = GeneratedNoDevice\n Default2DSpeakerType = Stereo\n Default3DSpeakerType = Stereo\n Relative2DVolume = 0%\n MinSampleVolume = 1%\n"
      " DefaultSoundVolume = 100%\n Default3DSoundVolume = 100%\n DefaultSpeechVolume = 100%\n DefaultMusicVolume = 100%\n End\n");
    const char* music="MusicTrack GeneratedMusic\n Filename = generated-track\n Volume = 100%\n VolumeShift = 0%\n PitchShift = -5 5\n Delay = 0 10\n LoopCount = 1\n Priority = NORMAL\n Type = UI EVERYONE\n End\n";
    const char* sounds="AudioEvent GeneratedSound\n Sounds = generated-a generated-b generated-c\n Attack = generated-attack-a generated-attack-b\n Decay = generated-decay-a generated-decay-b\n"
      " Volume = 100%\n VolumeShift = -10%\n PitchShift = -5 5\n Delay = 0 10\n LoopCount = 1\n Priority = NORMAL\n Type = UI EVERYONE\n Control = RANDOM\n End\n"
      "AudioEvent GeneratedInterrupt\n Sounds = generated-a generated-b generated-c\n Volume = 100%\n VolumeShift = -10%\n PitchShift = -5 5\n Delay = 0 10\n LoopCount = 1\n Priority = NORMAL\n Type = UI EVERYONE\n Control = RANDOM INTERRUPT\n End\n";
    assets.put("Data/INI/Music.ini",music);assets.put("Data/INI/SoundEffects.ini",sounds);
    for(const char* name:{"Data/INI/Default/Music.ini","Data/INI/Default/SoundEffects.ini","Data/INI/Default/Speech.ini","Data/INI/Default/Voice.ini","Data/INI/Speech.ini","Data/INI/Voice.ini","Data/INI/MiscAudio.ini"})assets.put(name,"");
    // Startup only checks existence. This generated sentinel is not decoded audio.
    assets.put("Data/Audio/Tracks/generated-track","generated existence sentinel");
    assets.put("Data/Audio/Tracks/English/generated-track","generated localized sentinel");
    try{
      files.mountReadOnly({assets.root.string()});TheFileSystem=&files;TheNativeUserStorage=&storage;TheVersion=&version;
      INI ini;const INIBlockDefinition blocks[]{{"GameData",GlobalData::parseGameDataDefinition}};
      ini.loadBlocks("game.ini",INI_LOAD_OVERWRITE,blocks);data.reset(TheWritableGlobalData);data->setName("generated audio defaults");
      audio=std::make_unique<QueueAudio>();TheAudio=audio.get();audio->init();
    }catch(...){retire();throw;}
  }
  void retire(){if(audio){TheAudio=audio.get();audio.reset();}TheAudio=previousAudio;
    data.reset();TheWritableGlobalData=previousData;TheVersion=previousVersion;TheNativeUserStorage=previousStorage;TheFileSystem=previousFiles;}
  ~Context(){retire();}
};
struct Events {
  AsciiString musicName{"GeneratedMusic"},soundName{"GeneratedSound"},interruptName{"GeneratedInterrupt"};
  InspectEvent music{musicName},sound{soundName},interrupt{interruptName};
  explicit Events(bool logical=false){for(auto* value:{&music,&sound,&interrupt}){value->setUninterruptable(TRUE);value->setIsLogicalAudio(logical);}}
};
Int audioPeek(){GameAudioRandomTransaction guard;return GameAudioRandomValue(0,1000000);}
Int requestUnits(){return TheMemoryPoolFactory->findMemoryPool("AudioRequest")->getUsedBlockCount();}
void operation(QueueAudio& audio,Events& events){
  require(audio.addAudioEvent(&events.music)>=AHSV_FirstHandle,"original music queue admission");
  require(audio.addAudioEvent(&events.interrupt)>=AHSV_FirstHandle,"original interrupt queue admission, not playback");
  require(audio.addAudioEvent(&events.sound)==AHSV_NoSound,"zero physical channels reject sound without dangling result");
  audio.removeAudioEvent(AHSV_StopTheMusic);audio.removeAudioEvent(AHSV_FirstHandle);
}
void functional(Context& context){auto& audio=*context.audio;Events events;
  const auto start=audio.cursor();const auto rng=audioPeek();const auto logic=GetGameLogicRandomSeedCRC();
  const auto live=AllocationFault::live();const auto units=requestUnits();
  {NativeAudioAdmission outer(audio);operation(audio,events);
    require(audio.pending()==4 && audio.cursor()==start+3,"queue cardinality and consumed policy-rejection handle");
    require(events.music.rawInfo() && events.sound.getPlayingAudioIndex()>=0,"original caller caches published");
    require(audioPeek()!=rng,"source audio RNG advanced before outer rollback");}
  require(audio.pending()==0 && audio.cursor()==start && audioPeek()==rng && GetGameLogicRandomSeedCRC()==logic,"outer rollback restores queue/handle/both RNG streams");
  require(!events.music.rawInfo()&&!events.sound.rawInfo()&&events.sound.getPlayingAudioIndex()==-1,"outer restores descendant caller snapshots");
  require(AllocationFault::live()==live&&requestUnits()==units,"all payloads and request nodes/pool units retired after rollback");
  operation(audio,events);require(audio.pending()==4,"standalone admissions retain queue");
  auto* request=audio.first();auto* payload=request->takePendingEvent();require(payload&&request->takePendingEvent()==nullptr,"consumer takes exactly one payload ownership unit");
  require(payload->getFilename()=="Data/Audio\\Tracks\\English\\generated-track","localized music takes exact single-separator path");
  AudioEventRTS probe(events.sound);probe.setAudioEventInfo(audio.findAudioEventInfo(events.soundName));probe.generateFilename();probe.generatePlayInfo();
  require(std::strstr(probe.getFilename().str(),"English")==nullptr&&std::strstr(probe.getAttackFilename().str(),"English")==nullptr&&std::strstr(probe.getDecayFilename().str(),"English")==nullptr,"missing localization retains generic sound/attack/decay paths");
  audio.drain();delete payload;require(audio.pending()==0,"queue and transferred payload separately retire");
  events.music.setAudioEventInfo(nullptr);events.sound.setAudioEventInfo(nullptr);events.interrupt.setAudioEventInfo(nullptr);
  audio.addAudioEvent(&events.music); // Whole-owner teardown must retire an accepted pending payload.
}
void negative(Context& context){auto& audio=*context.audio;Events events;
  bool rejected=false;try{audio.addAudioEvent(nullptr);}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}require(rejected,"null event rejects");
  {const AsciiString emptyName,disabledName("NoSound");AudioEventRTS empty(emptyName),disabled(disabledName);
    require(audio.addAudioEvent(&empty)==AHSV_NoSound&&audio.addAudioEvent(&disabled)==AHSV_NoSound,"original empty/NoSound fast policy");}
  const auto beforePolicy=audio.cursor();audio.setOn(FALSE,AudioAffect_Music);
  require(audio.addAudioEvent(&events.music)==AHSV_NoSound&&audio.cursor()==beforePolicy&&events.music.rawInfo(),"disabled music retains cached metadata without consuming handle");
  audio.setOn(TRUE,AudioAffect_Music);events.music.setAudioEventInfo(nullptr);
  events.sound.setVolume(0);const auto rngBeforeMuted=audioPeek();
  require(audio.addAudioEvent(&events.sound)==AHSV_Muted&&audio.cursor()==beforePolicy+1&&audioPeek()!=rngBeforeMuted,"muted policy retains source handle/RNG and copied caller index without queueing");
  events.sound.setVolume(-1);events.sound.setAudioEventInfo(nullptr);events.sound.setPlayingAudioIndex(-1);
  const auto start=audio.cursor();
  {NativeAudioAdmission outer(audio);audio.addAudioEvent(&events.music);
    for(unsigned path=0;path<4;++path){rejected=false;try{if(path==0)audio.drain();else if(path==1)audio.first()->takePendingEvent();else if(path==2)audio.reset();else audio.init();}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}
      require(rejected&&audio.pending()==1,"active ancestor forbids queue consumption/transfer and parent replacement");}
    audio.failLimit=true;rejected=false;try{audio.addAudioEvent(&events.sound);}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}audio.failLimit=false;
    require(rejected&&audio.pending()==1&&audio.cursor()==start+1&&!events.sound.rawInfo(),"failed child preserves accepted sibling and caller boundary");
    require(audio.addAudioEvent(&events.sound)==AHSV_NoSound,"same-owner corrected sound rejection");outer.commit();}
  require(audio.pending()==1&&audio.cursor()==start+2,"outer commit preserves successful child and policy rejection");audio.drain();
  events.sound.setAudioEventInfo(audio.findAudioEventInfo(events.musicName));
  const auto* stale=events.sound.rawInfo();
  {NativeAudioAdmission outer(audio);audio.addAudioEvent(&events.sound);}
  require(events.sound.rawInfo()==stale,"raw stale-but-valid borrowed cache restored without lazy getter mutation");events.sound.setAudioEventInfo(nullptr);
  bool wrongThread=false;std::thread worker([&]{try{NativeAudioAdmission guard(audio);}catch(ErrorCode code){wrongThread=code==ERROR_BAD_ARG;}});worker.join();require(wrongThread,"wrong owner thread rejects before global/state access");
}
void faults(Context& context,bool logical){auto& audio=*context.audio;
  // Discovery and each injection enter the same source cursor/RNG/caller state.
  std::size_t census=0;{Events events(logical);NativeAudioAdmission outer(audio);AllocationFault::arm(SIZE_MAX);
    try{operation(audio,events);}catch(...){AllocationFault::disarm();throw;}census=AllocationFault::attempts();AllocationFault::disarm();}
  require(census>0&&census<256,"bounded complete audio admission census");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){Events events(logical);const auto live=AllocationFault::live();const auto units=requestUnits();const auto start=audio.cursor();const auto rng=audioPeek();const auto logic=GetGameLogicRandomSeedCRC();bool failed=false;
    {NativeAudioAdmission outer(audio);AllocationFault::arm(ordinal);try{operation(audio,events);}catch(const std::bad_alloc&){failed=true;}
      catch(...){AllocationFault::disarm();throw;}AllocationFault::disarm();
      require(failed==(ordinal<census)&&failed==AllocationFault::triggered()&&(failed||AllocationFault::attempts()==census),"every failure/retry pair and exact audio terminal");}
    require(audio.pending()==0&&audio.cursor()==start&&audioPeek()==rng&&GetGameLogicRandomSeedCRC()==logic,"each failed or accepted outer rollback restores exact admission state");
    require(!events.music.rawInfo()&&!events.sound.rawInfo()&&!events.interrupt.rawInfo()&&events.sound.getPlayingAudioIndex()==-1,"complete caller ancestor restoration");
    require(AllocationFault::live()==live&&requestUnits()==units,"each prefix retires standard and pooled ownership");
    if(failed){NativeAudioAdmission retry(audio);operation(audio,events);}
    require(AllocationFault::live()==live&&requestUnits()==units&&audio.pending()==0&&audio.cursor()==start,"each same-owner corrected retry retires exactly");
  }
  std::cout<<"audio "<<(logical?"logical":"ordinary")<<" prefixes [0,"<<census<<"); terminal "<<census<<"; every retry\n";
}
void poolFailures(Context& context){auto& audio=*context.audio;auto* pool=TheMemoryPoolFactory->findMemoryPool("AudioRequest");
  std::vector<void*> held;held.reserve(pool->getFreeBlockCount());
  struct Retire {MemoryPool* pool;std::vector<void*>& held;~Retire(){for(auto* value:held)pool->freeBlock(value);}} retire{pool,held};
  while(pool->getFreeBlockCount())held.push_back(pool->allocateBlock("generated audio request exhaustion"));
  Events events;const auto start=audio.cursor();const auto units=requestUnits();const auto live=AllocationFault::live();const auto rng=audioPeek();const auto logic=GetGameLogicRandomSeedCRC();
  for(unsigned path=0;path<4;++path){bool rejected=false;std::unique_ptr<AudioEventRTS> direct;
    if(path==2){direct=std::make_unique<AudioEventRTS>(events.music);direct->setAudioEventInfo(audio.findAudioEventInfo(events.musicName));}
    const auto baseline=AllocationFault::live();try{if(path==0)audio.addAudioEvent(&events.music);else if(path==1)audio.addAudioEvent(&events.interrupt);else if(path==2)audio.music().playTrack(direct.get());else audio.removeAudioEvent(AHSV_FirstHandle);}
    catch(ErrorCode code){rejected=code==ERROR_OUT_OF_MEMORY;}
    require(rejected&&audio.pending()==0&&audio.cursor()==start&&requestUnits()==units&&AllocationFault::live()==baseline&&audioPeek()==rng&&GetGameLogicRandomSeedCRC()==logic,"all pooled request callers reject without leaked payload/cursor/RNG");}
  require(AllocationFault::live()==live,"direct caller retains and retires its failed candidate");
  for(auto* value:held)pool->freeBlock(value);held.clear();
  {NativeAudioAdmission retry(audio);operation(audio,events);}
  require(audio.pending()==0&&requestUnits()==0&&AllocationFault::live()==live,"same owner pooled corrected retry retires exactly");
  std::cout<<"audio pooled exhaustion: all callers and corrected retry\n";
}
void startupFaults(Context& context){
  auto create=[&]{
    struct Lease {AudioManager* previous=TheAudio;std::unique_ptr<QueueAudio> owner;
      ~Lease(){if(owner){TheAudio=owner.get();owner.reset();}TheAudio=previous;}} lease;
    lease.owner=std::make_unique<QueueAudio>();TheAudio=lease.owner.get();lease.owner->init();
    require(lease.owner->pending()==0&&lease.owner->cursor()==AHSV_FirstHandle,"actual source startup initialized queue and cursor");
  };
  create();AllocationFault::arm(SIZE_MAX);try{create();}catch(...){AllocationFault::disarm();throw;}
  const auto census=AllocationFault::attempts();AllocationFault::disarm();require(census>0&&census<1024,"bounded complete audio startup census");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){const auto live=AllocationFault::live();const auto units=requestUnits();
    const auto infos=TheMemoryPoolFactory->findMemoryPool("AudioEventInfo")->getUsedBlockCount();bool failed=false;
    AllocationFault::arm(ordinal);try{create();}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
    AllocationFault::disarm();require(failed==(ordinal<census)&&failed==AllocationFault::triggered()&&(failed||AllocationFault::attempts()==census),"every source startup failure and exact terminal");
    require(TheAudio==context.audio.get()&&AllocationFault::live()==live&&requestUnits()==units&&TheMemoryPoolFactory->findMemoryPool("AudioEventInfo")->getUsedBlockCount()==infos,"every partial startup retires paired managers/metadata/pools and restores borrowed publication");
    if(failed)create();require(AllocationFault::live()==live&&TheAudio==context.audio.get(),"each startup corrected retry retires exactly");
  }
  std::cout<<"audio startup prefixes [0,"<<census<<"); terminal "<<census<<"; every retry\n";
}
void lifetimes(Context& context){auto& audio=*context.audio;Events events;
  const auto live=AllocationFault::live();const auto start=audio.cursor();const auto units=requestUnits();
  {NativeAudioAdmission outer(audio);
    {NativeAudioAdmission child(audio);
      {AudioEventRTS temporary(events.interrupt);audio.addAudioEvent(&temporary);}
      child.commit();}
    require(audio.pending()==1,"expired stack caller leaves its accepted owned payload alive until outer rollback");}
  require(audio.pending()==0&&audio.cursor()==start&&requestUnits()==units&&AllocationFault::live()==live,"expired caller cannot be restored; owned payload still retires exactly");
  alignas(InspectEvent) unsigned char backing[sizeof(InspectEvent)];InspectEvent* occupant=nullptr;
  struct Retire {InspectEvent*& value;~Retire(){if(value)value->~InspectEvent();}} retire{occupant};
  const auto* borrowed=audio.findAudioEventInfo(events.musicName);
  {NativeAudioAdmission outer(audio);
    occupant=new(backing) InspectEvent(events.musicName);occupant->setUninterruptable(TRUE);audio.addAudioEvent(occupant);
    AllocationFault::arm(0);occupant->~InspectEvent();occupant=nullptr;
    const auto attempts=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();
    require(!hit&&attempts==0,"retirement withdraws all ancestor snapshots without allocation");
    occupant=new(backing) InspectEvent(events.soundName);occupant->setUninterruptable(TRUE);occupant->setPlayingAudioIndex(9);occupant->setAudioEventInfo(borrowed);
    require(audio.addAudioEvent(occupant)==AHSV_NoSound,"new lifetime at reused address follows its own original policy");}
  require(occupant->rawInfo()==borrowed&&occupant->getPlayingAudioIndex()==9,"address reuse restores new lifetime boundary, never expired caller's cache");
  occupant->~InspectEvent();occupant=nullptr;
  const AudioEventRTS immutable(events.interrupt);
  {NativeAudioAdmission outer(audio);require(audio.addAudioEvent(&immutable)>=AHSV_FirstHandle,"actually const caller accepts through mutable source caches");}
  require(!immutable.getAudioEventInfo()&&immutable.getPlayingAudioIndex()==-1,"actually const caller cache restores without const-cast mutation");
  require(audio.pending()==0&&audio.cursor()==start&&requestUnits()==units&&AllocationFault::live()==live,"all caller lifetime variants retire exact queue and ownership state");
}
}
int main(int argc,char** argv){bool initialized=false;const auto initial=AllocationFault::live();try{
  require(argc==2,"audio family required");initMemoryManager();initialized=true;
  TheMemoryPoolFactory->createMemoryPool("AudioRequest",sizeof(AudioRequest),32,0);
  TheMemoryPoolFactory->createMemoryPool("AudioEventInfo",sizeof(AudioEventInfo),32,0);
  const std::string_view family(argv[1]);auto run=[&](Context& context){
    if(family=="functional")functional(context);else if(family=="negative")negative(context);else if(family=="faults")faults(context,false);else if(family=="logical-faults")faults(context,true);else if(family=="pool-failures")poolFailures(context);else if(family=="startup-faults")startupFaults(context);else if(family=="lifetimes")lifetimes(context);else throw std::runtime_error("unknown audio family");};
  // Warm the identical complete root and selected family, not unrelated providers.
  {Context warm;run(warm);}
  for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();
    {Context context;run(context);}
    require(AllocationFault::live()==live,"whole parsed audio root retires exactly");}
  shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole memory owner retires exactly");
  std::cout<<"PASS original audio admission/queue; physical playback untested\n";return 0;
}catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode){std::cerr<<"FAIL original audio boundary\n";}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
