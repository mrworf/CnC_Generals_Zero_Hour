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

// FILE: StatsCollector.h /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
//	created:	Jul 2002
//
//	Filename: 	StatsCollector.h
//
//	author:		Chris Huybregts
//	
//	purpose:	Convinience class to help with collecting stats.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __STATSCOLLECTOR_H_
#define __STATSCOLLECTOR_H_

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// FORWARD REFERENCES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/NativeStatsSource.h"
#include <cstdint>
#include <string>
class NativeUserStorage;
class GameMessage;


//-----------------------------------------------------------------------------
// TYPE DEFINES ///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class StatsCollector
{
public:
	StatsCollector( void );
  StatsCollector(const NativeUserStorage& storage,NativeStatsSource& source);
	~StatsCollector( void );
	
	void reset( void );							///< Reset's all values and writes the file header
	
	void collectMsgStats( const GameMessage *msg );			///< collects Msg Stats if
	void collectUnitCountStats( void );									///< cycle through all units and takes count
	void incrementScrollMoveCount( void );
	void incrementBuildCount( void );
	void incrementAttackCount( void );
	void incrementMoveCount( void );
	void startScrollTime( void );		///< Start our logging on the amount of time we're scrolling
	void endScrollTime( void );			///< end our logging on the amount of time we're scrolling

	void update( void );						///< called once a frame to see if we should poll this frame
	
	void writeFileEnd(void);
  bool outputFailed() const noexcept {return m_outputFailed;}
  bool outputReady() const noexcept {return !m_resetPending && !m_state.ended && !m_statsFileName.empty();}
  bool outputDurable() const noexcept {return m_outputDurable;}				///< Write the end of the file
private:
  struct State {
    UnsignedInt build=0,move=0,attack=0,scrollMoves=0;
    UnsignedInt scrollBegin=0,lastUpdate=0,startFrame=0;
    std::uint64_t scrollFrames=0,timeSeconds=0;
    bool scrolling=false,ended=false;
    NativeStatsSample units;
  } m_state;
  const NativeUserStorage* m_storage;
  NativeStatsSource* m_source;
  std::string m_statsFileName;
  bool m_outputFailed=false,m_outputDurable=true;
  bool m_resetPending=true;
  bool publish(const std::string& name,const std::string& bytes,bool append);
  static std::string row(const State& state);

};


//-----------------------------------------------------------------------------
// INLINING ///////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// EXTERNALS //////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
extern StatsCollector* TheStatsCollector;			///< we need a singleton

#endif // __STATSCOLLECTOR_H_
