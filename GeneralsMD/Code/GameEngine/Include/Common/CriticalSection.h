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

// CriticalSection.h ///////////////////////////////////////////////////////
// Utility class to use critical sections in areas of code.
// Author: JohnM And MattC, August 13, 2002

#pragma once

#ifndef __CRITICALSECTION_H__
#define __CRITICALSECTION_H__

#include <mutex>
class CriticalSection {
    std::recursive_mutex mutex;
public:
    CriticalSection()=default;
    virtual ~CriticalSection()=default;
    CriticalSection(const CriticalSection&)=delete;
    CriticalSection& operator=(const CriticalSection&)=delete;
    void enter(){mutex.lock();}
    void exit(){mutex.unlock();}
};
class ScopedCriticalSection {
    CriticalSection* owner;
public:
    explicit ScopedCriticalSection(CriticalSection* value):owner(value){if(owner)owner->enter();}
    ~ScopedCriticalSection(){if(owner)owner->exit();}
    ScopedCriticalSection(const ScopedCriticalSection&)=delete;
    ScopedCriticalSection& operator=(const ScopedCriticalSection&)=delete;
};
extern CriticalSection* TheUnicodeStringCriticalSection;
extern CriticalSection* TheDmaCriticalSection;
extern CriticalSection* TheMemoryPoolCriticalSection;
extern CriticalSection* TheDebugLogCriticalSection;
#endif
