// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/DrawModule.h"
#include "Common/W3DModelDrawData.h"
#include <thread>
#include <optional>

class GlobalData;
class NativeModelQueryTransaction;

// Original logic-facing draw queries. Presentation/current-animation services
// stay abstract; a fixture cannot establish them by returning synthetic output.
class NativeModelDrawQueries : public DrawModule, public ObjectDrawInterface {
  MEMORY_POOL_GLUE_ABC(NativeModelDrawQueries)
  friend class NativeModelQueryTransaction;
  friend class NativeModelDrawQueryDispatch;
  friend class Drawable;
  struct Cache {
    const ModelConditionInfo* source;
    const GlobalData* configuration;
    Real scale;
    ModelConditionInfo::NativeQuery query;
  };
  NativeW3DAssets& m_assets;
  const W3DModelDrawModuleData& m_data;
  NameKeyGenerator* const m_names;
  const std::thread::id m_thread;
  mutable std::shared_ptr<const Cache> m_cache;
  mutable Bool m_attachValid=FALSE;
  mutable Vector3 m_attachOffset{0,0,0};
  mutable Bool m_lastPristineFallback=FALSE;
  mutable Bool m_lastProjectileOutput=FALSE;
  void requireLogicPhase() const;
  const ModelConditionInfo* prepare(const ModelConditionFlags&) const;
protected:
  const ModelConditionInfo* nativeQueryData()const noexcept{return m_cache?m_cache->query.condition.get():nullptr;}
public:
  NativeModelDrawQueries(Thing*,const ModuleData*,NativeW3DAssets&);
  ObjectDrawInterface* getObjectDrawInterface()override{return this;}
  const ObjectDrawInterface* getObjectDrawInterface()const override{return this;}
  Int getPristineBonePositionsForConditionState(const ModelConditionFlags&,const char*,Int,Coord3D*,Matrix3D*,Int)const override;
  Bool getProjectileLaunchOffset(const ModelConditionFlags&,WeaponSlotType,Int,Matrix3D*,WhichTurretType,Coord3D*,Coord3D*)const override;
};

class NativeModelQueryTransaction final {
  struct Previous {
    const NativeModelDrawQueries* module;
    std::shared_ptr<const NativeModelDrawQueries::Cache> cache;
    Bool attachValid;
    Vector3 offset;
  };
  NameKeyTransaction m_keys;
  NativeW3DAssets::Transaction m_admission;
  std::vector<Previous> m_previous;
  bool m_committed=false;
public:
  NativeW3DAssets& assets;
  static thread_local NativeModelQueryTransaction* current;
  explicit NativeModelQueryTransaction(NativeW3DAssets&);
  ~NativeModelQueryTransaction() noexcept;
  NativeModelQueryTransaction(const NativeModelQueryTransaction&)=delete;
  NativeModelQueryTransaction& operator=(const NativeModelQueryTransaction&)=delete;
  void touch(const NativeModelDrawQueries&);
  void commit() noexcept;
};

// Stack-owned root scope for complete original Drawable module walks, including
// recursive attachment lookups. Non-native draw paths retain their old routing.
class NativeModelDrawQueryDispatch final {
  std::optional<NativeModelQueryTransaction> m_transaction;
  bool m_active=false;
public:
  explicit NativeModelDrawQueryDispatch(const Drawable&);
  bool active()const noexcept{return m_active;}
  void commit()noexcept{if(m_transaction)m_transaction->commit();}
};
