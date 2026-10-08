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

#include "Common/Recorder.h"
#include "Common/NativeTransferWire.h"
#include "Common/NativeReplayCommand.h"
#include "Common/NativeSourceStrings.h"
#include "Common/FileSystem.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"
#include "Common/GlobalData.h"
#include "Common/GameEngine.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/InGameUI.h"
#include "GameClient/Shell.h"
#include "GameClient/GameText.h"

#include "GameNetwork/LANAPICallbacks.h"
#include "GameNetwork/GameMessageParser.h"
#include "GameNetwork/GameInfo.h"
#include "GameNetwork/networkutil.h"
#include "GameLogic/GameLogic.h"
#include "Common/RandomValue.h"
#include "Common/CRCDebug.h"
#include "Common/version.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

Int REPLAY_CRC_INTERVAL = 100;

const char *replayExtention = ".rep";
const char *lastReplayFileName = "00000000";	// a name the user is unlikely to ever type, but won't cause panic & confusion

static void retireRecorderCRC(CRCInfo*& owner) noexcept;
static const UnsignedInt startTimeOffset = 6;
static const UnsignedInt endTimeOffset = startTimeOffset + 4;
static const UnsignedInt framesOffset = endTimeOffset + 4;
static const UnsignedInt desyncOffset = framesOffset + sizeof(UnsignedInt);
static const UnsignedInt quitEarlyOffset = desyncOffset + sizeof(Bool);
static const UnsignedInt disconOffset = quitEarlyOffset + sizeof(Bool);

void RecorderClass::logGameStart(AsciiString options)
{
    NativeReplaySession::OwnerGuard sessionGuard(m_file);
	if (!m_file)
		return;

	time(&m_startTime);
	UnsignedInt fileSize = m_file->position();
	// move to appropriate offset
	if (!m_file->seek(startTimeOffset, SEEK_SET))
	{
		// save off start time
		m_file->writeEpoch(m_startTime);
	}
	// move back to end of stream
#ifdef DEBUG_CRASHING
	Int res =
#endif
		m_file->seek(fileSize, SEEK_SET);
	DEBUG_ASSERTCRASH(res == 0, ("Could not seek to end of file!"));

#if defined(_DEBUG) || defined(_INTERNAL)
	if (TheNetwork && TheGlobalData->m_saveStats)
	{
		//if (TheLAN)
		{
			unsigned long bufSize = MAX_COMPUTERNAME_LENGTH + 1;
			char computerName[MAX_COMPUTERNAME_LENGTH + 1];
			if (!GetComputerName(computerName, &bufSize))
			{
				strcpy(computerName, "unknown");
			}
			AsciiString statsFile = TheGlobalData->m_baseStatsDir;
			TheFileSystem->createDirectory(statsFile);
			statsFile.concat(computerName);
			statsFile.concat(".txt");
			FILE *logFP = fopen(statsFile.str(), "a+");
			if (!logFP)
			{
				// try again locally
				TheWritableGlobalData->m_baseStatsDir = TheGlobalData->getPath_UserData();
				statsFile = TheGlobalData->m_baseStatsDir;
				statsFile.concat(computerName);
				statsFile.concat(".txt");
				logFP = fopen(statsFile.str(), "a+");
			}
			if (logFP)
			{
				struct tm *t2 = localtime(&m_startTime);
				fprintf(logFP, "\nGame start at %s\tOptions are %s\n", asctime(t2), options.str());
				fclose(logFP);
			}
		}
	}
#endif
}

void RecorderClass::logPlayerDisconnect(UnicodeString player, Int slot)
{
    NativeReplaySession::OwnerGuard sessionGuard(m_file);
	if (!m_file)
		return;

	DEBUG_ASSERTCRASH((slot >= 0) && (slot < MAX_SLOTS), ("Attempting to disconnect an invalid slot number"));
	if ((slot < 0) || (slot >= (MAX_SLOTS)))
	{
		return;
	}
	UnsignedInt fileSize = m_file->position();
	// move to appropriate offset
	if (!m_file->seek(disconOffset + slot*sizeof(Bool), SEEK_SET))
	{
		// save off discon status
		Bool b = TRUE;
		m_file->write(&b, sizeof(Bool), 1);
	}
	// move back to end of stream
#ifdef DEBUG_CRASHING
	Int res =
#endif
		m_file->seek(fileSize, SEEK_SET);
	DEBUG_ASSERTCRASH(res == 0, ("Could not seek to end of file!"));

#if defined(_DEBUG) || defined(_INTERNAL)
	if (TheGlobalData->m_saveStats)
	{
		unsigned long bufSize = MAX_COMPUTERNAME_LENGTH + 1;
		char computerName[MAX_COMPUTERNAME_LENGTH + 1];
		if (!GetComputerName(computerName, &bufSize))
		{
			strcpy(computerName, "unknown");
		}
		AsciiString statsFile = TheGlobalData->m_baseStatsDir;
		statsFile.concat(computerName);
		statsFile.concat(".txt");
		FILE *logFP = fopen(statsFile.str(), "a+");
		if (logFP)
		{
			time_t t;
			time(&t);
			struct tm *t2 = localtime(&t);
			fprintf(logFP, "\tPlayer %ls dropped at %s", player.str(), asctime(t2));
			fclose(logFP);
		}
	}
#endif
}

