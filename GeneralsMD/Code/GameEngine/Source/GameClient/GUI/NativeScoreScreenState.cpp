// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Electronic Arts Inc.
// Actual source score-screen state and fixup, independent of online transport.
#include "PreRTS.h"
#include "GameClient/WindowLayout.h"
WindowLayout *s_blankLayout = nullptr;
void FixupScoreScreenMovieWindow( void )
{
	if (s_blankLayout)
	{
		s_blankLayout->hide(FALSE);
		s_blankLayout->bringForward();
	}
}
