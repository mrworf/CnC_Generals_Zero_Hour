#include "PreRTS.h"
#include "Common/FileSystem.h"
#include "Common/Errors.h"
#include "Common/GameEngine.h"
#include "Common/INI.h"
#include "Common/INIException.h"
#include "Common/LocalFileSystem.h"
#include "GameNetwork/GameSpy/GameSpyColors.h"
#include "PosixDevice/Common/PosixLocalFileSystem.h"

#include <cstdio>
#include <cstring>

// The dispatch-only harness does not construct the production engine.  Live
// original callbacks still reference its canonical singleton, so provide the
// same absent-service state that exists before GameMain creates the engine.
GameEngine *TheGameEngine = NULL;

int main(int argc, char **argv)
{
	if (argc != 3)
		return 2;
	initMemoryManager();
	PosixLocalFileSystem localFiles;
	FileSystem files;
	TheLocalFileSystem = &localFiles;
	TheFileSystem = &files;

	bool rejected = false;
	try
	{
		INI ini;
		ini.load(AsciiString(argv[2]), INI_LOAD_OVERWRITE, NULL);
	}
	catch (const INIException& error)
	{
		std::fprintf(stderr, "%s", error.mFailureMessage ? error.mFailureMessage : "INI exception\n");
		rejected = true;
	}
	catch (Int error)
	{
		std::fprintf(stderr, "INI error: %d\n", error);
		rejected = true;
	}
	catch (ErrorCode error)
	{
		std::fprintf(stderr, "INI error code: %d\n", static_cast<Int>(error));
		rejected = true;
	}
	catch (...)
	{
		std::fprintf(stderr, "unknown INI exception\n");
		rejected = true;
	}

	const bool expectFailure = std::strcmp(argv[1], "invalid") == 0;
	if (rejected != expectFailure)
		return 3;
	if (!expectFailure && GameSpyColor[GSCOLOR_DEFAULT] != GameMakeColor(1, 2, 3, 4))
		return 4;
	std::printf("original-config dispatch: %s\n", expectFailure ? "rejected" : "ok");
	return 0;
}
