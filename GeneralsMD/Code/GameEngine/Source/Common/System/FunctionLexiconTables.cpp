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

// FILE: FunctionLexicon.cpp //////////////////////////////////////////////////////////////////////
// Created:    Colin Day, September 2001
// Desc:       Collection of function pointers to help us in managing
//						 and assign callbacks
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/FunctionLexicon.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GUICallbacks.h"
#include "GameClient/Gadget.h"

// Popup Ladder Select --------------------------------------------------------------------------
extern void PopupLadderSelectInit( WindowLayout *layout, void *userData );
extern WindowMsgHandledType PopupLadderSelectSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType PopupLadderSelectInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

extern WindowMsgHandledType PopupBuddyNotificationSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// WOL Buddy Overlay Right Click menu callbacks --------------------------------------------------------------
extern void RCGameDetailsMenuInit( WindowLayout *layout, void *userData );
extern WindowMsgHandledType RCGameDetailsMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Beacon control bar callback --------------------------------------------------------------
extern WindowMsgHandledType BeaconWindowInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Popup Replay Save Menu ----------------------------------------------------------------------------------
extern void PopupReplayInit( WindowLayout *layout, void *userData );
extern void PopupReplayUpdate( WindowLayout *layout, void *userData );
extern void PopupReplayShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType PopupReplaySystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType PopupReplayInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Extended MessageBox ----------------------------------------------------------------------------------
extern WindowMsgHandledType ExtendedMessageBoxSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// game window draw table -----------------------------------------------------------------------
constinit static FunctionLexicon::TableEntry gameWinDrawTable[] =
{
	{ NAMEKEY_INVALID, "IMECandidateMainDraw",						IMECandidateMainDraw },
	{ NAMEKEY_INVALID, "IMECandidateTextAreaDraw",				IMECandidateTextAreaDraw },
	{ NAMEKEY_INVALID, NULL,																nullptr }
};