void RecorderClass::logCRCMismatch( void )
{
    NativeReplaySession::OwnerGuard sessionGuard(m_file);
	if (!m_file)
		return;

	UnsignedInt fileSize = m_file->position();
	// move to appropriate offset
	if (!m_file->seek(desyncOffset, SEEK_SET))
	{
		// save off desync status
		Bool b = TRUE;
		m_file->write(&b, sizeof(Bool), 1);
	}
	// move back to end of stream
#ifdef DEBUG_CRASHING
	Int res =
#endif
		m_file->seek(fileSize, SEEK_SET);
	DEBUG_ASSERTCRASH(res == 0, ("Could not seek to end of file!"));

#if defined(_DEBUG) || defined(_INTERNAL)
	if (TheGlobalData->m_saveStats)
	{
		m_wasDesync = TRUE;
		unsigned long bufSize = MAX_COMPUTERNAME_LENGTH + 1;
		char computerName[MAX_COMPUTERNAME_LENGTH + 1];
		if (!GetComputerName(computerName, &bufSize))
		{
			strcpy(computerName, "unknown");
		}
		AsciiString statsFile = TheGlobalData->m_baseStatsDir;
		statsFile.concat(computerName);
		statsFile.concat(".txt");
		FILE *logFP = fopen(statsFile.str(), "a+");
		if (logFP)
		{
			time_t t;
			time(&t);
			struct tm *t2 = localtime(&t);
			fprintf(logFP, "\tCRC mismatch at %s", asctime(t2));
			fclose(logFP);
		}
	}
#endif
}

void RecorderClass::logGameEnd( void )
{
    NativeReplaySession::OwnerGuard sessionGuard(m_file);
	if (!m_file)
		return;

	time_t t;
	time(&t);
	UnsignedInt duration = TheGameLogic->getFrame();
	UnsignedInt fileSize = m_file->position();
	// move to appropriate offset
	if (!m_file->seek(endTimeOffset, SEEK_SET))
	{
		// save off end time
		m_file->writeEpoch(t);
	}
	// move to appropriate offset
	if (!m_file->seek(framesOffset, SEEK_SET))
	{
		// save off duration
		m_file->write(&duration, sizeof(UnsignedInt), 1);
	}
	// move back to end of stream
#ifdef DEBUG_CRASHING
	Int res =
#endif
		m_file->seek(fileSize, SEEK_SET);
	DEBUG_ASSERTCRASH(res == 0, ("Could not seek to end of file!"));

#if defined(_DEBUG) || defined(_INTERNAL)
	if (TheNetwork && TheGlobalData->m_saveStats)
	{
		//if (TheLAN)
		{
			unsigned long bufSize = MAX_COMPUTERNAME_LENGTH + 1;
			char computerName[MAX_COMPUTERNAME_LENGTH + 1];
			if (!GetComputerName(computerName, &bufSize))
			{
				strcpy(computerName, "unknown");
			}
			AsciiString statsFile = TheGlobalData->m_baseStatsDir;
			statsFile.concat(computerName);
			statsFile.concat(".txt");
			FILE *logFP = fopen(statsFile.str(), "a+");
			if (logFP)
			{
				struct tm *t2 = localtime(&t);
				duration = t - m_startTime;
				Int minutes = duration/60;
				Int seconds = duration%60;
				fprintf(logFP, "Game end at   %s(%d:%2.2d elapsed time)\n", asctime(t2), minutes, seconds);
				fclose(logFP);
			}
		}
	}
#endif
}

#ifdef DEBUG_LOGGING
	#if defined(_INTERNAL)
		#define DEBUG_FILE_NAME				"DebugLogFileI.txt"
		#define DEBUG_FILE_NAME_PREV	"DebugLogFilePrevI.txt"
	#elif defined(_DEBUG)
		#define DEBUG_FILE_NAME				"DebugLogFileD.txt"
		#define DEBUG_FILE_NAME_PREV	"DebugLogFilePrevD.txt"
	#else
		#define DEBUG_FILE_NAME				"DebugLogFile.txt"
		#define DEBUG_FILE_NAME_PREV	"DebugLogFilePrev.txt"
	#endif
#endif

void RecorderClass::cleanUpReplayFile( void )
{
#if defined(_DEBUG) || defined(_INTERNAL)
	if (TheGlobalData->m_saveStats)
	{
		char fname[_MAX_PATH+1];
		strncpy(fname, TheGlobalData->m_baseStatsDir.str(), _MAX_PATH);
		strncat(fname, m_fileName.str(), _MAX_PATH - strlen(fname));
		DEBUG_LOG(("Saving replay to %s\n", fname));
		AsciiString oldFname;
		oldFname.format("%s%s", getReplayDir().str(), m_fileName.str());
		CopyFile(oldFname.str(), fname, TRUE);
#ifdef DEBUG_FILE_NAME
		AsciiString debugFname = fname;
		debugFname.removeLastChar();
		debugFname.removeLastChar();
		debugFname.removeLastChar();
		debugFname.concat("txt");
		UnsignedInt fileSize = 0;
		FILE *fp = fopen(DEBUG_FILE_NAME, "rb");
		if (fp)
		{
			fseek(fp, 0, SEEK_END);
			fileSize = ftell(fp);
			fclose(fp);
			fp = NULL;
			DEBUG_LOG(("Log file size was %d\n", fileSize));
		}

		const int MAX_DEBUG_SIZE = 65536;
		if (fileSize <= MAX_DEBUG_SIZE || TheGlobalData->m_saveAllStats)
		{
			DEBUG_LOG(("Using CopyFile to copy %s\n", DEBUG_FILE_NAME));
			CopyFile(DEBUG_FILE_NAME, debugFname.str(), TRUE);
		}
		else
		{
			DEBUG_LOG(("manual copy of %s\n", DEBUG_FILE_NAME));
			FILE *ifp = fopen(DEBUG_FILE_NAME, "rb");
			FILE *ofp = fopen(debugFname.str(), "wb");
			if (ifp && ofp)
			{
				fseek(ifp, fileSize-MAX_DEBUG_SIZE, SEEK_SET);
				char buf[4096];
				Int len;
				while ( (len=fread(buf, 1, 4096, ifp)) > 0 )
				{
					fwrite(buf, 1, len, ofp);
				}
				fclose(ofp);
				fclose(ifp);
				ifp = NULL;
				ofp = NULL;
			}
			else
			{
				if (ifp) fclose(ifp);
				if (ofp) fclose(ofp);
				ifp = NULL;
				ofp = NULL;
			}
		}
#endif // DEBUG_FILE_NAME
	}
#endif
}

