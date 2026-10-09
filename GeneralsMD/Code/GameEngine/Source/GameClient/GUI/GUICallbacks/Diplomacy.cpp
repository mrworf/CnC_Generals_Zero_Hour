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

// FILE: Diplomacy.cpp ///////////////////////////////////////////////////////////////////////
// Author: Matthew D. Campbell - August 2002
// Desc: GUI callbacks for the diplomacy menu
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GlobalData.h"
#include "Common/MultiplayerSettings.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/PlayerTemplate.h"
#include "Common/Recorder.h"
#include "GameClient/AnimateWindowManager.h"
#include "GameClient/Diplomacy.h"
#include "GameClient/NativeDiplomacyBriefing.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/WindowLayout.h"
#include <memory>
#include <vector>
#include "GameClient/DisconnectMenu.h"
#include "GameClient/GameWindow.h"
#include "GameClient/Gadget.h"
#include "GameClient/GadgetCheckBox.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GadgetRadioButton.h"
#include "GameClient/GameClient.h"
#include "GameClient/GameText.h"
#include "GameClient/GUICallbacks.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/VictoryConditions.h"
#include "GameNetwork/GameInfo.h"
#include "GameNetwork/NetworkInterface.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//-------------------------------------------------------------------------------------------------

static NameKeyType staticTextPlayerID[MAX_SLOTS];
static NameKeyType staticTextSideID[MAX_SLOTS];
static NameKeyType staticTextTeamID[MAX_SLOTS];
static NameKeyType staticTextStatusID[MAX_SLOTS];
static NameKeyType buttonMuteID[MAX_SLOTS];
static NameKeyType buttonUnMuteID[MAX_SLOTS];
static NameKeyType radioButtonInGameID = NAMEKEY_INVALID;
static NameKeyType radioButtonBuddiesID = NAMEKEY_INVALID;
static GameWindow *radioButtonInGame = NULL;
static GameWindow *radioButtonBuddies = NULL;
static NameKeyType winInGameID = NAMEKEY_INVALID;
static NameKeyType winBuddiesID = NAMEKEY_INVALID;
static NameKeyType winSoloID = NAMEKEY_INVALID;
static GameWindow *winInGame = NULL;
static GameWindow *winBuddies = NULL;
static GameWindow *winSolo = NULL;
static GameWindow *staticTextPlayer[MAX_SLOTS] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static GameWindow *staticTextSide[MAX_SLOTS] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static GameWindow *staticTextTeam[MAX_SLOTS] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static GameWindow *staticTextStatus[MAX_SLOTS] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static GameWindow *buttonMute[MAX_SLOTS] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static GameWindow *buttonUnMute[MAX_SLOTS] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static Int slotNumInRow[MAX_SLOTS];

//-------------------------------------------------------------------------------------------------