// game window system table -----------------------------------------------------------------------
constinit static FunctionLexicon::TableEntry gameWinSystemTable[] =
{


	{ NAMEKEY_INVALID, "PassSelectedButtonsToParentSystem",	PassSelectedButtonsToParentSystem },
	{ NAMEKEY_INVALID, "PassMessagesToParentSystem",				PassMessagesToParentSystem },

	{ NAMEKEY_INVALID, "GameWinDefaultSystem",							GameWinDefaultSystem },
	{ NAMEKEY_INVALID, "GadgetPushButtonSystem",						GadgetPushButtonSystem },
	{ NAMEKEY_INVALID, "GadgetCheckBoxSystem",							GadgetCheckBoxSystem },
	{ NAMEKEY_INVALID, "GadgetRadioButtonSystem",						GadgetRadioButtonSystem },
	{ NAMEKEY_INVALID, "GadgetTabControlSystem",						GadgetTabControlSystem },
	{ NAMEKEY_INVALID, "GadgetListBoxSystem",								GadgetListBoxSystem },
	{ NAMEKEY_INVALID, "GadgetComboBoxSystem",							GadgetComboBoxSystem },
	{ NAMEKEY_INVALID, "GadgetHorizontalSliderSystem",			GadgetHorizontalSliderSystem },
	{ NAMEKEY_INVALID, "GadgetVerticalSliderSystem",				GadgetVerticalSliderSystem },
	{ NAMEKEY_INVALID, "GadgetProgressBarSystem",						GadgetProgressBarSystem },
	{ NAMEKEY_INVALID, "GadgetStaticTextSystem",						GadgetStaticTextSystem },
	{ NAMEKEY_INVALID, "GadgetTextEntrySystem",							GadgetTextEntrySystem },
	{ NAMEKEY_INVALID, "MessageBoxSystem",									MessageBoxSystem },
	{ NAMEKEY_INVALID, "QuitMessageBoxSystem",							QuitMessageBoxSystem },

	{ NAMEKEY_INVALID, "ExtendedMessageBoxSystem",					ExtendedMessageBoxSystem },

	{ NAMEKEY_INVALID, "MOTDSystem",										MOTDSystem },
	{ NAMEKEY_INVALID, "MainMenuSystem",								MainMenuSystem },
	{ NAMEKEY_INVALID, "OptionsMenuSystem",							OptionsMenuSystem },
	{ NAMEKEY_INVALID, "SinglePlayerMenuSystem",				SinglePlayerMenuSystem },
	{ NAMEKEY_INVALID, "QuitMenuSystem",								QuitMenuSystem },
	{ NAMEKEY_INVALID, "MapSelectMenuSystem",						MapSelectMenuSystem },
	{ NAMEKEY_INVALID, "ReplayMenuSystem",							ReplayMenuSystem },
	{ NAMEKEY_INVALID, "CreditsMenuSystem",							CreditsMenuSystem },
	{ NAMEKEY_INVALID, "LanLobbyMenuSystem",						LanLobbyMenuSystem },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuSystem",			LanGameOptionsMenuSystem },
	{ NAMEKEY_INVALID, "LanMapSelectMenuSystem",				LanMapSelectMenuSystem },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuSystem", SkirmishGameOptionsMenuSystem },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuSystem",   SkirmishMapSelectMenuSystem },
	{ NAMEKEY_INVALID, "ChallengeMenuSystem",			ChallengeMenuSystem },
	{ NAMEKEY_INVALID, "SaveLoadMenuSystem",            SaveLoadMenuSystem },
	{ NAMEKEY_INVALID, "PopupCommunicatorSystem",       PopupCommunicatorSystem },
	{ NAMEKEY_INVALID, "PopupBuddyNotificationSystem",  PopupBuddyNotificationSystem },
	{ NAMEKEY_INVALID, "PopupReplaySystem",							PopupReplaySystem },
	{ NAMEKEY_INVALID, "KeyboardOptionsMenuSystem",     KeyboardOptionsMenuSystem },
	{ NAMEKEY_INVALID, "WOLLadderScreenSystem",			    WOLLadderScreenSystem },
	{ NAMEKEY_INVALID, "WOLLoginMenuSystem",						WOLLoginMenuSystem },
	{ NAMEKEY_INVALID, "WOLLocaleSelectSystem",					WOLLocaleSelectSystem },
	{ NAMEKEY_INVALID, "WOLLobbyMenuSystem",						WOLLobbyMenuSystem },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuSystem",				WOLGameSetupMenuSystem },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuSystem",				WOLMapSelectMenuSystem },
	{ NAMEKEY_INVALID, "WOLBuddyOverlaySystem",					WOLBuddyOverlaySystem },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayRCMenuSystem",		WOLBuddyOverlayRCMenuSystem },
	{ NAMEKEY_INVALID, "RCGameDetailsMenuSystem",				RCGameDetailsMenuSystem },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlaySystem",GameSpyPlayerInfoOverlaySystem },
	{ NAMEKEY_INVALID, "WOLMessageWindowSystem",				WOLMessageWindowSystem },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuSystem",				WOLQuickMatchMenuSystem },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuSystem",					WOLWelcomeMenuSystem },
	{ NAMEKEY_INVALID, "WOLStatusMenuSystem",						WOLStatusMenuSystem },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenSystem",				WOLQMScoreScreenSystem },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenSystem",		WOLCustomScoreScreenSystem },
	{ NAMEKEY_INVALID, "NetworkDirectConnectSystem",		NetworkDirectConnectSystem },
	{ NAMEKEY_INVALID, "PopupHostGameSystem",						PopupHostGameSystem },
	{ NAMEKEY_INVALID, "PopupJoinGameSystem",						PopupJoinGameSystem },
	{ NAMEKEY_INVALID, "PopupLadderSelectSystem",				PopupLadderSelectSystem },
	{ NAMEKEY_INVALID, "InGamePopupMessageSystem",			InGamePopupMessageSystem },
	{ NAMEKEY_INVALID, "ControlBarSystem",							ControlBarSystem },
	{ NAMEKEY_INVALID, "ControlBarObserverSystem",			ControlBarObserverSystem },
	{ NAMEKEY_INVALID, "IMECandidateWindowSystem",			IMECandidateWindowSystem },
	{ NAMEKEY_INVALID, "ReplayControlSystem",						ReplayControlSystem },
	{ NAMEKEY_INVALID, "InGameChatSystem",							InGameChatSystem },
	{ NAMEKEY_INVALID, "DisconnectControlSystem",				DisconnectControlSystem },
	{ NAMEKEY_INVALID, "DiplomacySystem",								DiplomacySystem },
	{ NAMEKEY_INVALID, "GeneralsExpPointsSystem",				GeneralsExpPointsSystem },
	{ NAMEKEY_INVALID, "DifficultySelectSystem",				DifficultySelectSystem },
	
	{ NAMEKEY_INVALID, "IdleWorkerSystem",							IdleWorkerSystem },
	{ NAMEKEY_INVALID, "EstablishConnectionsControlSystem", EstablishConnectionsControlSystem },
	{ NAMEKEY_INVALID, "GameInfoWindowSystem",					GameInfoWindowSystem },
	{ NAMEKEY_INVALID, "ScoreScreenSystem",							ScoreScreenSystem },
	{ NAMEKEY_INVALID, "DownloadMenuSystem",            DownloadMenuSystem },

	{ NAMEKEY_INVALID, NULL,																nullptr }

};

