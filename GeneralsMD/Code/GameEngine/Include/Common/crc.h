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

// CRC.h ///////////////////////////////////////////////////////////////
// A class encapsulating CRC calculation
// Author: Matthew D. Campbell, October 2001

#pragma once

#ifndef _CRC_H_
#define _CRC_H_

#include "Lib/BaseType.h"

// Identical byte-wise rotate/add protocol in every compiler/build.
class CRC {
    UnsignedInt crc=0;
public:
    void computeCRC(const void* buffer,Int length) {
        if(!buffer || length<1)return;
        const auto* bytes=static_cast<const UnsignedByte*>(buffer);
        for(Int i=0;i<length;++i) {
            const UnsignedInt carry=crc>>31;
            crc=(crc<<1)+bytes[i]+carry;
        }
    }
    void clear() {crc=0;}
    UnsignedInt get() const {return crc;}
};
#endif // _CRC_H_
