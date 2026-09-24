#include "PreRTS.h"
#include "Common/GlobalData.h"
#include "Common/FileSystem.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/GameText.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/Gadget.h"
#include "GameClient/MapUtil.h"
#include "GameNetwork/GameInfo.h"
#include "PosixDevice/Common/PosixLocalFileSystem.h"
#include "zh/original_process.h"
#include <cstdio>

void updateMapStartSpots(GameInfo *, GameWindow *[], Bool);
void GadgetButtonSetText(GameWindow *, UnicodeString);
namespace { int failures; void check(bool ok, const char *m) { if (!ok) { std::fprintf(stderr, "FAIL: %s\n", m); ++failures; } }
class Text final : public GameTextInterface { public: void init() override {} void reset() override {} void update() override {} UnicodeString fetch(const Char *s, Bool *e=NULL) override { if(e)*e=TRUE; UnicodeString v; v.translate(AsciiString(s?s:"")); return v; } UnicodeString fetch(AsciiString s, Bool *e=NULL) override{return fetch(s.str(),e);} AsciiStringVec& getStringsWithLabelPrefix(AsciiString) override{return v;} void initMapStringFile(const AsciiString&) override{} private: AsciiStringVec v; };
class Button final : public GameWindow { public: void winDrawBorder() override {} };
class String final : public DisplayString { MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(String,"MapMarkerString",8,8) public: void setWordWrap(Int) override {} void setWordWrapCentered(Bool) override {} void draw(Int,Int,Color,Color) override {} void draw(Int,Int,Color,Color,Int,Int) override {} void getSize(Int *x,Int *y) override {if(x)*x=getWidth();if(y)*y=1;} Int getWidth(Int p=-1) override{return p<0?getTextLength():p;} void setUseHotkey(Bool,Color) override {} }; String::~String()=default;
class Strings final : public DisplayStringManager { public: ~Strings() override{while(m_stringList)freeDisplayString(m_stringList);} DisplayString *newDisplayString() override{++allocated; DisplayString *s=newInstance(String);link(s);return s;} void freeDisplayString(DisplayString *s) override{if(s){unLink(s);++freed;s->deleteInstance();}} DisplayString *getGroupNumeralString(Int) override{return newDisplayString();} DisplayString *getFormationLetterString() override{return newDisplayString();} static Int allocated,freed; }; Int Strings::allocated=0; Int Strings::freed=0;
class Windows final : public GameWindowManager { public:
 GameWindow *allocateNewWindow() override{return NULL;}
#define DRAW(name) GameWinDrawFunc name() override{return NULL;}
 DRAW(getPushButtonImageDrawFunc) DRAW(getPushButtonDrawFunc) DRAW(getCheckBoxImageDrawFunc) DRAW(getCheckBoxDrawFunc) DRAW(getRadioButtonImageDrawFunc) DRAW(getRadioButtonDrawFunc) DRAW(getTabControlImageDrawFunc) DRAW(getTabControlDrawFunc) DRAW(getListBoxImageDrawFunc) DRAW(getListBoxDrawFunc) DRAW(getComboBoxImageDrawFunc) DRAW(getComboBoxDrawFunc) DRAW(getHorizontalSliderImageDrawFunc) DRAW(getHorizontalSliderDrawFunc) DRAW(getVerticalSliderImageDrawFunc) DRAW(getVerticalSliderDrawFunc) DRAW(getProgressBarImageDrawFunc) DRAW(getProgressBarDrawFunc) DRAW(getStaticTextImageDrawFunc) DRAW(getStaticTextDrawFunc)
#undef DRAW
 GameWinDrawFunc getTextEntryImageDrawFunc() override{return NULL;}
 GameWinDrawFunc getTextEntryDrawFunc() override{return NULL;}
}; }
int main() {
 char diagnostic[128]{}; if(!zh::original_process::initialize_services(0,diagnostic,sizeof(diagnostic))) return 1; initMemoryManager();
 NameKeyGenerator keys; TheNameKeyGenerator=&keys; keys.init(); PosixLocalFileSystem localFiles; FileSystem files; TheLocalFileSystem=&localFiles; TheFileSystem=&files;
 for(int generation=0; generation!=2; ++generation) {
	{
  GlobalData global; TheWritableGlobalData=&global; Text text; TheGameText=&text; Strings strings; TheDisplayStringManager=&strings; Windows windows; TheWindowManager=&windows;
  Button buttons[MAX_SLOTS]; GameWindow *marker[MAX_SLOTS]{}; marker[0]=&buttons[0]; marker[1]=&buttons[1];
  buttons[0].winSetSystemFunc(GadgetPushButtonSystem); buttons[1].winSetSystemFunc(GadgetPushButtonSystem);
  UnicodeString one; one.translate(AsciiString("NUMBER:1")); UnicodeString two; two.translate(AsciiString("NUMBER:2"));
  GadgetButtonSetText(marker[0], one); GadgetButtonSetText(marker[1], two); GadgetButtonSetText(NULL, one);
  check(buttons[0].winGetText()==one && buttons[1].winGetText()==two, "source push-button label message contract changed");
  GadgetButtonSetText(marker[0], two); check(buttons[0].winGetText()==two, "source push-button label re-entry changed");
	MapCache cache; TheMapCache=&cache; MapMetaData map; map.m_numPlayers=2; map.m_isMultiplayer=TRUE; cache["generated.map"]=map;
	GameInfo game; game.init(); game.setMap("Generated.map"); GameSlot first, second, ownedFirst, ownedSecond;
	first.setState(SLOT_PLAYER); first.setPlayerTemplate(1); first.setStartPos(1);
	second.setState(SLOT_EASY_AI); second.setPlayerTemplate(1); second.setStartPos(0);
	game.setSlotPointer(0,&ownedFirst); game.setSlotPointer(1,&ownedSecond); game.setSlot(0,first); game.setSlot(1,second);
	updateMapStartSpots(&game,marker,FALSE);
	check(buttons[0].winGetText()==two && buttons[1].winGetText()==one,
		"source updateMapStartSpots did not route slot labels through live gadgets");
	game.getSlot(0)->setStartPos(2); updateMapStartSpots(&game,marker,FALSE); UnicodeString empty;
	check(game.getSlot(0)->getStartPos()==2, "generated out-of-range source slot was not published");
	check(buttons[0].winGetText()==two, "source valid sibling start-marker changed during rejection");
	check(buttons[1].winGetText()==empty, "source out-of-range start-marker rejection changed");
	game.getSlot(0)->setStartPos(1); updateMapStartSpots(&game,marker,FALSE);
	check(buttons[0].winGetText()==two && buttons[1].winGetText()==one,
		"source start-marker re-entry changed");
	game.setMap("missing.map"); updateMapStartSpots(&game,marker,FALSE);
	check(buttons[0].winIsHidden() && buttons[1].winIsHidden(), "source missing-map start-marker rejection changed");
	TheMapCache=NULL;
	}
	check(Strings::allocated==Strings::freed, "source start-marker teardown retained display strings");
  TheWindowManager=NULL; TheDisplayStringManager=NULL; TheGameText=NULL; TheWritableGlobalData=NULL;
 }
 TheFileSystem=NULL; TheLocalFileSystem=NULL; TheNameKeyGenerator=NULL; zh::original_process::shutdown_services(); shutdownMemoryManager();
 std::puts("original generated map-start labels: generations=2 markers=0"); return failures?1:0;
}
