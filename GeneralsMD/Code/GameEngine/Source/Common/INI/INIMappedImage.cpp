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

// FILE: INIMappedImage.cpp ///////////////////////////////////////////////////////////////////////
// Author: Colin Day, December 2001
// Desc:   Mapped image INI parsing
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/INI.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/Image.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
/** Parse mapped image entry */
//-------------------------------------------------------------------------------------------------
void INI::parseMappedImageDefinition( INI* ini )
{
    if (!ini || !TheMappedImageCollection || !TheNameKeyGenerator) throw ERROR_BAD_ARG;
    NameKeyTransaction keys(*TheNameKeyGenerator);
    AsciiString name(ini->getNextToken());
    if (name.isEmpty()) throw ERROR_BAD_INI;
    Image* accepted=const_cast<Image*>(TheMappedImageCollection->findImageByName(name));
    if (accepted && accepted->getRawTextureData()) throw ERROR_BAD_INI;
    Image* candidate=newInstance(Image);
    MemoryPoolObjectHolder owner(candidate);
    if (accepted) candidate->copyDefinition(*accepted);
    else candidate->setName(name);
    ini->initFromINI(candidate,candidate->getFieldParse());
    if (accepted) accepted->swapDefinition(*candidate);
    else {
        TheMappedImageCollection->addImage(candidate);
        owner.release();
    }
    keys.commit();
}  // end parseMappedImage