/**
 * The recorder object.
 */
RecorderClass *TheRecorder = NULL;

/**
 * Constructor
 */
RecorderClass::RecorderClass() 
{
    m_crcInfo=nullptr;
	m_originalGameMode = GAME_NONE;
	m_mode = RECORDERMODETYPE_RECORD;
	m_file = nullptr;
	m_fileName.clear();
	m_currentFilePosition = 0;
	//Added By Sadullah Nader
	//Initializtion(s) inserted
	m_doingAnalysis = FALSE;
	m_nextFrame = 0;
	m_wasDesync = FALSE;
	//

	init(); // just for the heck of it.
}

/**
 * Destructor
 */
RecorderClass::~RecorderClass() {
    m_file.reset();
    retireRecorderCRC(m_crcInfo);
}

/**
 * Initialization
 * The recorder will record by default since every game will be recorded.
 * Obviously a game that is being played back will not be recorded.
 * Since the playback is done through a special interface, that interface
 * will set the recorder mode to RECORDERMODETYPE_PLAYBACK.
 */
void RecorderClass::init() {
    m_file.reset();
    retireRecorderCRC(m_crcInfo);
    m_startTime=0;
	m_originalGameMode = GAME_NONE;
	m_mode = RECORDERMODETYPE_NONE;
	m_file = nullptr;
	m_fileName.clear();
	m_currentFilePosition = 0;
	m_gameInfo.clearSlotList();
	m_gameInfo.reset();
	if (TheGlobalData->m_pendingFile.isEmpty())
		m_gameInfo.setMap(TheGlobalData->m_mapName);
	else
		m_gameInfo.setMap(TheGlobalData->m_pendingFile);
	m_gameInfo.setSeed(GetGameLogicRandomSeed());
	m_wasDesync = FALSE;
	m_doingAnalysis = FALSE;
}

/**
 * Reset the recorder to the "initialized state."
 */
void RecorderClass::reset() {
	if (m_file != nullptr) {
		m_file.reset();
		m_file = nullptr;
	}
	m_fileName.clear();

	init();
}

/**
 * update
 * Do the update for this frame.
 */
void RecorderClass::update() {
	if (m_mode == RECORDERMODETYPE_RECORD || m_mode == RECORDERMODETYPE_NONE) {
		updateRecord();
	} else if (m_mode == RECORDERMODETYPE_PLAYBACK) {
		updatePlayback();
	}
}

/**
 * Do the update for the next frame of this playback.
 */
void RecorderClass::updatePlayback() {
	cullBadCommands();	// Remove any bad commands that have been inserted by the local user that shouldn't be
											// executed during playback.

	if (m_nextFrame == -1) {
		// This is reached if there are no more commands to be executed.
		return;
	}
	UnsignedInt curFrame = TheGameLogic->getFrame();
	if (m_doingAnalysis)
		curFrame = m_nextFrame;

	// While there are commands to be queued up for this frame, do it.
	while (m_nextFrame == curFrame) {
		appendNextCommand();	// append the next command to TheCommandQueue
		readNextFrame();	// Read the next command's frame number for playback.
	}
}

/**
 * Stop the currently running playback. This is probably due either to the user exiting out of the playback or
 * reaching the end of the playback file.
 */
void RecorderClass::stopPlayback() {
	if (m_file != nullptr) {
		m_file.reset();
		m_file = nullptr;
	}
	m_fileName.clear();
	// Don't clear the game data if the replay is over - let things continue
//#ifdef DEBUG_CRC
	if (!m_doingAnalysis)
		TheMessageStream->appendMessage(GameMessage::MSG_CLEAR_GAME_DATA);
//#endif
}

/**
 * Update function for recording a game. Basically all the pertinant logic commands for this frame are written out
 * to a file.
 */
void RecorderClass::updateRecord() 
{
    NativeReplaySession::OwnerGuard sessionGuard(m_file);
    if (!TheCommandList || !TheGameLogic) throw ERROR_BAD_ARG;
	Bool needFlush = FALSE;
	static Int lastFrame = -1;
	GameMessage *msg = TheCommandList->getFirstMessage();
	while (msg != NULL) {
		if (msg->getType() == GameMessage::MSG_NEW_GAME &&
			 msg->getArgument(0)->integer != GAME_SHELL && 
			 msg->getArgument(0)->integer != GAME_SINGLE_PLAYER && // Due to the massive amount of scripts that use <local player> in GC and single player, replays have been cut for them.
			 msg->getArgument(0)->integer != GAME_NONE) 
		{
			m_originalGameMode = msg->getArgument(0)->integer;
			DEBUG_LOG(("RecorderClass::updateRecord() - original game is mode %d\n", m_originalGameMode));
			lastFrame = 0;
			GameDifficulty diff = DIFFICULTY_NORMAL;
			if (msg->getArgumentCount() >= 2)
				diff = (GameDifficulty)msg->getArgument(1)->integer;
			Int rankPoints = 0;
			if (msg->getArgumentCount() >= 3)
				rankPoints = msg->getArgument(2)->integer;
			Int maxFPS = 0;
			if (msg->getArgumentCount() >= 4)
				maxFPS = msg->getArgument(3)->integer;

			startRecording(diff, m_originalGameMode, rankPoints, maxFPS);
		} else if (msg->getType() == GameMessage::MSG_CLEAR_GAME_DATA) {
			if (m_file != nullptr) {
				lastFrame = -1;
				writeToFile(msg);
				stopRecording();
			}
			m_fileName.clear();
		} else {
			if (m_file != nullptr) {
				if ((msg->getType() > GameMessage::MSG_BEGIN_NETWORK_MESSAGES) &&
						(msg->getType() < GameMessage::MSG_END_NETWORK_MESSAGES)) {
					// Only write the important messages to the file.
					writeToFile(msg);
					needFlush = TRUE;
				}
			}
		}
		msg = msg->next();
	}

	if (needFlush) {
		m_file->flush();
	}
}

