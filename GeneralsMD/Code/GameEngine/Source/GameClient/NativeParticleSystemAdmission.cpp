// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeParticleSystemAdmission.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/GameClient.h"
#include "Common/GlobalData.h"
#include <vector>

struct NativeParticleSystemAdmission::Impl {
  struct Cache {const ParticleSystemTemplate* source;const ParticleSystemTemplate* prior;};
  ParticleSystemManager& owner;
  GameClient* const client=TheGameClient;
  const GlobalData* const configuration=TheGlobalData;
  const ParticleSystemID next;
  const std::size_t count;
  GameClientRandomTransaction random;
  std::vector<Cache> caches;
  Impl* previous;
  bool committed=false;
  Impl(ParticleSystemManager& value,Impl* prior):owner(value),next(value.m_uniqueSystemID),count(value.m_allParticleSystemList.size()),previous(prior){}
  void remember(const ParticleSystemTemplate& source){
    for(const auto& entry:caches)if(entry.source==&source)return;
    caches.push_back({&source,source.m_slaveTemplate});
  }
  ~Impl()noexcept{
    if(!committed){
      // Descendants precede ancestors in the original list. Retire the newest
      // ancestor first, keeping each destructor's remaining links alive.
      while(owner.m_allParticleSystemList.size()>count)owner.m_allParticleSystemList.back()->deleteInstance();
      owner.m_uniqueSystemID=next;
      for(auto it=caches.rbegin();it!=caches.rend();++it)it->source->m_slaveTemplate=it->prior;
    }
  }
};
thread_local NativeParticleSystemAdmission::Impl* NativeParticleSystemAdmission::current=nullptr;
NativeParticleSystemAdmission::NativeParticleSystemAdmission(ParticleSystemManager& owner){
  // Worker rejection precedes mutable registry/global identity inspection.
  if(owner.m_creationThread!=std::this_thread::get_id())throw ERROR_BAD_ARG;
  if(TheParticleSystemManager!=&owner||!TheGameClient)throw ERROR_BAD_ARG;
  if(current&&(&current->owner!=&owner||current->client!=TheGameClient||current->configuration!=TheGlobalData))throw ERROR_BAD_ARG;
  if(owner.m_allParticleSystemList.size()!=owner.m_particleSystemCount)throw ERROR_BAD_ARG;
  m_state=std::make_unique<Impl>(owner,current);current=m_state.get();
}
NativeParticleSystemAdmission::~NativeParticleSystemAdmission()noexcept{
  auto* previous=m_state->previous;m_state.reset();current=previous;
}
void NativeParticleSystemAdmission::commit()noexcept{m_state->random.commit();m_state->committed=true;}
const ParticleSystemTemplate* NativeParticleSystemAdmission::slave(const ParticleSystemTemplate& source){
  if(!current)throw ERROR_BAD_ARG;
  if(source.m_slaveTemplate||source.m_slaveSystemName.isEmpty())return source.m_slaveTemplate;
  for(auto* scope=current;scope;scope=scope->previous)scope->remember(source);
  source.m_slaveTemplate=current->owner.findTemplate(source.m_slaveSystemName);
  return source.m_slaveTemplate;
}
