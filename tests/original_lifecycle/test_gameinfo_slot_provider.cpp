#include "PreRTS.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "GameClient/GameText.h"
#include "GameNetwork/GameInfo.h"
#include <cstdio>

namespace { int failures; void check(bool ok, const char *m) { if (!ok) { std::fprintf(stderr, "FAIL: %s\n", m); ++failures; } } }
class GeneratedGameText final : public GameTextInterface {
public:
	void init() override {} void reset() override {} void update() override {}
	UnicodeString fetch(const Char *label, Bool *exists = NULL) override { if (exists) *exists = TRUE; UnicodeString value; value.translate(AsciiString(label ? label : "")); return value; }
	UnicodeString fetch(AsciiString label, Bool *exists = NULL) override { return fetch(label.str(), exists); }
	AsciiStringVec &getStringsWithLabelPrefix(AsciiString) override { return strings; }
	void initMapStringFile(const AsciiString &) override {}
private: AsciiStringVec strings;
};
int main()
{
	for (int generation = 0; generation != 2; ++generation)
	{
		{
			FileSystem files;
			TheFileSystem = &files;
			GlobalData globalData;
			TheWritableGlobalData = &globalData;
			GeneratedGameText gameText;
			TheGameText = &gameText;
			{
			GameInfo game; game.init();
			TheGameInfo = &game;
			game.setLocalIP(0x7f000001);
			game.enterGame();
			GameSlot human; human.setState(SLOT_PLAYER); human.setIP(0x7f000001); human.setPlayerTemplate(0); human.setColor(2); human.setTeamNumber(1);
			UnicodeString name; name.translate(AsciiString("GeneratedHuman")); human.setName(name);
			GameSlot ai; ai.setState(SLOT_EASY_AI); ai.setPlayerTemplate(1); ai.setColor(3);
			GameSlot ownedHuman, ownedAI;
			game.setSlotPointer(0, &ownedHuman); game.setSlotPointer(1, &ownedAI);
			game.setSlot(0, human); game.setSlot(1, ai);
			check(game.getSlot(0) && game.getSlot(0)->isHuman() && game.getSlot(1) && game.getSlot(1)->isAI(), "generated source slots did not retain human/AI state");
			check(game.getLocalSlotNum() == 0, "source local-slot selection changed");
			check(game.getNumPlayers() == 2 && game.getSlot(-1) == NULL && game.getSlot(MAX_SLOTS) == NULL, "source slot counting or invalid lookup changed");
			game.clearSlotList(); check(game.getNumPlayers() == 0, "source slot clear retained generated ownership");
			game.reset();
			TheGameInfo = NULL;
			}
			TheWritableGlobalData = NULL;
			TheGameText = NULL;
		}
		TheFileSystem = NULL;
	}
	std::puts("original generated game-info slots: generations=2 slots=0");
	return failures ? 1 : 0;
}
