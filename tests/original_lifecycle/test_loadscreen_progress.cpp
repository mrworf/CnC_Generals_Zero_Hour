#include "PreRTS.h"

// This probe is intentionally test-only. The source factory is private, and
// the Linux contract must be observed directly rather than replaced with a
// synthetic owner.
#define private public
#include "GameLogic/GameLogic.h"
#undef private

#include "zh/original_process.h"

#include <cstdio>

namespace {
int failures;

void check(bool condition, const char *message)
{
	if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}
}

int main()
{
	char diagnostic[128]{};
	if (!zh::original_process::initialize_services(0, diagnostic, sizeof(diagnostic)))
	{
		std::fprintf(stderr, "%s\n", diagnostic);
		return 1;
	}
	initMemoryManager();

	const Int modes[] = {
		GAME_NONE, GAME_SHELL, GAME_SINGLE_PLAYER, GAME_SKIRMISH,
		GAME_LAN, GAME_REPLAY, GAME_INTERNET,
	};
	for (Int generation = 0; generation != 2; ++generation)
	{
		GameLogic logic;
		check(logic.m_loadScreen == NULL,
			"new generated progress generation retained an owner");
		for (Int mode : modes)
		{
			logic.setGameMode(mode);
			for (Bool loadingSaveGame : { FALSE, TRUE })
			{
				check(logic.getLoadScreen(loadingSaveGame) == NULL,
					"Linux source factory admitted an unavailable load-screen owner");
				logic.updateLoadProgress(1);
				check(logic.m_loadScreen == NULL,
					"no-owner progress dispatch retained a source owner");
				logic.deleteLoadScreen();
				check(logic.m_loadScreen == NULL,
					"no-owner deletion retained a source owner");
			}
		}
	}

	shutdownMemoryManager();
	zh::original_process::shutdown_services();
	std::printf("M22 original loadscreen progress: %s generations=2 modes=7 owners=0 windows=0 devices=0\n",
		failures ? "failed" : "ok");
	return failures ? 1 : 0;
}
