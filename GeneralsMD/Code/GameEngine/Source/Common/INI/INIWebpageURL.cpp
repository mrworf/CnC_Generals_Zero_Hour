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

// FILE: INIWebpageURL.cpp /////////////////////////////////////////////////////////////////////////////
// Author: Bryan Cleveland, November 2001
// Desc:   Parsing Webpage URL INI entries
///////////////////////////////////////////////////////////////////////////////////////////////////

// The authored metadata block remains recognized. The embedded ATL browser and
// its CWD-dependent file URL activation are outside the native product.
#include "PreRTS.h"
#include "Common/INI.h"
#include <cstddef>

void INI::parseWebpageURLDefinition(INI* ini)
{
    const char* name=ini->getNextToken();
    if (!name || !*name) throw ERROR_BAD_INI;
    AsciiString tag(name);
    struct Definition { AsciiString url; } candidate;
    const FieldParse fields[] {
        {"URL",INI::parseAsciiString,nullptr,offsetof(Definition,url)},
        {nullptr,nullptr,nullptr,0}
    };
    ini->initFromINI(&candidate,fields);
    // Complete normal source field validation, with no browser/global mutation,
    // process launch, filesystem write, or fabricated Internet provider.
}