/**
 * Start a new file for recording. This will always overwrite the "LastReplay.rep" file with the new one.
 * So don't call this unless you really mean it.
 */
void RecorderClass::startRecording(GameDifficulty diff, Int originalGameMode, Int rankPoints, Int maxFPS) {
    NativeReplaySession::OwnerGuard sessionGuard(m_file);
    if (!TheNativeUserStorage) { m_mode=RECORDERMODETYPE_NONE; return; }
	DEBUG_ASSERTCRASH(m_file == nullptr, ("Starting to record game while game is in progress."));

	reset();

	m_mode = RECORDERMODETYPE_RECORD;

	AsciiString filepath = getReplayDir();

	// We have to make sure the replay dir exists. 
	// Protected beginWrite creates only captured user directories.

	m_fileName = getLastReplayFileName();
	m_fileName.concat(getReplayExtention());
	filepath.concat(m_fileName);
	if (!TheNativeUserStorage) { m_mode=RECORDERMODETYPE_NONE; return; }
    try { m_file=NativeReplaySession::record(*TheNativeUserStorage,filepath); }
    catch (const NativeStorageError&) { m_mode=RECORDERMODETYPE_NONE; m_fileName.clear(); return; }
	if (m_file == nullptr) {
		DEBUG_ASSERTCRASH(m_file != nullptr, ("Failed to create replay file"));
		return;
	}
	m_file->write("GENREP",1,6);

	//
	// save space for stats to be filled in.
	//
	// **** if this changes, change the LAN code above ****
	//
	time_t t = 0;
	m_file->writeEpoch(t);	// reserve space for start time
	m_file->writeEpoch(t);	// reserve space for end time

	UnsignedInt frames = 0;
	m_file->write(&frames, sizeof(UnsignedInt), 1);	// reserve space for duration in frames

	Bool b = FALSE;
	m_file->write(&b, sizeof(Bool), 1);	// reserve space for flag (true if we desync)
	m_file->write(&b, sizeof(Bool), 1);	// reserve space for flag (true if we quit early)
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		m_file->write(&b, sizeof(Bool), 1);	// reserve space for flag (true if player i disconnects)
	}

	// Print out the name of the replay.
	UnicodeString replayName;
	replayName = TheGameText->fetch("GUI:LastReplay");
	m_file->writeUnicode(replayName);

	// Date and Time
	NativeCalendarTime systemTime;
	systemTime = nativeCalendarNow();
	m_file->write(&systemTime, sizeof(NativeCalendarTime), 1);

	// write out version info
	UnicodeString versionString = TheVersion->getUnicodeVersion();
	UnicodeString versionTimeString = TheVersion->getUnicodeBuildTime();
	UnsignedInt versionNumber = TheVersion->getVersionNumber();
	m_file->writeUnicode(versionString);
	m_file->writeUnicode(versionTimeString);
	m_file->write(&versionNumber, sizeof(UnsignedInt), 1);
	m_file->write(&(TheGlobalData->m_exeCRC), sizeof(UnsignedInt), 1);
	m_file->write(&(TheGlobalData->m_iniCRC), sizeof(UnsignedInt), 1);

	// Number of players
	/*
	Int numPlayers = ThePlayerList->getPlayerCount();
	m_file->write(&numPlayers, sizeof(numPlayers), 1);
	*/

	// Write the slot list.
	AsciiString theSlotList;
	Int localIndex = -1;
	if (TheNetwork)
	{
		if (TheLAN)
		{
			GameInfo *game = TheLAN->GetMyGame();
			DEBUG_ASSERTCRASH(game, ("Starting a LAN game with no LANGameInfo object!"));
			theSlotList = GameInfoToAsciiString(game);

			for (Int i=0; i<MAX_SLOTS; ++i)
			{
				if (game->getLocalIP() == game->getSlot(i)->getIP())
				{
					localIndex = i;
					break;
				}
			}
		}
		else
		{
			// Excluded online backend cannot provide a native replay game owner.
            throw ERROR_BAD_ARG;
		}
	}
	else
	{
    if(TheSkirmishGameInfo)
    {
			TheSkirmishGameInfo->setCRCInterval(REPLAY_CRC_INTERVAL);
      theSlotList = GameInfoToAsciiString(TheSkirmishGameInfo);
      DEBUG_LOG(("GameInfo String: %s\n",theSlotList.str()));
			localIndex = 0;
    }
    else
    {
		  // single player.  format the generic (empty) slotlist
			m_gameInfo.setCRCInterval(REPLAY_CRC_INTERVAL);
		  theSlotList = GameInfoToAsciiString(&m_gameInfo);
    }
	}
	logGameStart(theSlotList);
	DEBUG_LOG(("RecorderClass::startRecording - theSlotList = %s\n", theSlotList.str()));

	// write slot list (starting spots, color, alliances, etc
	m_file->write(theSlotList.str(), theSlotList.getLength() + 1, 1);
	const auto localIndexText=std::to_string(localIndex);
    m_file->write(localIndexText.c_str(),1,localIndexText.size()+1);

	/*
	/// @todo fix this to use starting spots and player alliances when those are put in the game.
	for (Int i = 0; i < numPlayers; ++i) {
		Player *player = ThePlayerList->getNthPlayer(i);
		if (player == NULL) {
			continue;
		}
		UnicodeString name = player->getPlayerDisplayName();
		fwprintf(m_file, L"%s", name.str());
		fputwc(0, m_file);
		UnicodeString faction = player->getFaction()->getFactionDisplayName();
		fwprintf(m_file, L"%s", faction.str());
		fputwc(0, m_file);
		Int color = player->getColor()->getAsInt();
		m_file->write(&color, sizeof(color), 1);
		Int team = 0;
		Int startingSpot = 0;
		m_file->write(&startingSpot, sizeof(Int), 1);
		m_file->write(&team, sizeof(Int), 1);
	}
	*/

	// Write the game difficulty.
	m_file->write(&diff, sizeof(Int), 1);

	// Write original game mode
	m_file->write(&originalGameMode, sizeof(originalGameMode), 1);

	// Write rank points to add at game start
	m_file->write(&rankPoints, sizeof(rankPoints), 1);

	// Write maxFPS chosen
	m_file->write(&maxFPS, sizeof(maxFPS), 1);

	DEBUG_LOG(("RecorderClass::startRecording() - diff=%d, mode=%d, FPS=%d\n", diff, originalGameMode, maxFPS));

	/*
	// Write the map name.
	fprintf(m_file, "%s", (TheGlobalData->m_mapName).str());
	fputc(0, m_file);
	*/

	/// @todo Need to write game options when there are some to be written.
}

