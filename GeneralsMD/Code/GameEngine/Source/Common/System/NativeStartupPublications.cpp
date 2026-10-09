// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Electronic Arts Inc.
// Actual original borrowed publications, independent of device/menu/transport
// implementation. Their real owners still control creation and retirement.
class NetworkInterface;
class LANAPI;
class IMEManagerInterface;
class DisconnectMenu;
class SkirmishGameInfo;
NetworkInterface *TheNetwork = nullptr;
LANAPI *TheLAN = nullptr;
IMEManagerInterface *TheIMEManager = nullptr;
DisconnectMenu *TheDisconnectMenu = nullptr;
SkirmishGameInfo *TheChallengeGameInfo = nullptr;
SkirmishGameInfo *TheSkirmishGameInfo = nullptr;
