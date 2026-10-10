// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "Common/NativeAudioAdmission.h"
#include "Common/GameAudio.h"
#include "Common/GameSounds.h"
#include "Common/AudioEventRTS.h"
#include "Common/AudioRandomValue.h"
#include "Common/RandomValue.h"
#include <vector>
#include <array>
#include <algorithm>

struct NativeAudioAdmission::Impl {
  struct Caller {const AudioEventRTS* event;const AudioEventInfo* info;Int index;};
  AudioManager& owner;
  SoundManager* const sound;
  MusicManager* const music;
  const AudioHandle handle;
  const std::size_t count;
  std::array<UnsignedInt,4> samples{};
  GameAudioRandomTransaction random;
  GameLogicRandomTransaction logicRandom;
  std::vector<Caller> callers;
  Impl* parent;
  bool committed=false;
  Impl(AudioManager& value,Impl* prior):owner(value),sound(value.m_sound),music(value.m_music),handle(value.theAudioHandlePool),count(value.m_audioRequests.size()),parent(prior){
    if(sound)samples={sound->m_num2DSamples,sound->m_num3DSamples,sound->m_numPlaying2DSamples,sound->m_numPlaying3DSamples};
  }
  void remember(const AudioEventRTS& event){
    if(parent)parent->remember(event);
    for(const auto& entry:callers)if(entry.event==&event)return;
    callers.push_back({&event,event.m_eventInfo,event.m_playingAudioIndex});
  }
  ~Impl()noexcept{
    if(!committed){
      while(owner.m_audioRequests.size()>count){auto* request=owner.m_audioRequests.back();owner.m_audioRequests.pop_back();owner.releaseAudioRequest(request);}
      owner.theAudioHandlePool=handle;
      if(sound){sound->m_num2DSamples=samples[0];sound->m_num3DSamples=samples[1];sound->m_numPlaying2DSamples=samples[2];sound->m_numPlaying3DSamples=samples[3];}
      for(auto it=callers.rbegin();it!=callers.rend();++it){it->event->setAudioEventInfo(it->info);it->event->setPlayingAudioIndex(it->index);}
    }
  }
};
thread_local NativeAudioAdmission::Impl* NativeAudioAdmission::current=nullptr;
NativeAudioAdmission::NativeAudioAdmission(AudioManager& owner){
  if(owner.m_admissionThread!=std::this_thread::get_id())throw ERROR_BAD_ARG;
  if(TheAudio!=&owner)throw ERROR_BAD_ARG;
  if(current&&(&current->owner!=&owner||current->sound!=owner.m_sound||current->music!=owner.m_music))throw ERROR_BAD_ARG;
  m_state=std::make_unique<Impl>(owner,current);current=m_state.get();
  AudioEventRTS::setNativeRetirementObserver(&NativeAudioAdmission::retireCaller);
}
NativeAudioAdmission::~NativeAudioAdmission()noexcept{
  auto* prior=m_state->parent;m_state.reset();current=prior;
  if(!prior)AudioEventRTS::setNativeRetirementObserver(nullptr);
}
void NativeAudioAdmission::retireCaller(const AudioEventRTS& event)noexcept{
  for(auto* scope=current;scope;scope=scope->parent){auto& callers=scope->callers;
    callers.erase(std::remove_if(callers.begin(),callers.end(),[&](const auto& entry){return entry.event==&event;}),callers.end());}
}
void NativeAudioAdmission::commit()noexcept{m_state->random.commit();m_state->logicRandom.commit();m_state->committed=true;}
void NativeAudioAdmission::touch(const AudioEventRTS& event){if(!current)throw ERROR_BAD_ARG;current->remember(event);}
void NativeAudioAdmission::requireConsumption(AudioManager& owner){
  if(owner.m_admissionThread!=std::this_thread::get_id()||TheAudio!=&owner||current)throw ERROR_BAD_ARG;
}