/**
 * This will stop the current recording session and close the file. This should always be called at the end of
 * every game.
 */
void RecorderClass::stopRecording() {
    NativeReplaySession::OwnerGuard sessionGuard(m_file);
	logGameEnd();
    if (m_file) {
        const auto result=m_file->commit();
        if (result==NativeCommitResult::PublishedDurabilityUnknown) DEBUG_LOG(("Replay published; durability unknown.\n"));
        m_file.reset();
    }
	if (TheNetwork)
	{
		//if (TheLAN)
		{
			if (m_wasDesync)
				cleanUpReplayFile();
			m_wasDesync = FALSE;
		}
	}
	if (m_file != nullptr) {
		m_file.reset();
		m_file = nullptr;
	}
	m_fileName.clear();
}

/**
 * Write this game message to the record file. This also writes the game message's execution frame.
 */
void RecorderClass::writeToFile(GameMessage * msg) {
    NativeReplaySession::OwnerGuard sessionGuard(m_file);
    if (!msg || !m_file || !TheGameLogic) throw ERROR_BAD_ARG;
    // All source admission and payload preparation precedes the first write.
    nativeReplayWriteCommand(*msg,TheGameLogic->getFrame(),[&](const void* bytes,std::size_t count) {
        return m_file->write(bytes, 1, count);
    });
    if (m_file->flush()) throw XFER_WRITE_ERROR;
}

void RecorderClass::writeArgument(GameMessageArgumentDataType type, const GameMessageArgumentType arg) {
    NativeReplaySession::OwnerGuard sessionGuard(m_file);
    if (!m_file) throw ERROR_BAD_ARG;
    NativeReplayCommandBytes candidate;
    nativeReplayEncodeArgument(candidate,type,arg);
    if (m_file->write(candidate.bytes.data(), 1, candidate.count)!=candidate.count) throw XFER_WRITE_ERROR;
}

/**
 * Read in a replay header, for (1) populating a replay listbox or (2) starting playback.  In
 * case (2), set FILE *m_file.
 */
Bool RecorderClass::readReplayHeader(ReplayHeader& acceptedHeader)
{
    NativeReplaySession::OwnerGuard sessionGuard(m_file);
    if (!TheNativeUserStorage) return FALSE;
    ReplayHeader header{};
    header.filename=acceptedHeader.filename; header.forPlayback=acceptedHeader.forPlayback;
	AsciiString filepath = getReplayDir();
	filepath.concat(header.filename.str());
	if (!TheNativeUserStorage) return FALSE;
    m_file=NativeReplaySession::playback(*TheNativeUserStorage,filepath);
	if (m_file == nullptr)
	{
		DEBUG_LOG(("Can't open %s (%s)\n", filepath.str(), header.filename.str()));
		return FALSE;
	}

	// Read the GENREP header.
	char genrep[7];
	m_file->readExact(genrep,6);
	genrep[6] = 0;
	if (strncmp(genrep, "GENREP", 6)) {
		DEBUG_LOG(("RecorderClass::readReplayHeader - replay file did not have GENREP at the start.\n"));
		m_file.reset();
		m_file = nullptr;
		return FALSE;
	}

	// read in some stats
	header.startTime=m_file->readEpoch();
	header.endTime=m_file->readEpoch();

	header.frameDuration=m_file->readWord();

	header.desyncGame=m_file->readBoolean();
	header.quitEarly=m_file->readBoolean();
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		header.playerDiscons[i]=m_file->readBoolean();
	}

	// Read the Replay Name.  We don't actually do anything with it.  Oh well.
	header.replayName = readUnicodeString();

	// Read the date and time.  We don't really do anything with this either. Oh well.
	m_file->readExact(&header.timeVal,sizeof(NativeCalendarTime));

	// Read in the Version info
	header.versionString = readUnicodeString();
	header.versionTimeString = readUnicodeString();
	header.versionNumber=m_file->readWord();
	header.exeCRC=m_file->readWord();
	header.iniCRC=m_file->readWord();

	// Read in the GameInfo
	header.gameOptions = readAsciiString();

	AsciiString playerIndex = readAsciiString();
	header.localPlayerIndex=nativeSourceInteger<Int>(playerIndex.str());
	if (header.localPlayerIndex < -1 || header.localPlayerIndex >= MAX_SLOTS)
	{
		DEBUG_LOG(("RecorderClass::readReplayHeader - invalid local slot number.\n"));
		m_file.reset();
		m_file = nullptr;
		return FALSE;
	}
	m_gameInfo.reset();
	m_gameInfo.enterGame();
	DEBUG_LOG(("RecorderClass::readReplayHeader - GameInfo = %s\n", header.gameOptions.str()));
	if (!ParseAsciiStringToGameInfo(&m_gameInfo, header.gameOptions))
	{
		DEBUG_LOG(("RecorderClass::readReplayHeader - replay file did not have a valid GameInfo string.\n"));
		m_file.reset();
		m_file = nullptr;
		return FALSE;
	}
	m_gameInfo.startGame(0);

	if (header.localPlayerIndex >= 0)
	{
		Int localIP = m_gameInfo.getSlot(header.localPlayerIndex)->getIP();
		m_gameInfo.setLocalIP(localIP);
	}

	if (!header.forPlayback)
	{
		m_gameInfo.endGame();
		m_gameInfo.reset();
		m_file.reset();
		m_file = nullptr;
	}
    acceptedHeader.swap(header);
	return TRUE;
}

