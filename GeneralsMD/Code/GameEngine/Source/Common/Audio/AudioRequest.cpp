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

#include "Common/AudioRequest.h"
#include "Common/AudioEventRTS.h"
#include "Common/NativeAudioAdmission.h"


AudioRequest::~AudioRequest() 
{
  if(m_ownsPendingEvent)delete m_pendingEvent;
}
AudioEventRTS* AudioRequest::takePendingEvent(){
  if(m_queueOwner)NativeAudioAdmission::requireConsumption(*m_queueOwner);
  if(!m_usePendingEvent)return nullptr;
  if(m_pendingEvent&&!m_ownsPendingEvent)throw ERROR_BAD_ARG;
  auto* value=m_pendingEvent;m_pendingEvent=nullptr;m_ownsPendingEvent=FALSE;return value;
}
