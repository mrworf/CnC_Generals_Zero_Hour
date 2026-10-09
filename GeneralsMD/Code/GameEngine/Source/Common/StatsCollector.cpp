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

// FILE: StatsCollector.cpp /////////////////////////////////////////////////
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
//	Filename: 	StatsCollector.cpp
//
//	author:		Chris Huybregts
//	
//	purpose:	Convinience class to gather player stats
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"
#include "Common/StatsCollector.h"
#include "Common/NativeUserStorage.h"
#include "Common/FileOwner.h"
#include "Common/MessageStream.h"
#include <array>
#include <ctime>
#include <limits>

StatsCollector* TheStatsCollector=nullptr;
namespace {
constexpr UnsignedInt FramesPerSecond=LOGICFRAMES_PER_SECOND;
std::tm calendar(std::time_t value) {
  std::tm result{};
  if(!::localtime_r(&value,&result)) throw ERROR_BAD_ARG;
  return result;
}
std::string reportTime(const std::tm& value) {
  std::array<char,26> bytes{};
  if(!::asctime_r(&value,bytes.data())) throw ERROR_BAD_ARG;
  return bytes.data();
}
void validateText(const AsciiString& text) {
  for(const unsigned char* p=reinterpret_cast<const unsigned char*>(text.str());*p;++p)
    if(*p<32 || *p==127) throw ERROR_BAD_ARG;
}
std::string filenameLeaf(std::string_view input) {
  if(input.empty()) throw ERROR_BAD_ARG;
  constexpr char hex[]="0123456789ABCDEF";
  std::string result;
  for(unsigned char byte:input) {
    if((byte>='a' && byte<='z') || (byte>='A' && byte<='Z') ||
        (byte>='0' && byte<='9') || byte=='_' || byte=='-') result+=char(byte);
    else {result+='%';result+=hex[byte>>4];result+=hex[byte&15];}
  }
  // Retain the original 255-byte filename capacity as an admitted native leaf.
  if(result.size()>251) throw ERROR_BAD_ARG;
  return result+".txt";
}
std::string filename(const NativeStatsIdentity& identity,const std::tm& date,
    const NativeUserStorage& storage) {
  validateText(identity.map);validateText(identity.side);
  std::string directory=identity.directory.str();
  for(char& byte:directory) if(byte=='\\') byte='/';
  while(directory.size()>1 && directory.back()=='/') directory.pop_back();
  if(!directory.empty() && directory.front()=='/') {
    const auto relative=storage.relativeDataPath(directory);
    if(!relative) throw ERROR_BAD_ARG;
    directory=*relative;
  }
  std::string stem=identity.sessionName.str();
  if(stem.empty()) {
    stem=identity.map.str();const auto slash=stem.find_last_of("/\\");
    if(slash!=stem.npos) stem.erase(0,slash+1);
    if(stem.size()<4) throw ERROR_BAD_ARG;
    AsciiString map(stem.c_str());
    if(!map.endsWithNoCase(".map")) throw ERROR_BAD_ARG;
    stem.resize(stem.size()-4);
    if(stem.empty()) throw ERROR_BAD_ARG;
    std::array<char,128> suffix{};
    if(!std::strftime(suffix.data(),suffix.size(),"_%b%d_%I%M%p",&date)) throw ERROR_BAD_ARG;
    stem+=suffix.data();
  }
  const auto leaf=filenameLeaf(stem);
  return directory.empty()?leaf:directory+"/"+leaf;
}
}
StatsCollector::StatsCollector(const NativeUserStorage& storage,NativeStatsSource& source)
    :m_storage(&storage),m_source(&source) {
  m_state.startFrame=m_state.lastUpdate=source.frame();
}
StatsCollector::~StatsCollector()=default;
std::string StatsCollector::row(const State& state) {
  return std::to_string(state.timeSeconds)+"\t"+std::to_string(state.build)+"\t"+
      std::to_string(state.move)+"\t"+std::to_string(state.attack)+"\t"+
      std::to_string(state.scrollMoves)+"\t"+std::to_string(state.scrollFrames/FramesPerSecond)+
      "\t0\t"+std::to_string(state.units.money)+"\t"+std::to_string(state.units.playerUnits)+
      "\t"+std::to_string(state.units.aiUnits)+"\n";
}
bool StatsCollector::publish(const std::string& name,const std::string& bytes,bool append) {
  try {
    FileCloseOwner input;
    if(append) {
      input.reset(m_storage->openReadFile(NativeUserArea::Data,name));
      if(!input) throw NativeStorageError();
    }
    const auto previous=input?std::uint64_t(input->size()):0;
    if(bytes.size()>std::uint64_t(INT32_MAX)-previous) throw NativeStorageError();
    auto output=m_storage->beginWrite(NativeUserArea::Data,name);
    std::array<char,16384> backing{};
    Int remaining=static_cast<Int>(previous);
    while(remaining) {
      const Int count=std::min<Int>(remaining,backing.size());
      if(input->read(backing.data(),count)!=count) throw NativeStorageError();
      output->write(backing.data(),count);remaining-=count;
    }
    output->write(bytes.data(),static_cast<Int>(bytes.size()));
    const auto result=output->commit();
    m_outputDurable=result==NativeCommitResult::Durable;m_outputFailed=false;
    return true;
  } catch(const NativeStorageError&) {m_outputFailed=true;return false;}
}
void StatsCollector::reset() {
  // A failed new-session reset must not append the new world to the old report.
  m_resetPending=true;m_outputFailed=true;
  const auto frame=m_source->frame();
  const auto identity=m_source->identity();
  const auto date=calendar(m_source->timestamp());
  auto name=filename(identity,date,*m_storage);
  State candidate{};candidate.startFrame=candidate.lastUpdate=frame;
  candidate.units=m_source->sample();
  std::string header="---------------------------------------------------\nDate:\t"+reportTime(date)+
      "Map:\t"+identity.map.str()+"\nSide:\t"+identity.side.str()+
      "\n---------------------------------------------------\n\n"+
      "Time*\tBC\tMC\tAC\tSMC\tST*\tOC\t$$$\t#PU\t#AIU\n"+row(candidate);
  if(!publish(name,header,false)) return;
  m_statsFileName.swap(name);candidate.units={};m_state=candidate;m_resetPending=false;
}
void StatsCollector::collectMsgStats(const GameMessage* message) {
  if(!message) throw ERROR_BAD_ARG;
  if(message->getPlayerIndex()!=m_source->localPlayerIndex()) return;
  switch(message->getType()) {
    case GameMessage::MSG_QUEUE_UNIT_CREATE:
    case GameMessage::MSG_DOZER_CONSTRUCT:
    case GameMessage::MSG_DOZER_CONSTRUCT_LINE:++m_state.build;break;
    default:break;
  }
}
void StatsCollector::collectUnitCountStats() {m_state.units=m_source->sample();}
void StatsCollector::update() {
  if(!outputReady()) return;
  const Int seconds=m_source->intervalSeconds();
  if(seconds<=0) throw ERROR_BAD_ARG;
  const auto frame=m_source->frame();
  const auto elapsed=UnsignedInt(frame-m_state.lastUpdate);
  if(std::uint64_t(elapsed)<std::uint64_t(seconds)*FramesPerSecond) return;
  State candidate=m_state;candidate.units=m_source->sample();
  if(candidate.scrolling) {
    candidate.scrollFrames+=UnsignedInt(frame-candidate.scrollBegin);candidate.scrollBegin=frame;
  }
  candidate.timeSeconds+=static_cast<UnsignedInt>(seconds);
  if(!publish(m_statsFileName,row(candidate),true)) return;
  candidate.build=candidate.move=candidate.attack=candidate.scrollMoves=0;
  candidate.scrollFrames=0;candidate.units={};candidate.lastUpdate=frame;m_state=candidate;
}
void StatsCollector::incrementScrollMoveCount() {++m_state.scrollMoves;}
void StatsCollector::incrementAttackCount() {++m_state.attack;}
void StatsCollector::incrementBuildCount() {++m_state.build;}
void StatsCollector::incrementMoveCount() {++m_state.move;}
void StatsCollector::startScrollTime() {
  const auto frame=m_source->frame();
  if(!m_state.scrolling) {m_state.scrolling=true;m_state.scrollBegin=frame;}
  ++m_state.scrollMoves;
}
void StatsCollector::endScrollTime() {
  if(!m_state.scrolling) return;
  const auto frame=m_source->frame();
  m_state.scrollFrames+=UnsignedInt(frame-m_state.scrollBegin);m_state.scrolling=false;
}
void StatsCollector::writeFileEnd() {
  if(!outputReady()) return;
  const auto frame=m_source->frame();State candidate=m_state;
  candidate.units=m_source->sample();
  candidate.timeSeconds+=UnsignedInt(frame-candidate.lastUpdate)/FramesPerSecond;
  if(candidate.scrolling) candidate.scrollFrames+=UnsignedInt(frame-candidate.scrollBegin);
  const auto identity=m_source->identity();
  std::string footer=row(candidate)+"---------------------------------------------------\nEnd Time:\t"+
      reportTime(calendar(m_source->timestamp()))+"\n"+
      "=KEY===============================================\n"
      "Time* = The Time Interval\nBC = Build Commands\nMC = Move Commands\n"
      "AC = Attack Commands\nSMC = Scroll Map Commands\nST* = Scroll Time in Seconds\n"
      "OC = Other Commands (N/A)\n$$$ = Local Player's Cash Amount\n"
      "#PU = # of Player's Units\n#AIU = # of AI's Units\n"
      "===================================================\n"
      "* Times are in Game Seconds which are based off of frames. Current fps is set to "+
      std::to_string(FramesPerSecond)+"\n";
  if(identity.benchmarkSeconds>0) {
    const auto frames=UnsignedInt(frame-candidate.startFrame);
    std::array<char,64> fps{};
    const int count=std::snprintf(fps.data(),fps.size(),"%.2f",double(frames)/identity.benchmarkSeconds);
    if(count<0 || std::size_t(count)>=fps.size()) throw ERROR_BAD_ARG;
    footer+="\n*** BENCHMARK MODE STATS ***\n Frames = "+std::to_string(frames)+
        "\nSeconds = "+std::to_string(identity.benchmarkSeconds)+"\n    FPS = "+fps.data()+"\n";
  }
  if(!publish(m_statsFileName,footer,true)) return;
  candidate.ended=true;candidate.scrolling=false;m_state=candidate;
}
