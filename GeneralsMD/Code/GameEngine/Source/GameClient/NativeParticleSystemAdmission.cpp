// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeParticleSystemAdmission.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/GameClient.h"
#include "Common/GlobalData.h"
#include <vector>

struct NativeParticleSystemAdmission::Impl {
  struct Cache {const ParticleSystemTemplate* source;const ParticleSystemTemplate* prior;};
  struct Client {ParticleSystem* source;ParticleSystem::NativeClientState prior;};
  ParticleSystemManager& owner;
  GameClient* const client=TheGameClient;
  const GlobalData* const configuration=TheGlobalData;
  const ParticleSystemID next;
  const std::size_t count;
  GameClientRandomTransaction random;
  std::vector<Cache> caches;
  std::vector<Client> clients;
  Impl* previous;
  bool committed=false;
  Impl(ParticleSystemManager& value,Impl* prior):owner(value),next(value.m_uniqueSystemID),count(value.m_allParticleSystemList.size()),previous(prior){}
  void remember(const ParticleSystemTemplate& source){
    for(const auto& entry:caches)if(entry.source==&source)return;
    caches.push_back({&source,source.m_slaveTemplate});
  }
  void remember(ParticleSystem& source){
    for(const auto& entry:clients)if(entry.source==&source)return;
    auto it=owner.m_allParticleSystemList.begin();
    for(std::size_t index=0;index<count;++index,++it)if(*it==&source){clients.push_back({&source,source.nativeClientState()});return;}
  }
  ~Impl()noexcept{
    if(!committed){
      // Descendants precede ancestors in the original list. Retire the newest
      // ancestor first, keeping each destructor's remaining links alive.
      while(owner.m_allParticleSystemList.size()>count)owner.m_allParticleSystemList.back()->deleteInstance();
      owner.m_uniqueSystemID=next;
      for(auto it=clients.rbegin();it!=clients.rend();++it)it->source->restoreNativeClientState(it->prior);
      for(auto it=caches.rbegin();it!=caches.rend();++it)it->source->m_slaveTemplate=it->prior;
    }
  }
};
// Only systems present at a scope's entry need mutation rollback. New systems
// are retired as a whole before any retained accepted state is restored.
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
void NativeParticleSystemAdmission::touch(ParticleSystem& source){
  if(!current)return;
  if(TheParticleSystemManager!=&current->owner||TheGameClient!=current->client||TheGlobalData!=current->configuration)throw ERROR_BAD_ARG;
  for(auto* scope=current;scope;scope=scope->previous)scope->remember(source);
}
