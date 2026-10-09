// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeDisplayStringManager.h"
#include "GameClient/GlobalLanguage.h"
#include "GameClient/DrawGroupInfo.h"
#include "GameClient/GameText.h"
#include <memory>

namespace { void valid(bool condition) { if(!condition)throw ERROR_BAD_ARG; } }
NativeFontLibrary& NativeDisplayStringManager::fonts() const {
  auto* owner=dynamic_cast<NativeFontLibrary*>(TheFontLibrary);
  valid(owner && (!m_fonts || m_fonts==owner) && owner->initialized());return *owner;
}
NativeDisplayStringManager::~NativeDisplayStringManager() { reset(); }
void NativeDisplayStringManager::reset() {
  m_groups.fill(nullptr);m_formation=nullptr;m_currentCheckpoint=nullptr;
  while(m_stringList) { auto* text=m_stringList;unLink(text);text->deleteInstance(); }
}
std::size_t NativeDisplayStringManager::count() const noexcept {
  std::size_t result=0;for(auto* text=m_stringList;text;text=text->next())++result;return result;
}
DisplayString* NativeDisplayStringManager::newDisplayString() {
  auto& library=fonts();
  auto retire=[](NativeDisplayString* string) { string->deleteInstance(); };
  std::unique_ptr<NativeDisplayString,decltype(retire)> candidate(newInstance(NativeDisplayString)(library),retire);
  GameFont* font;
  if(TheGlobalLanguageData && TheGlobalLanguageData->m_defaultDisplayStringFont.name.isNotEmpty()) {
    const auto& descriptor=TheGlobalLanguageData->m_defaultDisplayStringFont;
    font=library.getFont(descriptor.name,descriptor.size,descriptor.bold);
  }else font=library.getFont("Times New Roman",12,FALSE);
  valid(font);candidate->setFont(font);link(candidate.get());return candidate.release();
}
void NativeDisplayStringManager::freeDisplayString(DisplayString* string) {
  if(!string)return;
  auto* owned=m_stringList;while(owned && owned!=string)owned=owned->next();valid(owned);
  for(auto& cache:m_groups)if(cache==owned)cache=nullptr;
  if(m_formation==owned)m_formation=nullptr;
  if(m_currentCheckpoint==owned)m_currentCheckpoint=nullptr;
  unLink(owned);owned->deleteInstance();
}
DisplayString* NativeDisplayStringManager::getGroupNumeralString(Int numeral) {
  valid(numeral>=0 && numeral<groupCount);return m_groups[numeral];
}
void NativeDisplayStringManager::postProcessLoad() {
  auto& library=fonts();valid(TheDrawGroupInfo && TheGameText);
  auto* font=library.getFont(TheDrawGroupInfo->m_fontName,TheDrawGroupInfo->m_fontSize,TheDrawGroupInfo->m_fontIsBold);valid(font);
  std::array<DisplayString*,groupCount> candidate{};DisplayString* formation=nullptr;
  struct Retire {
    NativeDisplayStringManager& manager;std::array<DisplayString*,groupCount>& groups;DisplayString*& formation;
    ~Retire() { for(auto* text:groups)if(text)manager.freeDisplayString(text);if(formation)manager.freeDisplayString(formation); }
  } retire{*this,candidate,formation};
  for(Int n=0;n<groupCount;++n) {
    candidate[n]=newDisplayString();candidate[n]->setFont(font);
#ifdef KRIS_BRUTAL_HACK_FOR_AIRCRAFT_CARRIER_DEBUGGING
    UnicodeString value;value.format(L"%d",n);candidate[n]->setText(value);
#else
    AsciiString label;label.format("NUMBER:%d",n);candidate[n]->setText(TheGameText->fetch(label));
#endif
  }
  formation=newDisplayString();formation->setFont(font);formation->setText(TheGameText->fetch("LABEL:FORMATION"));
  candidate.swap(m_groups);std::swap(formation,m_formation);
  // The scope guard now owns the superseded cache; all new pointers are admitted
  // together. Ordinary tracked strings are left intact across cache replacement.
}