// game window input table ------------------------------------------------------------------------
constinit static FunctionLexicon::TableEntry gameWinInputTable[] =
{

	{ NAMEKEY_INVALID, "GameWinDefaultInput",						GameWinDefaultInput },
	{ NAMEKEY_INVALID, "GameWinBlockInput",							GameWinBlockInput },
	{ NAMEKEY_INVALID, "GadgetPushButtonInput",					GadgetPushButtonInput },
	{ NAMEKEY_INVALID, "GadgetCheckBoxInput",						GadgetCheckBoxInput },
	{ NAMEKEY_INVALID, "GadgetRadioButtonInput",				GadgetRadioButtonInput },
	{ NAMEKEY_INVALID, "GadgetTabControlInput",					GadgetTabControlInput },
	{ NAMEKEY_INVALID, "GadgetListBoxInput",						GadgetListBoxInput },
	{ NAMEKEY_INVALID, "GadgetListBoxMultiInput",				GadgetListBoxMultiInput },
	{ NAMEKEY_INVALID, "GadgetComboBoxInput",						GadgetComboBoxInput },
	{ NAMEKEY_INVALID, "GadgetHorizontalSliderInput",		GadgetHorizontalSliderInput },
	{ NAMEKEY_INVALID, "GadgetVerticalSliderInput",			GadgetVerticalSliderInput },
	{ NAMEKEY_INVALID, "GadgetStaticTextInput",					GadgetStaticTextInput },
	{ NAMEKEY_INVALID, "GadgetTextEntryInput",					GadgetTextEntryInput },

	{ NAMEKEY_INVALID, "MainMenuInput",									MainMenuInput },
	{ NAMEKEY_INVALID, "MapSelectMenuInput",						MapSelectMenuInput },
	{ NAMEKEY_INVALID, "OptionsMenuInput",							OptionsMenuInput },
	{ NAMEKEY_INVALID, "SinglePlayerMenuInput",					SinglePlayerMenuInput },
	{ NAMEKEY_INVALID, "LanLobbyMenuInput",							LanLobbyMenuInput },
	{ NAMEKEY_INVALID, "ReplayMenuInput",								ReplayMenuInput },
	{ NAMEKEY_INVALID, "CreditsMenuInput",								CreditsMenuInput },
	{ NAMEKEY_INVALID, "KeyboardOptionsMenuInput",      KeyboardOptionsMenuInput },
	{ NAMEKEY_INVALID, "PopupCommunicatorInput",        PopupCommunicatorInput },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuInput",				LanGameOptionsMenuInput },
	{ NAMEKEY_INVALID, "LanMapSelectMenuInput",					LanMapSelectMenuInput },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuInput",  SkirmishGameOptionsMenuInput },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuInput",    SkirmishMapSelectMenuInput },
	{ NAMEKEY_INVALID, "ChallengeMenuInput",			ChallengeMenuInput },
	{ NAMEKEY_INVALID, "WOLLadderScreenInput",					WOLLadderScreenInput },
	{ NAMEKEY_INVALID, "WOLLoginMenuInput",							WOLLoginMenuInput },
	{ NAMEKEY_INVALID, "WOLLocaleSelectInput",					WOLLocaleSelectInput },
	{ NAMEKEY_INVALID, "WOLLobbyMenuInput",							WOLLobbyMenuInput },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuInput",					WOLGameSetupMenuInput },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuInput",					WOLMapSelectMenuInput },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayInput",					WOLBuddyOverlayInput },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlayInput",	GameSpyPlayerInfoOverlayInput },
	{ NAMEKEY_INVALID, "WOLMessageWindowInput",					WOLMessageWindowInput },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuInput",				WOLQuickMatchMenuInput },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuInput",						WOLWelcomeMenuInput },
	{ NAMEKEY_INVALID, "WOLStatusMenuInput",						WOLStatusMenuInput },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenInput",					WOLQMScoreScreenInput },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenInput",			WOLCustomScoreScreenInput },
	{ NAMEKEY_INVALID, "NetworkDirectConnectInput",			NetworkDirectConnectInput },
	{ NAMEKEY_INVALID, "PopupHostGameInput",						PopupHostGameInput },
	{ NAMEKEY_INVALID, "PopupJoinGameInput",						PopupJoinGameInput },
	{ NAMEKEY_INVALID, "PopupLadderSelectInput",				PopupLadderSelectInput },
	{ NAMEKEY_INVALID, "InGamePopupMessageInput",				InGamePopupMessageInput },
	{ NAMEKEY_INVALID, "ControlBarInput",								ControlBarInput },
	{ NAMEKEY_INVALID, "ReplayControlInput",						ReplayControlInput },
	{ NAMEKEY_INVALID, "InGameChatInput",								InGameChatInput },
	{ NAMEKEY_INVALID, "DisconnectControlInput",				DisconnectControlInput },
	{ NAMEKEY_INVALID, "DiplomacyInput",								DiplomacyInput },
	{ NAMEKEY_INVALID, "EstablishConnectionsControlInput", EstablishConnectionsControlInput },
	{ NAMEKEY_INVALID, "LeftHUDInput",									LeftHUDInput },
	{ NAMEKEY_INVALID, "ScoreScreenInput",							ScoreScreenInput },
	{ NAMEKEY_INVALID, "SaveLoadMenuInput",							SaveLoadMenuInput },
	{ NAMEKEY_INVALID, "BeaconWindowInput",							BeaconWindowInput },
	{ NAMEKEY_INVALID, "DifficultySelectInput",					DifficultySelectInput },
	{ NAMEKEY_INVALID, "PopupReplayInput",							PopupReplayInput },
	{ NAMEKEY_INVALID, "GeneralsExpPointsInput",				GeneralsExpPointsInput},

	{ NAMEKEY_INVALID, "DownloadMenuInput",							DownloadMenuInput },

	{ NAMEKEY_INVALID, "IMECandidateWindowInput",				IMECandidateWindowInput },
	{ NAMEKEY_INVALID, NULL,														nullptr }

};