#if defined _DEBUG || defined _INTERNAL
Bool RecorderClass::analyzeReplay( AsciiString filename )
{
	m_doingAnalysis = TRUE;
	return playbackFile(filename);
}

Bool RecorderClass::isAnalysisInProgress( void )
{
	return m_mode == RECORDERMODETYPE_PLAYBACK && m_nextFrame != -1;
}
#endif

AsciiString RecorderClass::getCurrentReplayFilename( void )
{
	if (m_mode == RECORDERMODETYPE_PLAYBACK)
	{
		return m_currentReplayFilename;
	}
	return AsciiString::TheEmptyString;
}

class CRCInfo
{
public:
	CRCInfo();
	void addCRC(UnsignedInt val);
	UnsignedInt readCRC(void);

	void setLocalPlayer(UnsignedInt index) { m_localPlayer = index; }
	UnsignedInt getLocalPlayer(void) { return m_localPlayer; }

	void setSawCRCMismatch(void) { m_sawCRCMismatch = TRUE; }
	Bool sawCRCMismatch(void) { return m_sawCRCMismatch; }

protected:

	Bool m_sawCRCMismatch;
	Bool m_skippedOne;
	std::list<UnsignedInt> m_data;
	UnsignedInt m_localPlayer;
};

static void retireRecorderCRC(CRCInfo*& owner) noexcept { delete owner; owner=nullptr; }

CRCInfo::CRCInfo()
{
	m_localPlayer = ~0;
	m_skippedOne = FALSE;
	m_sawCRCMismatch = FALSE;
}

void CRCInfo::addCRC(UnsignedInt val)
{
	//if (!m_skippedOne)
	//{
	//	m_skippedOne = TRUE;
	//	return;
	//}

	m_data.push_back(val);
	//DEBUG_LOG(("CRCInfo::addCRC() - crc %8.8X pushes list to %d entries (full=%d)\n", val, m_data.size(), !m_data.empty()));
}

UnsignedInt CRCInfo::readCRC(void)
{
	if (m_data.empty())
	{
		//DEBUG_LOG(("CRCInfo::readCRC() - bailing, full=0, size=%d\n", m_data.size()));
		return 0;
	}

	UnsignedInt val = m_data.front();
	m_data.pop_front();
	//DEBUG_LOG(("CRCInfo::readCRC() - returning %8.8X, full=%d, size=%d\n", val, !m_data.empty(), m_data.size()));
	return val;
}

void RecorderClass::handleCRCMessage(UnsignedInt newCRC, Int playerIndex, Bool fromPlayback)
{
	if (fromPlayback)
	{
		//DEBUG_LOG(("RecorderClass::handleCRCMessage() - Adding CRC of %X from %d to m_crcInfo\n", newCRC, playerIndex));
		m_crcInfo->addCRC(newCRC);
		return;
	}

	Int localPlayerIndex = m_crcInfo->getLocalPlayer();
	Bool samePlayer = FALSE;
	AsciiString playerName;
	playerName.format("player%d", localPlayerIndex);
	const Player *p = ThePlayerList->getNthPlayer(playerIndex);
	if (!p || (p->getPlayerNameKey() == NAMEKEY(playerName)))
		samePlayer = TRUE;
	if (samePlayer || (localPlayerIndex < 0))
	{
		UnsignedInt playbackCRC = m_crcInfo->readCRC();
		//DEBUG_LOG(("RecorderClass::handleCRCMessage() - Comparing CRCs of %8.8X/%8.8X from %d\n", newCRC, playbackCRC, playerIndex));
		if (TheGameLogic->getFrame() > 0 && newCRC != playbackCRC && !m_crcInfo->sawCRCMismatch())
		{
			m_crcInfo->setSawCRCMismatch();

			//Kris: Patch 1.01 November 10, 2003 (integrated changes from Matt Campbell)
			// Since we don't seem to have any *visible* desyncs when replaying games, but get this warning
			// virtually every replay, the assumption is our CRC checking is faulty.  Since we're at the
			// tail end of patch season, let's just disable the message, and hope the users believe the
			// problem is fixed. -MDC 3/20/2003
			//TheInGameUI->message("GUI:CRCMismatch");
			DEBUG_CRASH(("Replay has gone out of sync!  All bets are off!\nOld:%8.8X New:%8.8X\nFrame:%d",
				playbackCRC, newCRC, TheGameLogic->getFrame()));
		}
		return;
	}

	//DEBUG_LOG(("RecorderClass::handleCRCMessage() - Skipping CRC of %8.8X from %d (our index is %d)\n", newCRC, playerIndex, localPlayerIndex));
}

/**
 * Return true if this version of the file is the same as our version of the game
 */
Bool RecorderClass::testVersionPlayback(AsciiString filename)
{

	ReplayHeader header;
	header.forPlayback = TRUE;
	header.filename = filename;
	Bool success = readReplayHeader( header );
	if (!success)
	{
		return FALSE;
	}
	Bool versionStringDiff = header.versionString != TheVersion->getUnicodeVersion();
	Bool versionTimeStringDiff = header.versionTimeString != TheVersion->getUnicodeBuildTime();
	Bool versionNumberDiff = header.versionNumber != TheVersion->getVersionNumber();
	Bool exeCRCDiff = header.exeCRC != TheGlobalData->m_exeCRC;
	Bool exeDifferent = versionStringDiff || versionTimeStringDiff || versionNumberDiff || exeCRCDiff;
	Bool iniDifferent = header.iniCRC != TheGlobalData->m_iniCRC;

	if(exeDifferent || iniDifferent)
	{
		return TRUE;
	}
	return FALSE;

}

