// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/NameKeyGenerator.h"
#include <vector>

class ModuleData;
using NativeModuleDataList = std::vector<const ModuleData*>;

// The snapshot borrows the accepted pointer prefix but owns its original vector
// backing. Only newly acquired suffix units are retired on rollback.
class NativeModuleDataTransaction final {
  NativeModuleDataList& m_owner;
  NativeModuleDataList m_previous;
  std::size_t m_start;
  bool m_committed = false;
public:
  explicit NativeModuleDataTransaction(NativeModuleDataList& owner);
  ~NativeModuleDataTransaction() noexcept;
  NativeModuleDataTransaction(const NativeModuleDataTransaction&) = delete;
  NativeModuleDataTransaction& operator=(const NativeModuleDataTransaction&) = delete;
  void commit() noexcept { m_committed = true; }
};

// Raw, defined-width admission before converting any untrusted module bucket to
// the source enum. Successful keys preserve the original '0' + bucket protocol.
void nativeValidateModuleBucket(Int bucket);
NameKeyType nativeModuleNameKey(NameKeyGenerator& owner,
                               const AsciiString& name, Int bucket);
