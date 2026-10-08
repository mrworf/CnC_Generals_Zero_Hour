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

// Trig.cpp
// fast trig functions
// Author: Michael S. Booth, March 1994
// Converted to Generals by Matthew D. Campbell, February 2002

#include "Lib/BaseType.h"
#include "Lib/trig.h"
#include <cmath>
// Preserve the original source's active DEFAULT_TRIG path. The unused
// integer lookup-table alternative and content-generation tool are excluded.
Real Sin(Real value){return std::sin(value);}
Real Cos(Real value){return std::cos(value);}
Real Tan(Real value){return std::tan(value);}
Real ACos(Real value){return std::acos(value);}
Real ASin(Real value){return std::asin(value);}
