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

// FILE: SubsystemInterface.cpp
// ----------------------------------------------------------------------------

// SPDX-License-Identifier: GPL-3.0-or-later
// Original base lifecycle split from gameplay INI initialization.
#include "Common/SubsystemInterface.h"
SubsystemInterfaceList* TheSubsystemList=nullptr;
SubsystemInterface::SubsystemInterface()
#ifdef DUMP_PERF_STATS
:m_curDrawTime(0),
m_startDrawTimeConsumed(0),
m_startTimeConsumed(0),
m_curUpdateTime(0),
m_dumpUpdate(false),
m_dumpDraw(false)
#endif
{
	if (TheSubsystemList) {
		TheSubsystemList->addSubsystem(this);
	}
}


SubsystemInterface::~SubsystemInterface()
{
	if (TheSubsystemList) {
		TheSubsystemList->removeSubsystem(this);
	}
}

void SubsystemInterfaceList::addSubsystem(SubsystemInterface* sys)
{
	(void)sys;
#ifdef DUMP_PERF_STATS
	m_allSubsystems.push_back(sys);
#endif
}
//-----------------------------------------------------------------------------
void SubsystemInterfaceList::removeSubsystem(SubsystemInterface* sys)
{
	(void)sys;
#ifdef DUMP_PERF_STATS
	for (SubsystemList::iterator it = m_allSubsystems.begin(); it != m_allSubsystems.end(); ++it)
	{
		if ( (*it) == sys) {
			m_allSubsystems.erase(it);
			break;
		}
	}
#endif
}
//-----------------------------------------------------------------------------
