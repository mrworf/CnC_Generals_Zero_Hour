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


#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameNetwork/GameMessageParser.h"
#include "Common/Xfer.h"
#include <limits>

//----------------------------------------------------------------------------
GameMessageParser::GameMessageParser() 
{
	m_first = m_last = NULL;
	m_argTypeCount = 0;
}

//----------------------------------------------------------------------------
GameMessageParser::GameMessageParser(GameMessage *msg) 
{
	m_first = m_last = NULL;
	m_argTypeCount = 0;

    if (!msg) throw XFER_INVALID_PARAMETERS;
    try {
	UnsignedByte argCount = msg->getArgumentCount();
	GameMessageArgumentDataType lasttype = ARGUMENTDATATYPE_UNKNOWN;
	Int thisTypeCount = 0;

	for (UnsignedByte i = 0; i < argCount; ++i) {
		GameMessageArgumentDataType type = msg->getArgumentDataType(i);
		if (type != lasttype) {
			if (thisTypeCount > 0) {
				addArgType(lasttype, thisTypeCount);
			}
			lasttype = type;
			thisTypeCount = 0;
		}
		++thisTypeCount;
	}
	if (thisTypeCount > 0) {
		addArgType(lasttype, thisTypeCount);
	}
    } catch (...) { clear(); throw; }

}

//----------------------------------------------------------------------------
GameMessageParser::~GameMessageParser() { clear(); }

void GameMessageParser::clear() noexcept
{
	GameMessageParserArgumentType *temp = NULL;
	while (m_first != NULL) {
		temp = m_first->getNext();
		m_first->deleteInstance();
		m_first = temp;
	}
    m_last=nullptr; m_argTypeCount=0;
}

//----------------------------------------------------------------------------
void GameMessageParser::addArgType(Int type, Int argCount)
{
    constexpr Int maximum=std::numeric_limits<UnsignedByte>::max();
    if (type<ARGUMENTDATATYPE_INTEGER || type>=ARGUMENTDATATYPE_UNKNOWN
        || argCount<=0 || argCount>maximum
        || m_argTypeCount>=maximum) throw XFER_INVALID_PARAMETERS;
    Int total=0,seen=0;
    for (auto* node=m_first;node;node=node->getNext()) {
        if (++seen>m_argTypeCount || node->getArgCount()<=0
            || node->getArgCount()>maximum-total) throw XFER_INVALID_PARAMETERS;
        total+=node->getArgCount();
    }
    if (seen!=m_argTypeCount || argCount>maximum-total) throw XFER_INVALID_PARAMETERS;
    auto* candidate=newInstance(GameMessageParserArgumentType)(type,argCount);
    if (m_last) m_last->setNext(candidate);
    else m_first=candidate;
    m_last=candidate;
    ++m_argTypeCount;
}

//----------------------------------------------------------------------------
GameMessageParserArgumentType::GameMessageParserArgumentType(Int type, Int argCount)
{
    if (type<ARGUMENTDATATYPE_INTEGER || type>=ARGUMENTDATATYPE_UNKNOWN
        || argCount<=0 || argCount>std::numeric_limits<UnsignedByte>::max()) throw XFER_INVALID_PARAMETERS;
	m_next = NULL;
	m_type = static_cast<GameMessageArgumentDataType>(type);
	m_argCount = argCount;
}

//----------------------------------------------------------------------------
GameMessageParserArgumentType::~GameMessageParserArgumentType() 
{
}
