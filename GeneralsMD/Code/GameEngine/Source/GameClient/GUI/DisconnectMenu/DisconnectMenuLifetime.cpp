// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/DisconnectMenu.h"
// Actual source owner: the manager is borrowed, not deleted or called at exit.
DisconnectMenu::DisconnectMenu():m_disconnectManager(nullptr),m_menuState(DISCONNECTMENUSTATETYPE_SCREENOFF){}
DisconnectMenu::~DisconnectMenu(){}
void DisconnectMenu::attachDisconnectManager(DisconnectManager* manager){m_disconnectManager=manager;}