/**
 * Start playback of the file. Return true or false depending on if the file is
 * a valid replay file or not.
 */
Bool RecorderClass::playbackFile(AsciiString filename) 
{
    NativeReplaySession::OwnerGuard sessionGuard(m_file);
	if (!m_doingAnalysis)
	{
		if (TheGameLogic->isInGame())
		{
			TheGameLogic->clearGameData();
		}
	}

	m_mode = RECORDERMODETYPE_PLAYBACK;

	ReplayHeader header;
	header.forPlayback = TRUE;
	header.filename = filename;
	Bool success = readReplayHeader( header );
	if (!success)
	{
		return FALSE;
	}
#ifdef DEBUG_LOGGING

	Bool versionStringDiff = header.versionString != TheVersion->getUnicodeVersion();
	Bool versionTimeStringDiff = header.versionTimeString != TheVersion->getUnicodeBuildTime();
	Bool versionNumberDiff = header.versionNumber != TheVersion->getVersionNumber();
	Bool exeCRCDiff = header.exeCRC != TheGlobalData->m_exeCRC;
	Bool exeDifferent = versionStringDiff || versionTimeStringDiff || versionNumberDiff || exeCRCDiff;
	Bool iniDifferent = header.iniCRC != TheGlobalData->m_iniCRC;

	AsciiString debugString;
	AsciiString tempStr;
	if (exeDifferent)
	{
		debugString = "EXE is different:\n";
		if (versionStringDiff)
		{
			tempStr.format("   Version [%ls] vs [%ls]\n", TheVersion->getUnicodeVersion().str(), header.versionString.str());
			debugString.concat(tempStr);
		}
		if (versionTimeStringDiff)
		{
			tempStr.format("   Build Time [%ls] vs [%ls]\n", TheVersion->getUnicodeBuildTime().str(), header.versionTimeString.str());
			debugString.concat(tempStr);
		}
		if (versionNumberDiff)
		{
			tempStr.format("   Version Number %8.8X vs %8.8X\n", TheVersion->getVersionNumber(), header.versionNumber);
			debugString.concat(tempStr);
		}
		if (exeCRCDiff)
		{
			tempStr.format("   CRC %8.8X vs %8.8X\n", TheGlobalData->m_exeCRC, header.exeCRC);
			debugString.concat(tempStr);
		}
	}
	if (iniDifferent)
	{
		debugString.concat("INIs are different:\n");
		tempStr.format("   CRC %8.8X vs %8.8X\n", TheGlobalData->m_iniCRC, header.iniCRC);
		debugString.concat(tempStr);
	}
	DEBUG_ASSERTCRASH(!exeDifferent && !iniDifferent, (debugString.str()));
#endif

	TheWritableGlobalData->m_pendingFile = m_gameInfo.getMap();

#ifdef DEBUG_LOGGING
	if (header.localPlayerIndex >= 0)
	{
		DEBUG_LOG(("Local player is %ls (slot %d, IP %8.8X)\n",
			m_gameInfo.getSlot(header.localPlayerIndex)->getName().str(), header.localPlayerIndex, m_gameInfo.getSlot(header.localPlayerIndex)->getIP()));
	}
#endif

	std::unique_ptr<CRCInfo> crcCandidate(NEW CRCInfo);
    retireRecorderCRC(m_crcInfo);
    m_crcInfo=crcCandidate.release();
	m_crcInfo->setLocalPlayer(header.localPlayerIndex);
	REPLAY_CRC_INTERVAL = m_gameInfo.getCRCInterval();
	DEBUG_LOG(("Player index is %d, replay CRC interval is %d\n", m_crcInfo->getLocalPlayer(), REPLAY_CRC_INTERVAL));

	Int difficulty = 0;
	difficulty=std::bit_cast<Int>(m_file->readWord());

	m_originalGameMode=std::bit_cast<Int>(m_file->readWord());

	Int rankPoints = 0;
	rankPoints=std::bit_cast<Int>(m_file->readWord());
	
	Int maxFPS = 0;
	maxFPS=std::bit_cast<Int>(m_file->readWord());

	DEBUG_LOG(("RecorderClass::playbackFile() - original game was mode %d\n", m_originalGameMode));

	readNextFrame();

	// send a message to the logic for a new game
	if (!m_doingAnalysis)
	{
		GameMessage *msg = TheMessageStream->appendMessage( GameMessage::MSG_NEW_GAME );
		msg->appendIntegerArgument(GAME_REPLAY);
		msg->appendIntegerArgument(difficulty);
		msg->appendIntegerArgument(rankPoints);
		if( maxFPS != 0 )
			msg->appendIntegerArgument(maxFPS);
		//InitGameLogicRandom( m_gameInfo.getSeed());
		InitRandom( m_gameInfo.getSeed() );
	}

	m_currentReplayFilename = filename;
	return TRUE;
}

/**
 * Read a unicode string from the current file position. The string is assumed to be 0-terminated.
 */
UnicodeString RecorderClass::readUnicodeString() {
    NativeReplaySession::OwnerGuard sessionGuard(m_file);
    if (!m_file) throw XFER_FILE_NOT_OPEN;
    return nativeReplayReadUnicode([&](void* bytes,Int count) {
        return m_file->read(bytes, 1, static_cast<std::size_t>(count))==static_cast<std::size_t>(count);
    });
}

/**
 * Read an ascii string from the current file position. The string is assumed to be 0-terminated.
 */
AsciiString RecorderClass::readAsciiString() {
    NativeReplaySession::OwnerGuard sessionGuard(m_file);
    if (!m_file) throw XFER_FILE_NOT_OPEN;
    return nativeReplayReadAscii([&](void* bytes,Int count) {
        return m_file->read(bytes, 1, static_cast<std::size_t>(count))==static_cast<std::size_t>(count);
    });
}

/**
 * Read the frame number for the next command in the playback file. If the end of the file is reached, the playback
 * is stopped and the next frame is said to be -1.
 */
