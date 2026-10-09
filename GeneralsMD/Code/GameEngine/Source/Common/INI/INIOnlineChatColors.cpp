// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Electronic Arts Inc.
// Actual source shared color metadata and parser; no online transport dependency.
#include "PreRTS.h"
#include "Common/INI.h"
#include "GameClient/OnlineChatColors.h"
#include <array>
#include <algorithm>
#define OFFSET(x) (sizeof(Int) * (x))
static const FieldParse GameSpyColorFieldParse[] =
{

	{ "Default",						INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_DEFAULT) },
	{ "CurrentRoom",				INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CURRENTROOM) },
	{ "ChatRoom",						INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_ROOM) },
	{ "Game",								INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_GAME) },
	{ "GameFull",						INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_GAME_FULL) },
	{ "GameCRCMismatch",		INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_GAME_CRCMISMATCH) },
	{ "PlayerNormal",				INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_PLAYER_NORMAL) },
	{ "PlayerOwner",				INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_PLAYER_OWNER) },
	{ "PlayerBuddy",				INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_PLAYER_BUDDY) },
	{ "PlayerSelf",					INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_PLAYER_SELF) },
	{ "PlayerIgnored",			INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_PLAYER_IGNORED) },
	{ "ChatNormal",					INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_NORMAL) },
	{ "ChatEmote",					INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_EMOTE) },
	{ "ChatOwner",					INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_OWNER) },
	{ "ChatOwnerEmote",			INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_OWNER_EMOTE) },
	{ "ChatPriv",						INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_PRIVATE) },
	{ "ChatPrivEmote",			INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_PRIVATE_EMOTE) },
	{ "ChatPrivOwner",			INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_PRIVATE_OWNER) },
	{ "ChatPrivOwnerEmote",	INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_PRIVATE_OWNER_EMOTE) },
	{ "ChatBuddy",					INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_BUDDY) },
	{ "ChatSelf",						INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_SELF) },
	{ "AcceptTrue",					INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_ACCEPT_TRUE) },
	{ "AcceptFalse",				INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_ACCEPT_FALSE) },
	{ "MapSelected",				INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_MAP_SELECTED) },
	{ "MapUnselected",			INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_MAP_UNSELECTED) },
	{ "MOTD",								INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_MOTD) },
	{ "MOTDHeading",				INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_MOTD_HEADING) },

	{ NULL,					NULL,						NULL,						0 }  // keep this last

};

void INI::parseOnlineChatColorDefinition( INI* ini )
{
	// parse the ini definition
	if (!ini) throw ERROR_BAD_ARG;
    std::array<Color,GSCOLOR_MAX> candidate;
    std::copy_n(GameSpyColor,GSCOLOR_MAX,candidate.begin());
    ini->initFromINI(candidate.data(),GameSpyColorFieldParse);
    std::copy(candidate.begin(),candidate.end(),GameSpyColor);
}


Color GameSpyColor[GSCOLOR_MAX] =
{
	GameMakeColor(255,255,255,255),	// GSCOLOR_DEFAULT
	GameMakeColor(255,255,  0,255),	// GSCOLOR_CURRENTROOM
	GameMakeColor(255,255,255,255),	// GSCOLOR_ROOM
	GameMakeColor(128,128,0,255),		// GSCOLOR_GAME
	GameMakeColor(128,128,128,255),	// GSCOLOR_GAME_FULL
	GameMakeColor(128,128,128,255),	// GSCOLOR_GAME_CRCMISMATCH
	GameMakeColor(255,255,255,255),	// GSCOLOR_PLAYER_NORMAL
	GameMakeColor(255,  0,255,255),	// GSCOLOR_PLAYER_OWNER
	GameMakeColor(255,  0,128,255),	// GSCOLOR_PLAYER_BUDDY
	GameMakeColor(255,  0,  0,255),	// GSCOLOR_PLAYER_SELF
	GameMakeColor(128,128,128,255),	// GSCOLOR_PLAYER_IGNORED
	GameMakeColor(255,255,255,255),		// GSCOLOR_CHAT_NORMAL
	GameMakeColor(255,128,0,255),		// GSCOLOR_CHAT_EMOTE,
	GameMakeColor(255,255,0,255),		// GSCOLOR_CHAT_OWNER,
	GameMakeColor(128,255,0,255),		// GSCOLOR_CHAT_OWNER_EMOTE,
	GameMakeColor(0,0,255,255),			// GSCOLOR_CHAT_PRIVATE,
	GameMakeColor(0,255,255,255),		// GSCOLOR_CHAT_PRIVATE_EMOTE,
	GameMakeColor(255,0,255,255),		// GSCOLOR_CHAT_PRIVATE_OWNER,
	GameMakeColor(255,128,255,255),	// GSCOLOR_CHAT_PRIVATE_OWNER_EMOTE,
	GameMakeColor(255,  0,255,255),	// GSCOLOR_CHAT_BUDDY,
	GameMakeColor(255,  0,128,255),	// GSCOLOR_CHAT_SELF,
	GameMakeColor(  0,255,  0,255),	// GSCOLOR_ACCEPT_TRUE,
	GameMakeColor(255,  0,  0,255),	// GSCOLOR_ACCEPT_FALSE,
	GameMakeColor(255,255,  0,255),	// GSCOLOR_MAP_SELECTED,
	GameMakeColor(255,255,255,255),	// GSCOLOR_MAP_UNSELECTED,
	GameMakeColor(255,255,255,255),	// GSCOLOR_MOTD,
	GameMakeColor(255,255,  0,255),	// GSCOLOR_MOTD_HEADING,
};