// game window tooltip table ----------------------------------------------------------------------
constinit static FunctionLexicon::TableEntry gameWinTooltipTable[] =
{


	{ NAMEKEY_INVALID, "GameWinDefaultTooltip",		GameWinDefaultTooltip },

	{ NAMEKEY_INVALID, NULL,											nullptr }

};

// window layout init table -----------------------------------------------------------------------
constinit static FunctionLexicon::TableEntry winLayoutInitTable[] =
{

	{ NAMEKEY_INVALID, "MainMenuInit",									MainMenuInit },
	{ NAMEKEY_INVALID, "OptionsMenuInit",								OptionsMenuInit },
	{ NAMEKEY_INVALID, "SaveLoadMenuInit",              SaveLoadMenuInit },
	{ NAMEKEY_INVALID, "SaveLoadMenuFullScreenInit",    SaveLoadMenuFullScreenInit },

	{ NAMEKEY_INVALID, "PopupCommunicatorInit",         PopupCommunicatorInit },
	{ NAMEKEY_INVALID, "KeyboardOptionsMenuInit",       KeyboardOptionsMenuInit },
	{ NAMEKEY_INVALID, "SinglePlayerMenuInit",					SinglePlayerMenuInit },
	{ NAMEKEY_INVALID, "MapSelectMenuInit",							MapSelectMenuInit },
	{ NAMEKEY_INVALID, "LanLobbyMenuInit",							LanLobbyMenuInit },
	{ NAMEKEY_INVALID, "ReplayMenuInit",								ReplayMenuInit },
	{ NAMEKEY_INVALID, "CreditsMenuInit",								CreditsMenuInit },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuInit",				LanGameOptionsMenuInit },
	{ NAMEKEY_INVALID, "LanMapSelectMenuInit",					LanMapSelectMenuInit },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuInit",   SkirmishGameOptionsMenuInit },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuInit",     SkirmishMapSelectMenuInit },
	{ NAMEKEY_INVALID, "ChallengeMenuInit",				ChallengeMenuInit },
	{ NAMEKEY_INVALID, "WOLLadderScreenInit",						WOLLadderScreenInit },
	{ NAMEKEY_INVALID, "WOLLoginMenuInit",							WOLLoginMenuInit },
	{ NAMEKEY_INVALID, "WOLLocaleSelectInit",						WOLLocaleSelectInit },
	{ NAMEKEY_INVALID, "WOLLobbyMenuInit",							WOLLobbyMenuInit },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuInit",					WOLGameSetupMenuInit },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuInit",					WOLMapSelectMenuInit },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayInit",						WOLBuddyOverlayInit },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayRCMenuInit",			WOLBuddyOverlayRCMenuInit },
	{ NAMEKEY_INVALID, "RCGameDetailsMenuInit",					RCGameDetailsMenuInit },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlayInit",	GameSpyPlayerInfoOverlayInit },
	{ NAMEKEY_INVALID, "WOLMessageWindowInit",					WOLMessageWindowInit },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuInit",					WOLQuickMatchMenuInit },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuInit",						WOLWelcomeMenuInit },
	{ NAMEKEY_INVALID, "WOLStatusMenuInit",							WOLStatusMenuInit },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenInit",					WOLQMScoreScreenInit },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenInit",			WOLCustomScoreScreenInit },
	{ NAMEKEY_INVALID, "NetworkDirectConnectInit",			NetworkDirectConnectInit },
	{ NAMEKEY_INVALID, "PopupHostGameInit",							PopupHostGameInit },
	{ NAMEKEY_INVALID, "PopupJoinGameInit",							PopupJoinGameInit },
	{ NAMEKEY_INVALID, "PopupLadderSelectInit",					PopupLadderSelectInit },
	{ NAMEKEY_INVALID, "InGamePopupMessageInit",				InGamePopupMessageInit },
	{ NAMEKEY_INVALID, "GameInfoWindowInit",						GameInfoWindowInit },
	{ NAMEKEY_INVALID, "ScoreScreenInit",								ScoreScreenInit },
	{ NAMEKEY_INVALID, "DownloadMenuInit",              DownloadMenuInit },
	{ NAMEKEY_INVALID, "DifficultySelectInit",          DifficultySelectInit },
	{ NAMEKEY_INVALID, "PopupReplayInit",							  PopupReplayInit },

	{ NAMEKEY_INVALID, NULL,														nullptr }  // keep this last

};

