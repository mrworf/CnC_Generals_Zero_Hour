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

// FILE: DiscreteCircle.cpp ////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       EA Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: DiscreteCircle.cpp
//
// Created:   John McDonald, September 2002
//
// Desc:      ???
//
//-----------------------------------------------------------------------------

#include "PreRTS.h"
#include "Common/DiscreteCircle.h"
#include "Common/Errors.h"
#include <limits>

//-------------------------------------------------------------------------------------------------
DiscreteCircle::DiscreteCircle(Int xCenter, Int yCenter, Int radius)
{
	const auto r = static_cast<std::int64_t>(radius);
	const auto low = std::numeric_limits<Int>::min();
	const auto high = std::numeric_limits<Int>::max();
	if (radius < 0 || static_cast<std::int64_t>(xCenter)-r < low ||
		static_cast<std::int64_t>(xCenter)+r > high ||
		static_cast<std::int64_t>(yCenter)-r < low ||
		static_cast<std::int64_t>(yCenter)+r > high)
		throw ERROR_BAD_ARG;
	m_yPos = yCenter;
	m_yPosDoubled = static_cast<std::int64_t>(yCenter)*2;
	m_edges.reserve(static_cast<std::size_t>(radius)*2);

	generateEdgePairs(xCenter, yCenter, radius);
	removeDuplicates();
}

//-------------------------------------------------------------------------------------------------
void DiscreteCircle::drawCircle(ScanlineDrawFunc functionToDrawWith, void *parmToPass)
{
	if (!functionToDrawWith) throw ERROR_BAD_ARG;
	for (VecHorzLine::const_iterator it = m_edges.begin(); it != m_edges.end(); ++it) {
		(functionToDrawWith)(it->xStart, it->xEnd, it->yPos, parmToPass);
		if (it->yPos != m_yPos) {
			(functionToDrawWith)(it->xStart, it->xEnd, static_cast<Int>(m_yPosDoubled - it->yPos), parmToPass);
		}
	}
}

//-------------------------------------------------------------------------------------------------
void DiscreteCircle::generateEdgePairs(Int xCenter, Int yCenter, Int radius)
{
	// Uses Bresenham to generate points.
	std::int64_t x = 0;
	std::int64_t y = radius;
	std::int64_t d = (1 - static_cast<std::int64_t>(radius))*2;

	while (y >= 0) {
		HorzLine hl;
		hl.xStart = static_cast<Int>(xCenter - x);
		hl.xEnd		= static_cast<Int>(xCenter + x);
		hl.yPos		= static_cast<Int>(yCenter + y);
		m_edges.push_back(hl);
		
		if (d + y > 0) {
			--y;
			d -= ((y*2) - 1);
		} 

		if (x > d) {
			++x;
			d += ((x*2) + 1);
		}
	}
}

//-------------------------------------------------------------------------------------------------
void DiscreteCircle::removeDuplicates()
{
	VecHorzLineIt it, nextIt;
	for ( it = m_edges.begin(); it != m_edges.end(); /* empty */) {
		nextIt = it;
		++nextIt;
		if (nextIt == m_edges.end()) {
			break;
		}

		if (it->yPos == nextIt->yPos) {
			it = m_edges.erase(it);
		} else { 
			++it;
		}
	}
}
