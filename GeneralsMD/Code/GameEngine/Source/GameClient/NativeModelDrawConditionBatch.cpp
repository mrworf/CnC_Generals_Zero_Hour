// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeModelDrawConditionBatch.h"
#include "GameClient/NativeModelDrawAnimationProducer.h"
#include "GameClient/ClientRandomValue.h"
#include "Common/GlobalData.h"
#include <exception>

struct NativeModelDrawConditionBatch::Impl {
  struct Previous {
    NativeModelDrawAnimationProducer* module;
    NativeModelDrawAnimationProducer::Candidate saved;
    bool retained=false;
  };
  struct Publication {
    ModelConditionFlags* flags;
    ModelConditionFlags oldFlags;
    Bool* dirty;
    Bool oldDirty;
  };
  NameKeyTransaction keys;
  NativeW3DAssets::Transaction admission;
  GameClientRandomTransaction random;
  NativeW3DAssets& assets;
  const GlobalData* const configuration=TheGlobalData;
  std::vector<Previous> previous;
  std::vector<Publication> publications;
  bool committed=false;
  explicit Impl(NativeW3DAssets& owner):keys(*TheNameKeyGenerator),admission(owner),assets(owner){}
  Previous* find(const NativeModelDrawAnimationProducer& module)noexcept{
    for(auto& prior:previous)if(prior.module==&module)return &prior;return nullptr;
  }
  void enroll(NativeModelDrawAnimationProducer& module){
    if(find(module))return;
    NativeModelDrawAnimationProducer::Candidate saved;saved.state=module.m_state;
    previous.push_back({&module,std::move(saved),false});
  }
  void enroll(ModelConditionFlags* flags,Bool* dirty){
    if(!flags&&!dirty)return;
    // A forced caller enrolls both fields before a nested dirty getter enrolls
    // that same dirty field. Keep the earliest boundary, not its changed value.
    for(const auto& prior:publications)if((!flags||prior.flags==flags)&&(!dirty||prior.dirty==dirty))return;
    publications.push_back({flags,flags?*flags:ModelConditionFlags{},dirty,dirty?*dirty:FALSE});
  }
  ~Impl()noexcept{
    if(!committed){
      for(auto it=previous.rbegin();it!=previous.rend();++it){auto& module=*it->module;
        if(it->retained){module.m_current=std::move(it->saved.current);module.m_validated=std::move(it->saved.validated);}
        module.m_state=it->saved.state;
      }
      for(auto it=publications.rbegin();it!=publications.rend();++it){
        if(it->flags)*it->flags=it->oldFlags;if(it->dirty)*it->dirty=it->oldDirty;
      }
    }
  }
};
thread_local NativeModelDrawConditionBatch::Impl* NativeModelDrawConditionBatch::current=nullptr;
NativeModelDrawConditionBatch::NativeModelDrawConditionBatch(const DrawModule* const* modules,
    ModelConditionFlags* flags,Bool* dirty){
  if(!modules)throw ERROR_BAD_ARG;
  NativeW3DAssets* assets=nullptr;
  for(auto cursor=modules;*cursor;++cursor){
    const auto* module=dynamic_cast<const NativeModelDrawAnimationProducer*>(*cursor);
    if(!module)continue;
    module->requireClientOwner(); // Thread rejection precedes mutable fields.
    if(!module->m_state.source)continue;
    (void)module->syncTime();
    if(assets&&assets!=&module->nativeAssets())throw ERROR_BAD_ARG;
    assets=&module->nativeAssets();
  }
  if(!assets)return; // Uninitialized/old paths introduce no new allocation.
  if(current){if(&current->assets!=assets||current->configuration!=TheGlobalData)throw ERROR_BAD_ARG;m_root=current;}
  else {m_owned=std::make_unique<Impl>(*assets);m_root=m_owned.get();}
  m_root->enroll(flags,dirty);
  for(auto cursor=modules;*cursor;++cursor){
    auto* module=const_cast<NativeModelDrawAnimationProducer*>(dynamic_cast<const NativeModelDrawAnimationProducer*>(*cursor));
    if(module&&module->m_state.source)m_root->enroll(*module);
  }
  if(m_owned)current=m_root; // Publish only after all fallible enrollment.
}
NativeModelDrawConditionBatch::~NativeModelDrawConditionBatch()noexcept{
  if(m_owned){m_owned.reset();current=nullptr;}
}
void NativeModelDrawConditionBatch::commit()noexcept{
  if(m_owned){m_root->admission.commit();m_root->keys.commit();m_root->random.commit();m_root->committed=true;}
}
void NativeModelDrawConditionBatch::validate(const NativeModelDrawAnimationProducer& module){
  if(!current)return;
  module.requireClientOwner();
  if(!module.m_state.source)return;
  if(current->configuration!=TheGlobalData||&current->assets!=&module.nativeAssets())throw ERROR_BAD_ARG;
  current->enroll(const_cast<NativeModelDrawAnimationProducer&>(module));
}
bool NativeModelDrawConditionBatch::needsCopy(const NativeModelDrawAnimationProducer& module){
  validate(module);if(!current||!module.m_state.source)return false;
  return !current->find(module)->retained;
}
void NativeModelDrawConditionBatch::retainPrevious(NativeModelDrawAnimationProducer& module)noexcept{
  if(!current||!module.m_state.source)return;
  auto* prior=current->find(module);if(!prior)std::terminate(); // validate precedes publication.
  if(!prior->retained){prior->saved.current=std::move(module.m_current);prior->saved.validated=std::move(module.m_validated);prior->retained=true;}
}