// window layout update table ---------------------------------------------------------------------
constinit static FunctionLexicon::TableEntry winLayoutUpdateTable[] =
{

	{ NAMEKEY_INVALID, "MainMenuUpdate",								MainMenuUpdate },
	{ NAMEKEY_INVALID, "OptionsMenuUpdate",							OptionsMenuUpdate },
	{ NAMEKEY_INVALID, "SinglePlayerMenuUpdate",				SinglePlayerMenuUpdate },
	{ NAMEKEY_INVALID, "MapSelectMenuUpdate",						MapSelectMenuUpdate },
	{ NAMEKEY_INVALID, "LanLobbyMenuUpdate",						LanLobbyMenuUpdate },
	{ NAMEKEY_INVALID, "ReplayMenuUpdate",							ReplayMenuUpdate },
	{ NAMEKEY_INVALID, "SaveLoadMenuUpdate",							SaveLoadMenuUpdate },
	
	{ NAMEKEY_INVALID, "CreditsMenuUpdate",							CreditsMenuUpdate },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuUpdate",			LanGameOptionsMenuUpdate },
	{ NAMEKEY_INVALID, "LanMapSelectMenuUpdate",				LanMapSelectMenuUpdate },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuUpdate", SkirmishGameOptionsMenuUpdate },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuUpdate",   SkirmishMapSelectMenuUpdate },
	{ NAMEKEY_INVALID, "ChallengeMenuUpdate",			ChallengeMenuUpdate },
	{ NAMEKEY_INVALID, "WOLLadderScreenUpdate",					WOLLadderScreenUpdate },
	{ NAMEKEY_INVALID, "WOLLoginMenuUpdate",						WOLLoginMenuUpdate },
	{ NAMEKEY_INVALID, "WOLLocaleSelectUpdate",					WOLLocaleSelectUpdate },
	{ NAMEKEY_INVALID, "WOLLobbyMenuUpdate",						WOLLobbyMenuUpdate },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuUpdate",				WOLGameSetupMenuUpdate },
	{ NAMEKEY_INVALID, "PopupHostGameUpdate",						PopupHostGameUpdate },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuUpdate",				WOLMapSelectMenuUpdate },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayUpdate",					WOLBuddyOverlayUpdate },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlayUpdate",GameSpyPlayerInfoOverlayUpdate },
	{ NAMEKEY_INVALID, "WOLMessageWindowUpdate",				WOLMessageWindowUpdate },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuUpdate",				WOLQuickMatchMenuUpdate },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuUpdate",					WOLWelcomeMenuUpdate },
	{ NAMEKEY_INVALID, "WOLStatusMenuUpdate",						WOLStatusMenuUpdate },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenUpdate",				WOLQMScoreScreenUpdate },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenUpdate",		WOLCustomScoreScreenUpdate },
	{ NAMEKEY_INVALID, "NetworkDirectConnectUpdate",		NetworkDirectConnectUpdate },
	{ NAMEKEY_INVALID, "ScoreScreenUpdate",							ScoreScreenUpdate },
	{ NAMEKEY_INVALID, "DownloadMenuUpdate",						DownloadMenuUpdate },
	{ NAMEKEY_INVALID, "PopupReplayUpdate",							PopupReplayUpdate },
	{ NAMEKEY_INVALID, NULL,														nullptr }  // keep this last

};