static WindowLayout *theLayout = NULL;
static GameWindow *theWindow = NULL;
static AnimateWindowManager *theAnimateWindowManager = NULL;
static InGameUI* layoutUI=nullptr;
static GameWindowManager* layoutManager=nullptr;
struct BriefingView {
  GameWindow* listbox=nullptr;
  InGameUI* ui=nullptr;
  GameWindowManager* manager=nullptr;
  GameTextInterface* text=nullptr;
};
static BriefingView briefingView;
struct ParentPublication {
  InGameUI* ui=TheInGameUI;
  GameWindowManager* manager=TheWindowManager;
  ParentPublication(InGameUI* ownerUI,GameWindowManager* ownerManager){
    TheInGameUI=ownerUI;TheWindowManager=ownerManager;
  }
  ~ParentPublication(){TheInGameUI=ui;TheWindowManager=manager;}
};
static void presentBriefing(const BriefingList& entries,Bool reset,void* context){
  auto& view=*static_cast<BriefingView*>(context);
  if(!view.listbox || !view.ui || !view.manager || !view.text)throw ERROR_BAD_ARG;
  std::vector<UnicodeString> translated;
  if(reset){translated.reserve(entries.size());for(const auto& label:entries)translated.push_back(view.text->fetch(label));}
  else if(!entries.empty())translated.push_back(view.text->fetch(entries.back()));
  ParentPublication parents(view.ui,view.manager);
  if(reset)GadgetListBoxReset(view.listbox);
  for(const auto& text:translated){
    const Int index=GadgetListBoxGetNumEntries(view.listbox);if(index<0)throw ERROR_BAD_ARG;
    GadgetListBoxAddEntryText(view.listbox,text,view.ui->getMessageColor(index%2),-1);
  }
}
struct LayoutRetire {
  InGameUI* ui;GameWindowManager* manager;
  void operator()(WindowLayout* layout) const noexcept {
    if(!layout)return;
    ParentPublication parents(ui,manager);
    layout->setUpdate(nullptr);layout->destroyWindows();manager->retireDestroyedWindows();layout->deleteInstance();
  }
};
static void grabWindowPointers( void )
{
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		AsciiString temp;
		temp.format("Diplomacy.wnd:StaticTextPlayer%d", i);
		staticTextPlayerID[i] = NAMEKEY(temp);
		temp.format("Diplomacy.wnd:StaticTextSide%d", i);
		staticTextSideID[i] = NAMEKEY(temp);
		temp.format("Diplomacy.wnd:StaticTextTeam%d", i);
		staticTextTeamID[i] = NAMEKEY(temp);
		temp.format("Diplomacy.wnd:StaticTextStatus%d", i);
		staticTextStatusID[i] = NAMEKEY(temp);
		temp.format("Diplomacy.wnd:ButtonMute%d", i);
		buttonMuteID[i] = NAMEKEY(temp);
		temp.format("Diplomacy.wnd:ButtonUnMute%d", i);
		buttonUnMuteID[i] = NAMEKEY(temp);

		staticTextPlayer[i] = TheWindowManager->winGetWindowFromId(theWindow, staticTextPlayerID[i]);
		staticTextSide[i] = TheWindowManager->winGetWindowFromId(theWindow, staticTextSideID[i]);
		staticTextTeam[i] = TheWindowManager->winGetWindowFromId(theWindow, staticTextTeamID[i]);
		staticTextStatus[i] = TheWindowManager->winGetWindowFromId(theWindow, staticTextStatusID[i]);
		buttonMute[i] = TheWindowManager->winGetWindowFromId(theWindow, buttonMuteID[i]);
		buttonUnMute[i] = TheWindowManager->winGetWindowFromId(theWindow, buttonUnMuteID[i]);

		slotNumInRow[i] = -1;
	}
}

static void releaseWindowPointers( void )
{
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		staticTextPlayer[i] = NULL;
		staticTextSide[i] = NULL;
		staticTextTeam[i] = NULL;
		staticTextStatus[i] = NULL;
		buttonMute[i] = NULL;
		buttonUnMute[i] = NULL;

		slotNumInRow[i] = -1;
	}
}


//-------------------------------------------------------------------------------------------------

