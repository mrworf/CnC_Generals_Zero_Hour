// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/Module.h"
#include "Common/NativeModuleData.h"
#include <string>

NativeModuleDataTransaction::NativeModuleDataTransaction(NativeModuleDataList& owner)
  : m_owner(owner), m_previous(owner), m_start(owner.size()) {
  m_owner.swap(m_previous);
}

NativeModuleDataTransaction::~NativeModuleDataTransaction() noexcept {
  if (!m_committed) {
    while (m_owner.size() > m_start) {
      const ModuleData* value = m_owner.back();
      m_owner.pop_back();
      delete value;
    }
    m_owner.swap(m_previous);
  }
}

void nativeValidateModuleBucket(Int bucket) {
  if (bucket < 0 || bucket >= NUM_MODULE_TYPES)
    throw ERROR_BAD_ARG;
}

NameKeyType nativeModuleNameKey(NameKeyGenerator& owner,
                               const AsciiString& name, Int bucket) {
  nativeValidateModuleBucket(bucket);
  if (name.isEmpty()) throw ERROR_BAD_ARG;
  std::string decorated(1, char('0' + bucket));
  decorated.append(name.str());
  return owner.nameToKey(decorated.c_str());
}

ModuleData* Module::friend_newModuleData(INI* ini) {
  std::unique_ptr<ModuleData> data(MSGNEW("Module::friend_newModuleData") ModuleData);
  if(ini) ini->initFromINI(data.get(),nullptr); // Source base accepts only End.
  return data.release();
}