// window layout shutdown table -------------------------------------------------------------------
constinit static FunctionLexicon::TableEntry winLayoutShutdownTable[] =
{

	{ NAMEKEY_INVALID, "MainMenuShutdown",							MainMenuShutdown },
	{ NAMEKEY_INVALID, "OptionsMenuShutdown",						OptionsMenuShutdown },
	{ NAMEKEY_INVALID, "SaveLoadMenuShutdown",          SaveLoadMenuShutdown },
	{ NAMEKEY_INVALID, "PopupCommunicatorShutdown",     PopupCommunicatorShutdown },      
	{ NAMEKEY_INVALID, "KeyboardOptionsMenuShutdown",   KeyboardOptionsMenuShutdown },
	{ NAMEKEY_INVALID, "SinglePlayerMenuShutdown",			SinglePlayerMenuShutdown },
	{ NAMEKEY_INVALID, "MapSelectMenuShutdown",					MapSelectMenuShutdown },
	{ NAMEKEY_INVALID, "LanLobbyMenuShutdown",					LanLobbyMenuShutdown },
	{ NAMEKEY_INVALID, "ReplayMenuShutdown",						ReplayMenuShutdown },
	{ NAMEKEY_INVALID, "CreditsMenuShutdown",						CreditsMenuShutdown },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuShutdown",		LanGameOptionsMenuShutdown },
	{ NAMEKEY_INVALID, "LanMapSelectMenuShutdown",			LanMapSelectMenuShutdown },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuShutdown",SkirmishGameOptionsMenuShutdown },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuShutdown", SkirmishMapSelectMenuShutdown },
	{ NAMEKEY_INVALID, "ChallengeMenuShutdown",				ChallengeMenuShutdown },
	{ NAMEKEY_INVALID, "WOLLadderScreenShutdown",				WOLLadderScreenShutdown },
	{ NAMEKEY_INVALID, "WOLLoginMenuShutdown",					WOLLoginMenuShutdown },
	{ NAMEKEY_INVALID, "WOLLocaleSelectShutdown",				WOLLocaleSelectShutdown },
	{ NAMEKEY_INVALID, "WOLLobbyMenuShutdown",					WOLLobbyMenuShutdown },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuShutdown",			WOLGameSetupMenuShutdown },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuShutdown",			WOLMapSelectMenuShutdown },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayShutdown",				WOLBuddyOverlayShutdown },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlayShutdown",GameSpyPlayerInfoOverlayShutdown },
	{ NAMEKEY_INVALID, "WOLMessageWindowShutdown",			WOLMessageWindowShutdown },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuShutdown",			WOLQuickMatchMenuShutdown },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuShutdown",				WOLWelcomeMenuShutdown },
	{ NAMEKEY_INVALID, "WOLStatusMenuShutdown",					WOLStatusMenuShutdown },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenShutdown",			WOLQMScoreScreenShutdown },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenShutdown",	WOLCustomScoreScreenShutdown },
	{ NAMEKEY_INVALID, "NetworkDirectConnectShutdown",	NetworkDirectConnectShutdown },
	{ NAMEKEY_INVALID, "ScoreScreenShutdown",						ScoreScreenShutdown },
	{ NAMEKEY_INVALID, "DownloadMenuShutdown",          DownloadMenuShutdown },
	{ NAMEKEY_INVALID, "PopupReplayShutdown",	          PopupReplayShutdown },
	{ NAMEKEY_INVALID, NULL,														nullptr }  // keep this last

};

FunctionLexicon::FunctionLexicon():FunctionLexicon([] {
  static const NativeFunctionTable sources[]{
    {gameWinDrawTable,TABLE_GAME_WIN_DRAW},
    {gameWinSystemTable,TABLE_GAME_WIN_SYSTEM},
    {gameWinInputTable,TABLE_GAME_WIN_INPUT},
    {gameWinTooltipTable,TABLE_GAME_WIN_TOOLTIP},
    {winLayoutInitTable,TABLE_WIN_LAYOUT_INIT},
    {winLayoutUpdateTable,TABLE_WIN_LAYOUT_UPDATE},
    {winLayoutShutdownTable,TABLE_WIN_LAYOUT_SHUTDOWN}};
  return std::span<const NativeFunctionTable>(sources);
}()) {}