static void updateFunc( WindowLayout *layout, void *param )
{
	if (theWindow && theAnimateWindowManager && TheGlobalData && TheGlobalData->m_animateWindows)
	{
		Bool wasFinished = theAnimateWindowManager->isFinished();
		theAnimateWindowManager->update();
		if (theAnimateWindowManager->isFinished() && !wasFinished && theAnimateWindowManager->isReversed())
			theWindow->winHide( TRUE );
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ShowDiplomacy( Bool immediate )
{
  if(!TheInGameUI || !TheGameLogic || !TheGlobalData)throw ERROR_BAD_ARG;
  if(!TheInGameUI->getInputEnabled() || TheGameLogic->isIntroMoviePlaying() ||
      TheGameLogic->isLoadingMap() || TheInGameUI->isQuitMenuVisible() ||
      (TheDisconnectMenu && TheDisconnectMenu->isScreenVisible()))return;
  if(!TheWindowManager || !TheNameKeyGenerator || !TheRecorder || !TheGameText)throw ERROR_BAD_ARG;
  if(theLayout && (layoutUI!=TheInGameUI || layoutManager!=TheWindowManager ||
      (briefingView.text && briefingView.text!=TheGameText)))throw ERROR_BAD_ARG;
  try {
    if(!theLayout){
      std::unique_ptr<WindowLayout,LayoutRetire> candidate(
          TheWindowManager->winCreateLayout("Diplomacy.wnd"),LayoutRetire{TheInGameUI,TheWindowManager});
      if(!candidate || !candidate->getFirstWindow())throw ERROR_BAD_ARG;
      auto animation=std::make_unique<AnimateWindowManager>();
      layoutUI=TheInGameUI;layoutManager=TheWindowManager;
      theWindow=candidate->getFirstWindow();theLayout=candidate.release();
      theAnimateWindowManager=animation.release();theLayout->setUpdate(updateFunc);
      radioButtonInGameID=NAMEKEY("Diplomacy.wnd:RadioButtonInGame");
      radioButtonBuddiesID=NAMEKEY("Diplomacy.wnd:RadioButtonBuddies");
      winInGameID=NAMEKEY("Diplomacy.wnd:InGameParent");
      winBuddiesID=NAMEKEY("Diplomacy.wnd:BuddiesParent");
      winSoloID=NAMEKEY("Diplomacy.wnd:SoloParent");
      radioButtonInGame=TheWindowManager->winGetWindowFromId(theWindow,radioButtonInGameID);
      radioButtonBuddies=TheWindowManager->winGetWindowFromId(theWindow,radioButtonBuddiesID);
      winInGame=TheWindowManager->winGetWindowFromId(theWindow,winInGameID);
      winBuddies=TheWindowManager->winGetWindowFromId(theWindow,winBuddiesID);
      winSolo=TheWindowManager->winGetWindowFromId(theWindow,winSoloID);
      if(!radioButtonInGame || !radioButtonBuddies || !winInGame || !winBuddies || !winSolo)throw ERROR_BAD_ARG;
      auto* listbox=TheWindowManager->winGetWindowFromId(theWindow,NAMEKEY("Diplomacy.wnd:ListboxSolo"));
      if(listbox){
        briefingView={listbox,layoutUI,layoutManager,TheGameText};
        originalDiplomacyBriefing().attach(presentBriefing,&briefingView);
      }
    }
    theWindow->winHide(FALSE);theWindow->winEnable(TRUE);theLayout->hide(FALSE);
    // The excluded Internet buddy service is not a functioning native feature.
    radioButtonInGame->winHide(TRUE);radioButtonBuddies->winHide(TRUE);
    GadgetRadioSetSelection(radioButtonInGame,FALSE);
    winInGame->winHide(!TheRecorder->isMultiplayer());winBuddies->winHide(TRUE);
    winSolo->winHide(TheRecorder->isMultiplayer());
    theAnimateWindowManager->reset();
    if(!immediate && TheGlobalData->m_animateWindows)
      theAnimateWindowManager->registerGameWindow(theWindow,WIN_ANIMATION_SLIDE_TOP,TRUE,200);
    TheInGameUI->registerWindowLayout(theLayout);grabWindowPointers();PopulateInGameDiplomacyPopup();
  } catch(...) {
    ResetDiplomacy();throw;
  }
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void ResetDiplomacy( void )
{
  if(!originalDiplomacyBriefing().detach())throw ERROR_BAD_ARG;
  auto* layout=theLayout;auto* animation=theAnimateWindowManager;
  auto* ownerUI=layoutUI;auto* ownerManager=layoutManager;
  theLayout=nullptr;theWindow=nullptr;theAnimateWindowManager=nullptr;
  layoutUI=nullptr;layoutManager=nullptr;briefingView={};
  releaseWindowPointers();
  radioButtonInGame=nullptr;radioButtonBuddies=nullptr;
  winInGame=nullptr;winBuddies=nullptr;winSolo=nullptr;
  radioButtonInGameID=radioButtonBuddiesID=winInGameID=winBuddiesID=winSoloID=NAMEKEY_INVALID;
  for(Int i=0;i<MAX_SLOTS;++i){
    staticTextPlayerID[i]=staticTextSideID[i]=staticTextTeamID[i]=staticTextStatusID[i]=NAMEKEY_INVALID;
    buttonMuteID[i]=buttonUnMuteID[i]=NAMEKEY_INVALID;
  }
  delete animation;
  if(layout){
    if(!ownerUI || !ownerManager)throw ERROR_BAD_ARG;
    ParentPublication parents(ownerUI,ownerManager);
    ownerUI->unregisterWindowLayout(layout);
    LayoutRetire{ownerUI,ownerManager}(layout);
  }
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void HideDiplomacy( Bool immediate )
{
	releaseWindowPointers();
	if (theWindow)
	{
		if (immediate || !TheGlobalData || !TheGlobalData->m_animateWindows)
		{
			theWindow->winHide(TRUE);
			theWindow->winEnable(FALSE);
		}
		else
		{
			if (theAnimateWindowManager && theAnimateWindowManager->isFinished())
				theAnimateWindowManager->reverseAnimateWindow();
		}
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void ToggleDiplomacy( Bool immediate )
{
	// If we bring this up, let's hide the quit menu
	HideQuitMenu();

	if (theWindow)
	{
		Bool show = theWindow->winIsHidden();
		if (show)
			ShowDiplomacy( immediate );
		else
			HideDiplomacy( immediate );
	}
	else
	{
		ShowDiplomacy( immediate );
	}
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType DiplomacyInput( GameWindow *window, UnsignedInt msg,
																			WindowMsgData mData1, WindowMsgData mData2 )
{

	switch( msg ) 
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CHAR:
		{
			UnsignedByte key = mData1;
//			UnsignedByte state = mData2;

			switch( key )
			{

				// ----------------------------------------------------------------------------------------
				case KEY_ESC:
				{
					HideDiplomacy();
					return MSG_HANDLED;
					//return MSG_IGNORED;
				}  // end escape

			}  // end switch( key )

			return MSG_HANDLED;

		}  // end char

	}

	return MSG_IGNORED;

}  // end DiplomacyInput

//-------------------------------------------------------------------------------------------------
WindowMsgHandledType DiplomacySystem( GameWindow *window, UnsignedInt msg, 
																			 WindowMsgData mData1, WindowMsgData mData2 )
{
	// GameSpy/Internet buddy service replacement is outside this port's scope.
	switch( msg ) 
	{
		//---------------------------------------------------------------------------------------------
		case GGM_FOCUS_CHANGE:
		{
//			Bool focus = (Bool) mData1;
			//if (focus)
				//TheWindowManager->winSetGrabWindow( chatTextEntry );
			break;
		} // end focus change

		//---------------------------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:
		{	
			// if we're given the opportunity to take the keyboard focus we must say we don't want it
			if( mData1 == TRUE && mData2 )
				*(Bool *)mData2 = FALSE;

			return MSG_HANDLED;
		}//case GWM_INPUT_FOCUS:

		//---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{
      if(!theWindow)return MSG_IGNORED;
			GameWindow *control = (GameWindow *)mData1;
			if(!control)return MSG_IGNORED;
			NameKeyType controlID = (NameKeyType)control->winGetWindowId();
			static const StaticNameKey nativeCached_buttonHideID("Diplomacy.wnd:ButtonHide");
			NameKeyType buttonHideID = nativeCached_buttonHideID.key();
			if (controlID == buttonHideID)
			{
				HideDiplomacy( FALSE );
			}
			else if( controlID == radioButtonInGameID && winInGame && winBuddies)
			{
				winInGame->winHide(FALSE);
				winBuddies->winHide(TRUE);
			}
      else if(controlID==radioButtonBuddiesID)return MSG_IGNORED;

			for (Int i=0; i<MAX_SLOTS; ++i)
			{
				if (controlID == buttonMuteID[i] && TheGameInfo && slotNumInRow[i] >= 0 &&
              slotNumInRow[i]<MAX_SLOTS && TheGameInfo->getSlot(slotNumInRow[i]))
				{
					TheGameInfo->getSlot(slotNumInRow[i])->mute(TRUE);
					PopulateInGameDiplomacyPopup();
					break;
				}
				if (controlID == buttonUnMuteID[i] && TheGameInfo && slotNumInRow[i] >= 0 &&
              slotNumInRow[i]<MAX_SLOTS && TheGameInfo->getSlot(slotNumInRow[i]))
				{
					TheGameInfo->getSlot(slotNumInRow[i])->mute(FALSE);
					PopulateInGameDiplomacyPopup();
					break;
				}
			}
			break;

		}  // end button selected

		//---------------------------------------------------------------------------------------------
		default:
			return MSG_IGNORED;

	}  // end switch( msg )

	return MSG_HANDLED;

}  // end DiplomacySystem

void PopulateInGameDiplomacyPopup( void )
{
	if (!theWindow || !TheGameInfo)
		return;
  if(!ThePlayerList || !TheVictoryConditions || !TheMultiplayerSettings || !TheGameText || !TheNameKeyGenerator)
    throw ERROR_BAD_ARG;

	Int rowNum = 0;
	for (Int slotNum=0; slotNum<MAX_SLOTS; ++slotNum)
	{
		const GameSlot *slot = TheGameInfo->getConstSlot(slotNum);
		if (slot && slot->isOccupied())
		{
			Bool isInGame = false;
			// Note - for skirmish, TheNetwork == NULL.  jba.
			if (TheNetwork &&	TheNetwork->isPlayerConnected(slotNum)) {
				isInGame = true;
			} else if ((TheNetwork == NULL) && slot->isHuman()) {
				// this is a skirmish game and it is the human player.
				isInGame = true;
			}
			if (slot->isAI())
				isInGame = true;
			AsciiString playerName;
			playerName.format("player%d", slotNum);
			Player *player = ThePlayerList->findPlayerWithNameKey(NAMEKEY(playerName));
      if(!player)throw ERROR_BAD_ARG;
			Bool isAlive = !TheVictoryConditions->hasSinglePlayerBeenDefeated(player);
			Bool isObserver = player->isPlayerObserver();

			if (slot->isHuman() && TheGameInfo->getLocalSlotNum() != slotNum && isInGame)
			{
				// show mute button
				if (buttonMute[rowNum])
				{
					buttonMute[rowNum]->winHide(slot->isMuted());
				}
				if (buttonUnMute[rowNum])
				{
					buttonUnMute[rowNum]->winHide(!slot->isMuted());
				}
			}
			else
			{
				// can't mute self, AI players, or MIA humans
				if (buttonMute[rowNum])
					buttonMute[rowNum]->winHide(TRUE);
				if (buttonUnMute[rowNum])
					buttonUnMute[rowNum]->winHide(TRUE);
			}

      const Int colorIndex=slot->getOriginalPlayerTemplate()==PLAYERTEMPLATE_OBSERVER?
          PLAYERTEMPLATE_OBSERVER:slot->getApparentColor();
      auto* color=TheMultiplayerSettings->getColor(colorIndex);if(!color)throw ERROR_BAD_ARG;
			Color playerColor = color->getColor();
			Color backColor = GameMakeColor(0, 0, 0, 255);
			Color aliveColor = GameMakeColor(0, 255, 0, 255);
			Color deadColor = GameMakeColor(255, 0, 0, 255);
			Color observerInGameColor = GameMakeColor(255, 255, 255, 255);
			Color goneColor = GameMakeColor(196, 0, 0, 255);
			Color observerGoneColor = GameMakeColor(196, 196, 196, 255);

			if (staticTextPlayer[rowNum])
			{
        staticTextPlayer[rowNum]->winHide(FALSE);
				staticTextPlayer[rowNum]->winSetEnabledTextColors( playerColor, backColor );
				GadgetStaticTextSetText(staticTextPlayer[rowNum], slot->getName());
			}
			if (staticTextSide[rowNum])
			{
        staticTextSide[rowNum]->winHide(FALSE);
				staticTextSide[rowNum]->winSetEnabledTextColors( playerColor, backColor );
				GadgetStaticTextSetText(staticTextSide[rowNum], slot->getApparentPlayerTemplateDisplayName() );
			}
			if (staticTextTeam[rowNum])
			{
        staticTextTeam[rowNum]->winHide(FALSE);
				staticTextTeam[rowNum]->winSetEnabledTextColors( playerColor, backColor );
				AsciiString teamStr;
        if(slot->getTeamNumber()==std::numeric_limits<Int>::max())throw ERROR_BAD_ARG;
				teamStr.format("Team:%d", slot->getTeamNumber() + 1);
				if (slot->isAI() && slot->getTeamNumber() == -1)
					teamStr = "Team:AI";
				GadgetStaticTextSetText(staticTextTeam[rowNum], TheGameText->fetch(teamStr) );
			}
			if (staticTextStatus[rowNum])
			{
				staticTextStatus[rowNum]->winHide(FALSE);
				if (isInGame)
				{
					if (isAlive)
					{
						staticTextStatus[rowNum]->winSetEnabledTextColors( aliveColor, backColor );
						GadgetStaticTextSetText(staticTextStatus[rowNum], TheGameText->fetch("GUI:PlayerAlive"));
					}
					else
					{
						if (isObserver)
						{
							staticTextStatus[rowNum]->winSetEnabledTextColors( observerInGameColor, backColor );
							GadgetStaticTextSetText(staticTextStatus[rowNum], TheGameText->fetch("GUI:PlayerObserver"));
						}
						else
						{
							staticTextStatus[rowNum]->winSetEnabledTextColors( deadColor, backColor );
							GadgetStaticTextSetText(staticTextStatus[rowNum], TheGameText->fetch("GUI:PlayerDead"));
						}
					}
				}
				else
				{
					// not in game
					if (isObserver)
					{
						staticTextStatus[rowNum]->winSetEnabledTextColors( observerGoneColor, backColor );
						GadgetStaticTextSetText(staticTextStatus[rowNum], TheGameText->fetch("GUI:PlayerObserverGone"));
					}
					else
					{
						staticTextStatus[rowNum]->winSetEnabledTextColors( goneColor, backColor );
						GadgetStaticTextSetText(staticTextStatus[rowNum], TheGameText->fetch("GUI:PlayerGone"));
					}
				}
			}

			slotNumInRow[rowNum++] = slotNum;
		}
	}

	while (rowNum < MAX_SLOTS)
	{
		slotNumInRow[rowNum] = -1;
		if (staticTextPlayer[rowNum])
			staticTextPlayer[rowNum]->winHide(TRUE);
		if (staticTextSide[rowNum])
			staticTextSide[rowNum]->winHide(TRUE);
		if (staticTextTeam[rowNum])
			staticTextTeam[rowNum]->winHide(TRUE);
		if (staticTextStatus[rowNum])
			staticTextStatus[rowNum]->winHide(TRUE);
		if (buttonMute[rowNum])
			buttonMute[rowNum]->winHide(TRUE);
		if (buttonUnMute[rowNum])
			buttonUnMute[rowNum]->winHide(TRUE);

		++rowNum;
	}
}

