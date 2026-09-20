#include "PreRTS.h"

#include "Common/FunctionLexicon.h"
#include "GameClient/GameWindowManager.h"

#include <cctype>

// Device-only methods are supplied by the production renderer in the full
// entry point.  Slice 04's callback-free reset harness keeps them inert.
void GameWindowManager::winDrawImage(const Image *, Int, Int, Int, Int, Color) {}
void GameWindowManager::winFillRect(Color, Real, Int, Int, Int, Int) {}
void GameWindowManager::winOpenRect(Color, Real, Int, Int, Int, Int) {}
void GameWindowManager::winDrawLine(Color, Real, Int, Int, Int, Int) {}
Color GameWindowManager::winMakeColor(UnsignedByte red, UnsignedByte green,
	UnsignedByte blue, UnsignedByte alpha)
{
	return GameMakeColor(red, green, blue, alpha);
}
const Image *GameWindowManager::winFindImage(const char *) { return NULL; }
Int GameWindowManager::winFontHeight(GameFont *) { return 0; }
Int GameWindowManager::winIsDigit(Int value) { return std::isdigit(static_cast<unsigned char>(value)); }
Int GameWindowManager::winIsAscii(Int value) { return value >= 0 && value <= 127; }
Int GameWindowManager::winIsAlNum(Int value) { return std::isalnum(static_cast<unsigned char>(value)); }
void GameWindowManager::winFormatText(GameFont *, UnicodeString, Color, Int, Int, Int, Int) {}
void GameWindowManager::winGetTextSize(GameFont *, UnicodeString, Int *width, Int *height, Int)
{
	if (width) *width = 0;
	if (height) *height = 0;
}
GameFont *GameWindowManager::winFindFont(AsciiString, Int, Bool) { return NULL; }
