// SPDX-License-Identifier: GPL-3.0-or-later
#include "GameClient/GameText.h"
// Original WinMain's logical patterns, resolved only inside explicit mounts.
const Char* g_strFile="data/Generals.str";
const Char* g_csfFile="data/%s/Generals.csf";
namespace {AsciiString language("English");}
void configureGameTextLanguage(const AsciiString& directory) {
    if(directory.isEmpty())throw ERROR_BAD_ARG;
    for(const char* c=directory.str();*c;++c)
        if(!((*c>='A'&&*c<='Z')||(*c>='a'&&*c<='z')||(*c>='0'&&*c<='9')||*c=='_'||*c=='-'))throw ERROR_BAD_ARG;
    language=directory;
}
AsciiString getConfiguredGameTextLanguage(){return language;}