void RecorderClass::readNextFrame() {
    NativeReplaySession::OwnerGuard sessionGuard(m_file);
    if (!m_file) throw ERROR_BAD_ARG;
    const auto candidate=nativeReplayReadFrame([&](void* bytes,std::size_t count) {
        const auto read=m_file->read(bytes, 1, count);
        // Input was admitted completely when the protected session opened.
        return read;
    });
    if (candidate) m_nextFrame=*candidate;
    else { m_nextFrame=-1; stopPlayback(); }
}

/** Read a complete offside command before any parent publication. */
void RecorderClass::appendNextCommand() {
    NativeReplaySession::OwnerGuard sessionGuard(m_file);
    if (!m_file || (!m_doingAnalysis && !TheCommandList)) throw ERROR_BAD_ARG;
    nativeReplayPublishCommand([&](void* bytes,std::size_t count) {
        return m_file->read(bytes, 1, count);
    },TheCommandList,m_doingAnalysis);
}

void RecorderClass::readArgument(GameMessageArgumentDataType type, GameMessage *msg) {
    NativeReplaySession::OwnerGuard sessionGuard(m_file);
    if (!m_file || !msg) throw ERROR_BAD_ARG;
    auto read=[&](void* bytes,std::size_t count) { return m_file->read(bytes, 1, count); };
    nativeReplayReadArgument(read,*msg,static_cast<Int>(type));
}

/**
 * This needs to be called for every frame during playback. Basically it prevents the user from inserting.
 */
void RecorderClass::cullBadCommands() {
	if (m_doingAnalysis)
		return;

	GameMessage *msg = TheCommandList->getFirstMessage();
	GameMessage *next = NULL;

	while (msg != NULL) {
		next = msg->next();
		if ((msg->getType() > GameMessage::MSG_BEGIN_NETWORK_MESSAGES) &&
				(msg->getType() < GameMessage::MSG_END_NETWORK_MESSAGES) &&
				(msg->getType() != GameMessage::MSG_LOGIC_CRC)) {

			msg->deleteInstance();
		}

		msg = next;
	}
}

/**
 * returns the directory that holds the replay files.
 */
AsciiString RecorderClass::getReplayDir()
{
    if (!TheNativeUserStorage) throw XFER_INVALID_PARAMETERS;
    return AsciiString((TheNativeUserStorage->paths().data+"/Replays/").c_str());
}

/**
 * returns the file extention for the replay files.
 */
AsciiString RecorderClass::getReplayExtention() {
	return AsciiString(replayExtention);
}

/**
 * returns the file name used for the replay file that is recorded to.
 */
AsciiString RecorderClass::getLastReplayFileName() 
{
#if defined(_DEBUG) || defined(_INTERNAL)
	if (TheNetwork && TheGlobalData->m_saveStats)
	{
		GameInfo *game = NULL;
		if (TheLAN)
			game = TheLAN->GetMyGame();
		if (game)
		{
			AsciiString players;
			AsciiString full;
			AsciiString fullPlusNum;
			AsciiString mapName = game->getMap();
			const char *fname = mapName.reverseFind('\\');
			if (fname)
				mapName = fname+1;
			for (Int i=0; i<MAX_SLOTS; ++i)
			{
				GameSlot *slot = game->getSlot(i);
				if (slot && slot->isHuman())
				{
					AsciiString player;
					player.format("%ls_", slot->getName().str());
					players.concat(player);
				}
			}
			full.format("%s%s_%d_%d", players.str(), mapName.str(), game->getSeed(), game->getLocalSlotNum());
			AsciiString testString;
			testString.format("%s%s%s", getReplayDir().str(), full.str(), replayExtention);

			FILE *fp;
			fp = fopen(testString.str(), "rb");
			if (fp)
			{
				fclose(fp);
			}
			else
			{
				return full;
			}
			Int test = 1;
			while (test < 20)
			{
				fullPlusNum.format("%s_%d", full.str(), test);
				testString.format("%s%s%s", getReplayDir().str(), fullPlusNum.str(), replayExtention);
				fp = fopen(testString.str(), "rb");
				if (fp)
				{
					fclose(fp);
					++test;
				}
				else
				{
					return fullPlusNum;
				}
			}
			return fullPlusNum;
		}
	}
#endif
	return AsciiString(lastReplayFileName);
}

/**
 * return the current operating mode of TheRecorder.
 */
RecorderModeType RecorderClass::getMode() {
	return m_mode;
}

///< Show or Hide the Replay controls
void RecorderClass::initControls()
{
	NameKeyType parentReplayControlID = TheNameKeyGenerator->nameToKey( AsciiString("ReplayControl.wnd:ParentReplayControl") );
	GameWindow *parentReplayControl = TheWindowManager->winGetWindowFromId( NULL, parentReplayControlID );

	Bool show = (getMode() != RECORDERMODETYPE_PLAYBACK);
	if (parentReplayControl)
	{
		parentReplayControl->winHide(show);	// show the replay control window.
	}
}

///< is this a multiplayer game (record OR playback)?
Bool RecorderClass::isMultiplayer( void )
{

	if (m_mode == RECORDERMODETYPE_PLAYBACK)
	{
		GameSlot *slot;
		for (int i=0; i<MAX_SLOTS; ++i)
		{
			slot = m_gameInfo.getSlot(i);
			if (slot && slot->isOccupied())	///< slots default to closed for non-networked games
				return true;
		}
	}
	if (TheGameLogic->getGameMode()==GAME_SINGLE_PLAYER) {
		return false; // single player isn't multiplayer.
	}
	if (TheGameLogic->getGameMode()==GAME_SHELL) {
		return false; // shell isn't multiplayer.
	}
	if (TheNetwork || TheSkirmishGameInfo)
		return true;

	return false;
}

/**
 * Create a new recorder object.
 */
RecorderClass * createRecorder() {
	return NEW RecorderClass;
}
