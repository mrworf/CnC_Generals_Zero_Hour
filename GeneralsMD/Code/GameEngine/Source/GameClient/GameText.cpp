/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright(C) 2001 - All Rights Reserved                  
//                                                                          
//----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: GameText.cpp
//
// Created:   11/07/01
//
//----------------------------------------------------------------------------

#include "GameClient/GameText.h"
#include "GameClient/CSF.h"
#include "GameClient/LanguageFilter.h"
#include "Common/FileSystem.h"
#include "Common/FileOwner.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <strings.h>

// Native ownership of the original label/text/speech and lookup protocol.
namespace {
struct StringInfo {AsciiString label;UnicodeString text;AsciiString speech;};
struct StringLookUp {const AsciiString* label=nullptr;const StringInfo* info=nullptr;};
int compareLUT(const void* first,const void* second) {
    const auto& a=*static_cast<const StringLookUp*>(first);
    const auto& b=*static_cast<const StringLookUp*>(second);
    return a.label->compareNoCase(*b.label);
}
struct CatalogOwner {
    std::unique_ptr<StringInfo[]> strings;
    std::unique_ptr<StringLookUp[]> lookup;
    Int count=0;
    static CatalogOwner prepare(CSFCatalog catalog,Bool filter) {
        if(catalog.records.size()>std::size_t(INT32_MAX))throw ERROR_BAD_ARG;
        CatalogOwner candidate;candidate.count=Int(catalog.records.size());
        if(!candidate.count)return candidate;
        candidate.strings=std::make_unique<StringInfo[]>(std::size_t(candidate.count));
        candidate.lookup=std::make_unique<StringLookUp[]>(std::size_t(candidate.count));
        for(Int ordinal=0;ordinal<candidate.count;++ordinal) {
            const auto& record=catalog.records[std::size_t(ordinal)];
            auto& row=candidate.strings[ordinal];
            row.label=record.label;row.text=record.text;row.speech=record.speech;
            if(filter && TheLanguageFilter)TheLanguageFilter->filterLine(row.text);
            candidate.lookup[ordinal]={&row.label,&row};
        }
        std::qsort(candidate.lookup.get(),std::size_t(candidate.count),sizeof(StringLookUp),compareLUT);
        return candidate;
    }
    const StringInfo* find(const AsciiString& label) const {
        if(!count)return nullptr;
        const StringLookUp key{&label,nullptr};
        const auto* entry=static_cast<const StringLookUp*>(std::bsearch(
            &key,lookup.get(),std::size_t(count),sizeof(StringLookUp),compareLUT));
        return entry?entry->info:nullptr;
    }
};
struct NoString {UnicodeString text;std::unique_ptr<NoString> next;};
class GameTextManager final : public GameTextInterface {
    CatalogOwner m_base,m_map;
    std::unique_ptr<NoString> m_missing;
    AsciiStringVec m_prefix;
    Bool m_initialized=FALSE;
    UnicodeString m_failed{L"***FATAL*** String Manager failed to initilaize properly"};
public:
    ~GameTextManager() override {
        // Iterative teardown: a large missing-label cache must not recurse.
        while(m_missing){auto retired=std::move(m_missing);m_missing=std::move(retired->next);}
    }
    void init() override;
    void reset() override {m_map=CatalogOwner{};}
    void update() override {}
    Bool loadCSF(const AsciiString& filename) override;
    void initMapStringFile(const AsciiString& filename) override;
    UnicodeString fetchMapMetadataLabel(const AsciiString& filename,const AsciiString& label) override;
    UnicodeString fetch(const Char* label,Bool* exists=nullptr) override;
    UnicodeString fetch(AsciiString label,Bool* exists=nullptr) override{return fetch(label.str(),exists);}
    AsciiStringVec& getStringsWithLabelPrefix(AsciiString label) override;
};
}
extern const Char* g_strFile;
extern const Char* g_csfFile;
Bool g_useStringFile=TRUE;
GameTextInterface* TheGameText=nullptr;
GameTextInterface* CreateGameTextInterface(){return NEW GameTextManager;}
void GameTextManager::init() {
    if(m_initialized)return;
    if(!TheFileSystem)throw ERROR_BAD_ARG;
    if(g_useStringFile && TheFileSystem->doesFileExist(g_strFile)) {
        FileCloseOwner input(TheFileSystem->openFile(g_strFile,File::READ|File::TEXT));
        if(!input)throw ERROR_BAD_ARG;
        auto candidate=CatalogOwner::prepare(decodeOriginalStringFile(*input),FALSE);
        m_base=std::move(candidate);m_initialized=TRUE;return;
    }
    AsciiString name;name.format(g_csfFile,getConfiguredGameTextLanguage().str());
    (void)loadCSF(name);
}
Bool GameTextManager::loadCSF(const AsciiString& filename) {
    if(!TheFileSystem)throw ERROR_BAD_ARG;
    FileCloseOwner input(TheFileSystem->openFile(filename.str(),File::READ|File::BINARY));
    if(!input)return FALSE;
    auto candidate=CatalogOwner::prepare(decodeOriginalCSF(*input),FALSE);
    m_base=std::move(candidate);m_initialized=TRUE;return TRUE;
}
void GameTextManager::initMapStringFile(const AsciiString& filename) {
    if(!TheFileSystem)throw ERROR_BAD_ARG;
    FileCloseOwner input(TheFileSystem->openFile(filename.str(),File::READ|File::TEXT));
    if(!input)return;
    auto candidate=CatalogOwner::prepare(decodeOriginalStringFile(*input),TRUE);
    m_map=std::move(candidate);
}
UnicodeString GameTextManager::fetchMapMetadataLabel(const AsciiString& filename,const AsciiString& label) {
    if(!TheFileSystem) throw ERROR_BAD_ARG;
    CatalogOwner temporary;
    if(!filename.isEmpty()) {
        FileCloseOwner input(TheFileSystem->openFile(filename.str(),File::READ|File::TEXT));
        if(input) temporary=CatalogOwner::prepare(decodeOriginalStringFile(*input),TRUE);
    }
    if(!m_initialized || !m_base.count) return m_failed;
    const StringInfo* found=m_base.find(label);
    if(!found) found=temporary.find(label);
    if(found) return found->text;
    UnicodeString missing;missing.format(L"MISSING: '%hs'",label.str());
    return missing;
}
UnicodeString GameTextManager::fetch(const Char* raw,Bool* exists) {
    if(!raw)throw ERROR_BAD_ARG;
    if(exists)*exists=FALSE;
    if(!m_initialized || !m_base.count)return m_failed;
    const AsciiString label(raw);
    const StringInfo* found=m_base.find(label);if(!found)found=m_map.find(label);
    if(found){if(exists)*exists=TRUE;return found->text;}
    UnicodeString missing;missing.format(L"MISSING: '%hs'",raw);
    for(auto* entry=m_missing.get();entry;entry=entry->next.get())if(entry->text==missing)return missing;
    auto candidate=std::make_unique<NoString>();candidate->text=missing;
    candidate->next=std::move(m_missing);m_missing=std::move(candidate);
    return missing;
}
AsciiStringVec& GameTextManager::getStringsWithLabelPrefix(AsciiString label) {
    AsciiStringVec candidate;
    for(const CatalogOwner* owner:{&m_base,&m_map})for(Int ordinal=0;ordinal<owner->count;++ordinal) {
        const auto& name=*owner->lookup[ordinal].label;
        // Original source uses strstr: prefix selection is case-sensitive.
        if(std::strstr(name.str(),label.str())==name.str())candidate.push_back(name);
    }
    m_prefix.swap(candidate);return m_prefix;
}
