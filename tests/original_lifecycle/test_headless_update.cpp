#include "PreRTS.h"
#include "Common/BuildAssistant.h"
#include "Common/CDManager.h"
#include "Common/GameAudio.h"
#include "Common/GameEngine.h"
#include "Common/FileSystem.h"
#include "Common/FunctionLexicon.h"
#include "Common/GlobalData.h"
#include "Common/GameState.h"
#include "Common/INI.h"
#include "Common/MessageStream.h"
#include "Common/MultiplayerSettings.h"
#include "Common/NameKeyGenerator.h"
#include "Common/PlayerTemplate.h"
#include "Common/PlayerList.h"
#include "Common/Recorder.h"
#include "Common/Radar.h"
#include "Common/AudioHandleSpecialValues.h"
#include "GameClient/Display.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/Anim2D.h"
#include "GameClient/Image.h"
#include "GameClient/Eva.h"
#include "GameClient/GameClient.h"
#include "GameClient/CampaignManager.h"
#include "GameClient/GameText.h"
#include "GameClient/InGameUI.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetProgressBar.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GUICallbacks.h"
#include "GameClient/HeaderTemplate.h"
#include "GameClient/View.h"
#include "GameLogic/AI.h"
#include "GameLogic/GameLogic.h"
#include "GameClient/LoadScreen.h"
#include "GameClient/Mouse.h"
#include "GameClient/VideoPlayer.h"
#include "GameClient/MapUtil.h"
#include "GameLogic/Locomotor.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/RankInfo.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/VictoryConditions.h"
#include "GameLogic/Weapon.h"
#include "GameNetwork/NetworkInterface.h"
#include "PosixDevice/Common/PosixLocalFileSystem.h"
#include "zh/original_process.h"

#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <unistd.h>

namespace {
int failures;
int client_ticks;
int presentation_step;
bool presentation_tracking;

void check(bool condition, const char *message)
{
	if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}

class GeneratedLoadVideoBuffer;

class HeadlessDisplay final : public Display
{
public:
	HeadlessDisplay() { setWidth(800); setHeight(600); }
	void update() override { if (presentation_tracking) { check(presentation_step == 2, "load-screen display update order changed"); presentation_step = 3; } ++updates; }
	void draw() override { if (presentation_tracking) { check(presentation_step == 3, "load-screen display draw order changed"); presentation_step = 4; } ++draws; }
	Bool isMoviePlaying() override { return TRUE; }
	void doSmartAssetPurgeAndPreload(const char *) override {}
#if defined(_DEBUG) || defined(_INTERNAL)
	void dumpAssetUsage(const char *) override {}
	void dumpModelAssets(const char *) override {}
#endif
	VideoBuffer *createVideoBuffer() override;
	void setClipRegion(IRegion2D *) override {}
	Bool isClippingEnabled() override { return FALSE; }
	void enableClipping(Bool) override {}
	void setTimeOfDay(TimeOfDay) override {}
	void createLightPulse(const Coord3D *, const RGBColor *, Real, Real, UnsignedInt, UnsignedInt) override {}
	void drawLine(Int, Int, Int, Int, Real, UnsignedInt) override {}
	void drawLine(Int, Int, Int, Int, Real, UnsignedInt, UnsignedInt) override {}
	void drawOpenRect(Int, Int, Int, Int, Real, UnsignedInt) override {}
	void drawFillRect(Int, Int, Int, Int, UnsignedInt) override {}
	void drawRectClock(Int, Int, Int, Int, Int, UnsignedInt) override {}
	void drawRemainingRectClock(Int, Int, Int, Int, Int, UnsignedInt) override {}
	void drawImage(const Image *, Int, Int, Int, Int, Color, DrawImageMode) override {}
	void drawVideoBuffer(VideoBuffer *, Int, Int, Int, Int) override {}
	void setShroudLevel(Int, Int, CellShroudStatus) override {}
	void clearShroud() override {}
	void setBorderShroudLevel(UnsignedByte) override {}
	void preloadModelAssets(AsciiString) override {}
	void preloadTextureAssets(AsciiString) override {}
	void takeScreenShot() override {}
	void toggleMovieCapture() override {}
	void toggleLetterBox() override {}
	void enableLetterBox(Bool) override {}
	Real getAverageFPS() override { return 0.0f; }
	Int getLastFrameDrawCalls() override { return 0; }
	int updates = 0;
	int draws = 0;
};

class GeneratedLoadVideoBuffer final : public VideoBuffer
{
public:
	GeneratedLoadVideoBuffer() : VideoBuffer(TYPE_X8R8G8B8), allocated(FALSE) {}
	Bool allocate(UnsignedInt width, UnsignedInt height) override
	{
		if (width != 2 || height != 1) return FALSE;
		allocated = TRUE;
		m_width = m_textureWidth = width;
		m_height = m_textureHeight = height;
		m_pitch = 8;
		return TRUE;
	}
	void free() override { allocated = FALSE; m_width = m_height = m_textureWidth = m_textureHeight = m_pitch = 0; }
	void *lock() override { return NULL; }
	void unlock() override {}
	Bool valid() override { return allocated; }
private:
	Bool allocated;
};

VideoBuffer *HeadlessDisplay::createVideoBuffer()
{
	return new GeneratedLoadVideoBuffer;
}

class GeneratedLoadVideoStream final : public VideoStream
{
public:
	explicit GeneratedLoadVideoStream(VideoPlayer *player) { m_player = player; }
	Int width() override { return 2; }
	Int height() override { return 1; }
	Int frameCount() override { return 1; }
};

class GeneratedLoadVideoPlayer final : public VideoPlayer
{
public:
	VideoStreamInterface *open(AsciiString movie) override
	{
		if (movie.compare("GeneratedLoadMovie") != 0 || m_firstStream != NULL) return NULL;
		m_firstStream = new GeneratedLoadVideoStream(this);
		++opens;
		return m_firstStream;
	}
	int opens = 0;
};

class HeadlessClient final : public GameClient
{
public:
	void createRayEffectByTemplate(const Coord3D *, const Coord3D *, const ThingTemplate *) override {}
	void addScorch(const Coord3D *, Real, Scorches) override {}
	Drawable *friend_createDrawable(const ThingTemplate *, DrawableStatus) override { return NULL; }
	void setTeamColor(Int, Int, Int) override {}
	void adjustLOD(Int) override {}
	void notifyTerrainObjectMoved(Object *) override {}
private:
	Display *createGameDisplay() override { return NULL; }
	InGameUI *createInGameUI() override { return NULL; }
	GameWindowManager *createWindowManager() override { return NULL; }
	FontLibrary *createFontLibrary() override { return NULL; }
	DisplayStringManager *createDisplayStringManager() override { return NULL; }
	VideoPlayerInterface *createVideoPlayer() override { return NULL; }
	TerrainVisual *createTerrainVisual() override { return NULL; }
	Keyboard *createKeyboard() override { return NULL; }
	Mouse *createMouse() override { return NULL; }
	SnowManager *createSnowManager() override { return NULL; }
	void setFrameRate(Real) override {}
};

class HeadlessView final : public View
{
public:
	Drawable *pickDrawable(const ICoord2D *, Bool, PickType) override { return NULL; }
	Int iterateDrawablesInRegion(IRegion2D *, Bool (*)(Drawable *, void *), void *) override { return 0; }
	void forceRedraw() override {}
	const Coord3D& get3DCameraPosition() const override { return position; }
	WorldToScreenReturn worldToScreenTriReturn(const Coord3D *, ICoord2D *) override { return WTS_INVALID; }
	void screenToWorld(const ICoord2D *, Coord3D *) override {}
	void screenToTerrain(const ICoord2D *, Coord3D *) override {}
	void screenToWorldAtZ(const ICoord2D *, Coord3D *, Real) override {}
	void drawView() override {}
	void updateView() override {}
	void setGuardBandBias(const Coord2D *) override {}
private:
	Coord3D position{};
};

class HeadlessInGameUI final : public InGameUI
{
public:
	void draw() override {}
protected:
	View *createView() override { return NULL; }
};

class HeadlessRadar final : public Radar
{
public:
	void draw(Int, Int, Int, Int) override {}
	void clearShroud() override {}
	void setShroudLevel(Int, Int, CellShroudStatus) override {}
};

class HeadlessAudio final : public AudioManager
{
public:
#if defined(_DEBUG) || defined(_INTERNAL)
	void audioDebugDisplay(DebugDisplayInterface *, void *, FILE *) override {}
#endif
	void stopAudio(AudioAffect) override {}
	void pauseAudio(AudioAffect) override {}
	void resumeAudio(AudioAffect) override {}
	void pauseAmbient(Bool) override {}
	void killAudioEventImmediately(AudioHandle) override {}
	void nextMusicTrack() override {}
	void prevMusicTrack() override {}
	Bool isMusicPlaying() const override { return FALSE; }
	Bool hasMusicTrackCompleted(const AsciiString&, Int) const override { return FALSE; }
	AsciiString getMusicTrackName() const override { return AsciiString::TheEmptyString; }
	void openDevice() override { ++deviceOpens; }
	void closeDevice() override {}
	void *getDevice() override { return NULL; }
	void notifyOfAudioCompletion(UnsignedInt, UnsignedInt) override {}
	UnsignedInt getProviderCount() const override { return 0; }
	AsciiString getProviderName(UnsignedInt) const override { return AsciiString::TheEmptyString; }
	UnsignedInt getProviderIndex(AsciiString) const override { return 0; }
	void selectProvider(UnsignedInt) override {}
	void unselectProvider() override {}
	UnsignedInt getSelectedProvider() const override { return 0; }
	void setSpeakerType(UnsignedInt) override {}
	UnsignedInt getSpeakerType() override { return 0; }
	UnsignedInt getNum2DSamples() const override { return 0; }
	UnsignedInt getNum3DSamples() const override { return 0; }
	UnsignedInt getNumStreams() const override { return 0; }
	Bool doesViolateLimit(AudioEventRTS *) const override { return FALSE; }
	Bool isPlayingLowerPriority(AudioEventRTS *) const override { return FALSE; }
	Bool isPlayingAlready(AudioEventRTS *) const override { return FALSE; }
	AudioHandle addAudioEvent(const AudioEventRTS *event) override
	{
		if (failNextAdd || !event || event->getEventName().isEmpty())
		{
			failNextAdd = FALSE;
			++rejectedAdds;
			return AHSV_Error;
		}
		PlayingEvent entry;
		entry.handle = nextHandle++;
		entry.name = event->getEventName();
		active.push_back(entry);
		return entry.handle;
	}
	void removeAudioEvent(AudioHandle handle) override
	{
		if (handle < AHSV_FirstHandle) { ++rejectedRemoves; return; }
		for (std::vector<PlayingEvent>::iterator it = active.begin(); it != active.end(); ++it)
		{
			if (it->handle == handle) { active.erase(it); ++removed; return; }
		}
		++rejectedRemoves;
	}
	Bool isObjectPlayingVoice(UnsignedInt) const override { return FALSE; }
	void adjustVolumeOfPlayingAudio(AsciiString, Real) override {}
	void removePlayingAudio(AsciiString) override {}
	void removeAllDisabledAudio() override {}
	Bool has3DSensitiveStreamsPlaying() const override { return FALSE; }
	void *getHandleForBink() override { return NULL; }
	void releaseHandleForBink() override {}
	void friend_forcePlayAudioEventRTS(const AudioEventRTS *event) override
	{
		if (!event || event->getEventName().isEmpty()) { ++rejectedBriefings; return; }
		briefings.push_back(event->getEventName());
	}
	void setPreferredProvider(AsciiString) override {}
	void setPreferredSpeaker(AsciiString) override {}
	Real getFileLengthMS(AsciiString) const override { return 0.0f; }
	void closeAnySamplesUsingFile(const void *) override {}
	int deviceOpens = 0;
	Bool failNextAdd = FALSE;
	Int rejectedAdds = 0;
	Int rejectedRemoves = 0;
	Int rejectedBriefings = 0;
	Int removed = 0;
	Int activeCount() const { return static_cast<Int>(active.size()); }
	Int briefingCount() const { return static_cast<Int>(briefings.size()); }
	const AsciiString &lastBriefing() const { return briefings.back(); }
protected:
	void setDeviceListenerPosition() override {}
	private:
	struct PlayingEvent { AudioHandle handle; AsciiString name; };
	AudioHandle nextHandle = AHSV_FirstHandle;
	std::vector<PlayingEvent> active;
	std::vector<AsciiString> briefings;
};

class HeadlessMouse final : public Mouse
{
public:
	void initCursorResources() override {}
	void setCursor(MouseCursor cursor) override { m_currentCursor = cursor; }
	void capture() override {}
	void releaseCapture() override {}
	Bool tooltipEmpty() const { return m_isTooltipEmpty; }
protected:
	UnsignedByte getMouseEvent(MouseIO *, Bool) override { return 0; }
};

class HeadlessCDManager final : public CDManager
{
protected:
	CDDriveInterface *createDrive() override { return new CDDrive; }
};

class HeadlessEngine final : public GameEngine
{
public:
	void serviceWindowsOS() override { if (presentation_tracking) { check(presentation_step == 0, "load-screen OS service order changed"); presentation_step = 1; } }
protected:
	LocalFileSystem *createLocalFileSystem() override { return NULL; }
	ArchiveFileSystem *createArchiveFileSystem() override { return NULL; }
	GameLogic *createGameLogic() override { return NULL; }
	GameClient *createGameClient() override { return NULL; }
	ModuleFactory *createModuleFactory() override { return NULL; }
	ThingFactory *createThingFactory() override { return NULL; }
	FunctionLexicon *createFunctionLexicon() override { return NULL; }
	Radar *createRadar() override { return NULL; }
	WebBrowser *createWebBrowser() override { return NULL; }
	ParticleSystemManager *createParticleSystemManager() override { return NULL; }
	AudioManager *createAudioManager() override { return NULL; }
};

class HeadlessGameWindow final : public GameWindow
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(HeadlessGameWindow, "W3DGameWindow")
public:
	void winDrawBorder() override {}
};

HeadlessGameWindow::~HeadlessGameWindow() = default;

class HeadlessWindowManager final : public GameWindowManager
{
public:
	void drainDestroyedWindows() { processDestroyList(); }
	void update() override { if (presentation_tracking) { check(presentation_step == 1, "load-screen window update order changed"); presentation_step = 2; } GameWindowManager::update(); }
	GameWindow *allocateNewWindow() override { return newInstance(HeadlessGameWindow); }
#define ZH_HEADLESS_DRAW_GETTER(name) GameWinDrawFunc name() override { return NULL; }
	ZH_HEADLESS_DRAW_GETTER(getPushButtonImageDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getPushButtonDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getCheckBoxImageDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getCheckBoxDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getRadioButtonImageDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getRadioButtonDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getTabControlImageDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getTabControlDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getListBoxImageDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getListBoxDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getComboBoxImageDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getComboBoxDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getHorizontalSliderImageDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getHorizontalSliderDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getVerticalSliderImageDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getVerticalSliderDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getProgressBarImageDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getProgressBarDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getStaticTextImageDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getStaticTextDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getTextEntryImageDrawFunc)
	ZH_HEADLESS_DRAW_GETTER(getTextEntryDrawFunc)
#undef ZH_HEADLESS_DRAW_GETTER
	void winDrawImage(const Image *, Int, Int, Int, Int, Color) override {}
	void winFillRect(Color, Real, Int, Int, Int, Int) override {}
	void winOpenRect(Color, Real, Int, Int, Int, Int) override {}
	void winDrawLine(Color, Real, Int, Int, Int, Int) override {}
	Color winMakeColor(UnsignedByte red, UnsignedByte green, UnsignedByte blue, UnsignedByte alpha) override
	{
		return GameMakeColor(red, green, blue, alpha);
	}
	const Image *winFindImage(const char *) override { return NULL; }
	Int winFontHeight(GameFont *) override { return 0; }
	Int winIsDigit(Int value) override { return std::isdigit(static_cast<unsigned char>(value)); }
	Int winIsAscii(Int value) override { return value >= 0 && value <= 127; }
	Int winIsAlNum(Int value) override { return std::isalnum(static_cast<unsigned char>(value)); }
	void winFormatText(GameFont *, UnicodeString, Color, Int, Int, Int, Int) override {}
	void winGetTextSize(GameFont *, UnicodeString, Int *width, Int *height, Int) override
	{
		if (width) *width = 0;
		if (height) *height = 0;
	}
	GameFont *winFindFont(AsciiString, Int, Bool) override { return NULL; }
};

class BaseLoadScreenProbe final : public LoadScreen
{
public:
	void init(GameInfo *) override {}
	void reset() override {}
	void update() override {}
	void processProgress(Int, Int) override {}
	void setProgressRange(Int, Int) override {}
	void present(Int percent) { LoadScreen::update(percent); }
};

class TickTranslator final : public GameMessageTranslator
{
public:
	GameMessageDisposition translateGameMessage(const GameMessage *message) override
	{
		if (message->getType() == GameMessage::MSG_FRAME_TICK) ++client_ticks;
		return KEEP_MESSAGE;
	}
};

class HeadlessGameText final : public GameTextInterface
{
public:
	void init() override {}
	void reset() override {}
	void update() override {}
	UnicodeString fetch(const Char *label, Bool *exists = NULL) override
	{
		if (exists) *exists = TRUE;
		UnicodeString value;
		value.translate(AsciiString(label ? label : ""));
		return value;
	}
	UnicodeString fetch(AsciiString label, Bool *exists = NULL) override { return fetch(label.str(), exists); }
	AsciiStringVec& getStringsWithLabelPrefix(AsciiString) override { return strings; }
	void initMapStringFile(const AsciiString&) override {}
private:
	AsciiStringVec strings;
};

class HeadlessDisplayString final : public DisplayString
{
	MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(HeadlessDisplayString, "HeadlessDisplayString", 8, 8)
public:
	void setWordWrap(Int width) override { wrap = width; }
	void setWordWrapCentered(Bool value) override { centered = value; }
	void draw(Int, Int, Color, Color) override {}
	void draw(Int, Int, Color, Color, Int, Int) override {}
	void getSize(Int *width, Int *height) override
	{
		if (width) *width = getWidth();
		if (height) *height = 1;
	}
	Int getWidth(Int charPos = -1) override
	{
		return (charPos < 0 ? getTextLength() : std::min(charPos, getTextLength()));
	}
	void setUseHotkey(Bool, Color) override {}
private:
	Int wrap = 0;
	Bool centered = FALSE;
};
HeadlessDisplayString::~HeadlessDisplayString() = default;

class HeadlessDisplayStringManager final : public DisplayStringManager
{
public:
	~HeadlessDisplayStringManager() override
	{
		while (m_stringList) freeDisplayString(m_stringList);
	}
	DisplayString *newDisplayString() override
	{
		DisplayString *value = newInstance(HeadlessDisplayString);
		link(value);
		return value;
	}
	void freeDisplayString(DisplayString *value) override
	{
		if (!value) return;
		unLink(value);
		value->deleteInstance();
	}
	DisplayString *getGroupNumeralString(Int) override { return newDisplayString(); }
	DisplayString *getFormationLetterString() override { return newDisplayString(); }
	Int count() const
	{
		Int total = 0;
		for (DisplayString *value = m_stringList; value; value = value->next()) ++total;
		return total;
	}
};

class HeadlessScriptEngine final : public ScriptEngine { public: void update() override { ++updates; } int updates = 0; };
class HeadlessTerrain final : public TerrainLogic { public: void update() override { ++updates; } int updates = 0; };
class HeadlessAI final : public AI { public: void update() override { ++updates; } int updates = 0; };
class HeadlessBuild final : public BuildAssistant { public: void update() override { ++updates; } int updates = 0; };
class HeadlessPartition final : public PartitionManager { public: void update() override { ++updates; } int updates = 0; };

class HeadlessVictory final : public VictoryConditionsInterface
{
public:
	void init() override {}
	void reset() override {}
	void update() override { ++updates; }
	Bool hasAchievedVictory(Player *) override { return FALSE; }
	Bool hasBeenDefeated(Player *) override { return FALSE; }
	Bool hasSinglePlayerBeenDefeated(Player *) override { return FALSE; }
	void cachePlayerPtrs() override {}
	Bool isLocalAlliedVictory() override { return FALSE; }
	Bool isLocalAlliedDefeat() override { return FALSE; }
	Bool isLocalDefeat() override { return FALSE; }
	Bool amIObserver() override { return FALSE; }
	UnsignedInt getEndFrame() override { return 0; }
	int updates = 0;
};
}

int main()
{
	char temp_template[] = "/tmp/zh-m20-headless-XXXXXX";
	char *temp_root = mkdtemp(temp_template);
	check(temp_root != NULL, "could not create isolated runtime root");
	if (!temp_root) return 1;
	const std::filesystem::path root(temp_root);
	const std::string config = (root / "config").string();
	const std::string data = (root / "data").string();
	const std::string cache = (root / "cache").string();
	const std::string state = (root / "state").string();
	setenv("XDG_CONFIG_HOME", config.c_str(), 1);
	setenv("XDG_DATA_HOME", data.c_str(), 1);
	setenv("XDG_CACHE_HOME", cache.c_str(), 1);
	setenv("XDG_STATE_HOME", state.c_str(), 1);
	const std::filesystem::path input = root / "input";
	std::filesystem::create_directories(input / "Window/Menus");
	{
		std::ofstream blank(input / "Window/Menus/BlankWindow.wnd");
		blank << "FILE_VERSION = 2\n"
			"STARTLAYOUTBLOCK\nENDLAYOUTBLOCK\n"
			"WINDOW\nWINDOWTYPE = USER;\n"
			"SCREENRECT = UPPERLEFT: 0 0 BOTTOMRIGHT: 800 600 CREATIONRESOLUTION: 800 600;\n"
			"NAME = \"BlankWindow\";\nSTATUS = ENABLED IMAGE;\nSTYLE = USER;\nEND\n";
	}
	{
		std::ofstream callback(input / "Window/Menus/CallbackWindow.wnd");
		callback << "FILE_VERSION = 2\nSTARTLAYOUTBLOCK\n"
			"LAYOUTINIT = MainMenuInit;\nENDLAYOUTBLOCK\n";
	}
	{
		std::ofstream generated(input / "Window/Menus/GeneratedSinglePlayerTree.wnd");
		generated << "FILE_VERSION = 2\nSTARTLAYOUTBLOCK\nENDLAYOUTBLOCK\n"
			"WINDOW\nWINDOWTYPE = USER;\n"
			"SCREENRECT = UPPERLEFT: 0 0 BOTTOMRIGHT: 800 600 CREATIONRESOLUTION: 800 600;\n"
			"NAME = \"GeneratedSinglePlayerRoot\";\nSTATUS = ENABLED IMAGE;\nSTYLE = USER;\nCHILD\n"
			"WINDOW\nWINDOWTYPE = PROGRESSBAR;\n"
			"SCREENRECT = UPPERLEFT: 10 10 BOTTOMRIGHT: 110 30 CREATIONRESOLUTION: 800 600;\n"
			"NAME = \"GeneratedSinglePlayer:Progress\";\nSTATUS = ENABLED IMAGE;\nSTYLE = PROGRESSBAR;\nEND\n"
			"CHILD\nWINDOW\nWINDOWTYPE = STATICTEXT;\n"
			"SCREENRECT = UPPERLEFT: 10 40 BOTTOMRIGHT: 210 60 CREATIONRESOLUTION: 800 600;\n"
			"NAME = \"GeneratedSinglePlayer:Objective\";\nSTATUS = ENABLED IMAGE;\nSTYLE = STATICTEXT;\n"
			"STATICTEXTDATA = CENTERED:No;\nEND\n"
			"ENDALLCHILDREN\nEND\n";
	}
	{
		std::ofstream generated(input / "Window/Menus/SinglePlayerLoadScreen.wnd");
		generated << "FILE_VERSION = 2\nSTARTLAYOUTBLOCK\nENDLAYOUTBLOCK\n"
			"WINDOW\nWINDOWTYPE = USER;\n"
			"SCREENRECT = UPPERLEFT: 0 0 BOTTOMRIGHT: 800 600 CREATIONRESOLUTION: 800 600;\n"
			"NAME = \"SinglePlayerLoadScreen.wnd:ParentSinglePlayerLoadScreen\";\n"
			"STATUS = ENABLED IMAGE;\nSTYLE = USER;\nCHILD\n";
		auto child = [&](const char *type, const char *name, const char *style) {
			generated << "WINDOW\nWINDOWTYPE = " << type << ";\n"
				"SCREENRECT = UPPERLEFT: 10 10 BOTTOMRIGHT: 210 30 CREATIONRESOLUTION: 800 600;\n"
				"NAME = \"" << name << "\";\nSTATUS = ENABLED IMAGE;\nSTYLE = " << style << ";\n";
			if (std::string(type) == "STATICTEXT") generated << "STATICTEXTDATA = CENTERED:No;\n";
			generated << "END\n";
		};
		child("PROGRESSBAR", "SinglePlayerLoadScreen.wnd:ProgressLoad", "PROGRESSBAR");
		child("STATICTEXT", "SinglePlayerLoadScreen.wnd:Percent", "STATICTEXT");
		child("USER", "SinglePlayerLoadScreen.wnd:ObjectivesWin", "USER");
		child("STATICTEXT", "SinglePlayerLoadScreen.wnd:StaticTextLine0", "STATICTEXT");
		child("STATICTEXT", "SinglePlayerLoadScreen.wnd:StaticTextLine1", "STATICTEXT");
		child("STATICTEXT", "SinglePlayerLoadScreen.wnd:StaticTextLine2", "STATICTEXT");
		child("STATICTEXT", "SinglePlayerLoadScreen.wnd:StaticTextCameoText0", "STATICTEXT");
		child("STATICTEXT", "SinglePlayerLoadScreen.wnd:StaticTextCameoText1", "STATICTEXT");
		child("STATICTEXT", "SinglePlayerLoadScreen.wnd:StaticTextCameoText2", "STATICTEXT");
		child("STATICTEXT", "SinglePlayerLoadScreen.wnd:StaticTextCameoText3", "STATICTEXT");
		generated << "ENDALLCHILDREN\nEND\n";
	}
	{
		std::ofstream generated(input / "Window/Menus/MultiplayerLoadScreen.wnd");
		generated << "FILE_VERSION = 2\nSTARTLAYOUTBLOCK\nENDLAYOUTBLOCK\n"
			"WINDOW\nWINDOWTYPE = USER;\n"
			"SCREENRECT = UPPERLEFT: 0 0 BOTTOMRIGHT: 800 600 CREATIONRESOLUTION: 800 600;\n"
			"NAME = \"MultiplayerLoadScreen.wnd:Parent\";\nSTATUS = ENABLED IMAGE;\nSTYLE = USER;\nCHILD\n";
		auto child = [&](const char *type, const std::string &name, const char *style) {
			generated << "WINDOW\nWINDOWTYPE = " << type << ";\n"
				"SCREENRECT = UPPERLEFT: 10 10 BOTTOMRIGHT: 210 30 CREATIONRESOLUTION: 800 600;\n"
				"NAME = \"" << name << "\";\nSTATUS = ENABLED IMAGE;\nSTYLE = " << style << ";\n";
			if (std::string(type) == "STATICTEXT") generated << "STATICTEXTDATA = CENTERED:No;\n";
			generated << "END\n";
		};
		child("USER", "MultiplayerLoadScreen.wnd:WinMapPreview", "USER");
		child("USER", "MultiplayerLoadScreen.wnd:LocalGeneralPortrait", "USER");
		child("STATICTEXT", "MultiplayerLoadScreen.wnd:LocalGeneralFeatures", "STATICTEXT");
		child("STATICTEXT", "MultiplayerLoadScreen.wnd:LocalGeneralName", "STATICTEXT");
		for (Int i = 0; i < MAX_SLOTS; ++i)
		{
			child("PROGRESSBAR", "MultiplayerLoadScreen.wnd:ProgressLoad" + std::to_string(i), "PROGRESSBAR");
			child("PUSHBUTTON", "MultiplayerLoadScreen.wnd:ButtonMapStartPosition" + std::to_string(i), "PUSHBUTTON");
			child("STATICTEXT", "MultiplayerLoadScreen.wnd:StaticTextPlayer" + std::to_string(i), "STATICTEXT");
			child("STATICTEXT", "MultiplayerLoadScreen.wnd:StaticTextSide" + std::to_string(i), "STATICTEXT");
			child("STATICTEXT", "MultiplayerLoadScreen.wnd:StaticTextTeam" + std::to_string(i), "STATICTEXT");
		}
		generated << "ENDALLCHILDREN\nEND\n";
	}
	std::filesystem::create_directories(input / "Maps");
	{
		std::ofstream preview(input / "Maps/Generated.tga", std::ios::binary);
		preview << "generated-multiplayer-preview";
	}
	char diagnostic[128]{};
	check(zh::original_process::initialize_services(0, diagnostic, sizeof(diagnostic)), diagnostic);
	initMemoryManager();
	{
	PosixLocalFileSystem localFiles(input);
	FileSystem files;
	TheLocalFileSystem = &localFiles;
	TheFileSystem = &files;
	TheWritableGlobalData = new GlobalData;
	TheWritableGlobalData->m_playIntro = TRUE;
	TheWritableGlobalData->m_playSizzle = FALSE;
	HeadlessDisplay display;
	GeneratedLoadVideoPlayer videoPlayer;
	ImageCollection mappedImages;
	auto addGeneratedImage = [&](const char *name) {
		Image *image = newInstance(Image);
		check(image != NULL, "generated mapped-image fixture did not allocate");
		if (image) { image->setName(AsciiString(name)); mappedImages.addImage(image); }
	};
	HeadlessClient client;
	NameKeyGenerator nameKeys;
	TheNameKeyGenerator = &nameKeys;
	nameKeys.init();
	addGeneratedImage("MissionLoad_USA");
	addGeneratedImage("LoadingBar_ProgressCenter2");
	addGeneratedImage("LoadingBar_ProgressCenter0");
	addGeneratedImage("SAFactionLogoLg_US");
	FunctionLexicon functionLexicon;
	TheFunctionLexicon = &functionLexicon;
	functionLexicon.init();
	const NameKeyType offlineCallbackKey = nameKeys.nameToKey(AsciiString("MainMenuSystem"));
	check(functionLexicon.gameWinSystemFunc(offlineCallbackKey) == MainMenuSystem,
		"offline callback did not resolve to its actual original provider");
	const NameKeyType onlineCallbackKey = nameKeys.nameToKey(AsciiString("WOLLoginMenuSystem"));
	GameWinSystemFunc onlineCallback = functionLexicon.gameWinSystemFunc(onlineCallbackKey);
	Bool onlineRejected = FALSE;
	try
	{
		onlineCallback(NULL, 0, 0, 0);
	}
	catch (const std::runtime_error& error)
	{
		onlineRejected = std::string(error.what()) ==
			"unsupported online callback: WOLLoginMenuSystem";
	}
	check(onlineRejected, "online callback did not fail closed with its exact name");
	const auto rejectsLayout = [&](const char *name, WindowLayoutInitFunc callback) {
		try
		{
			callback(NULL, NULL);
		}
		catch (const std::runtime_error& error)
		{
			return std::string(error.what()) == std::string("unsupported online callback: ") + name;
		}
		return false;
	};
	check(rejectsLayout("WOLLoginMenuInit", functionLexicon.winLayoutInitFunc(
		nameKeys.nameToKey(AsciiString("WOLLoginMenuInit")))),
		"online init callback did not fail closed with its exact name");
	check(rejectsLayout("WOLLobbyMenuUpdate", reinterpret_cast<WindowLayoutInitFunc>(
		functionLexicon.winLayoutUpdateFunc(nameKeys.nameToKey(AsciiString("WOLLobbyMenuUpdate"))))),
		"online update callback did not fail closed with its exact name");
	check(rejectsLayout("WOLStatusMenuShutdown", reinterpret_cast<WindowLayoutInitFunc>(
		functionLexicon.winLayoutShutdownFunc(nameKeys.nameToKey(AsciiString("WOLStatusMenuShutdown"))))),
		"online shutdown callback did not fail closed with its exact name");
	TheFunctionLexicon = NULL;
	HeaderTemplateManager headers;
	TheHeaderTemplateManager = &headers;
	HeadlessWindowManager *windowManager = new HeadlessWindowManager;
	TheWindowManager = windowManager;
	RankInfoStore ranks;
	ranks.init();
	TheRankInfoStore = &ranks;
	PlayerList *players = new PlayerList;
	ThePlayerList = players;
	MessageStream messages;
	CommandList commands;
	messages.init();
	commands.init();
	messages.attachTranslator(new TickTranslator, 1);
	HeadlessView view;
	HeadlessScriptEngine scripts;
	HeadlessTerrain terrain;
	HeadlessAI ai;
	HeadlessBuild build;
	HeadlessPartition *partition = new HeadlessPartition;
	HeadlessGameText gameText;
	TheGameText = &gameText;
	HeadlessDisplayStringManager displayStrings;
	TheDisplayStringManager = &displayStrings;
	Anim2DCollection animations;
	TheAnim2DCollection = &animations;
	Eva eva;
	TheEva = &eva;
	InGameUI *inGameUI = new HeadlessInGameUI;
	TheInGameUI = inGameUI;
	RecorderClass recorder;
	recorder.init();
	WeaponStore weapons;
	LocomotorStore locomotors;
	HeadlessVictory victory;
	GameLogic logic;
	HeadlessRadar *radar = new HeadlessRadar;
	HeadlessAudio audio;
	HeadlessMouse mouse;
	HeadlessCDManager cdManager;
	HeadlessEngine *engine = new HeadlessEngine;

	TheDisplay = &display;
	TheVideoPlayer = &videoPlayer;
	TheMappedImageCollection = &mappedImages;
	TheMouse = &mouse;
	TheGameClient = &client;
	TheMessageStream = &messages;
	TheCommandList = &commands;
	TheTacticalView = &view;
	TheScriptEngine = &scripts;
	TheTerrainLogic = &terrain;
	TheAI = &ai;
	TheBuildAssistant = &build;
	ThePartitionManager = partition;
	TheRecorder = &recorder;
	TheWeaponStore = &weapons;
	TheLocomotorStore = &locomotors;
	TheVictoryConditions = &victory;
	TheGameLogic = &logic;
	TheRadar = radar;
	TheAudio = &audio;
	TheCDManager = &cdManager;
	TheGameEngine = engine;
	TheNetwork = NULL;

	const Int displayStringBaseline = displayStrings.count();
	for (int generation = 0; generation != 2; ++generation)
	{
		CampaignManager campaigns;
		TheCampaignManager = &campaigns;
		Campaign *campaign = campaigns.newCampaign(AsciiString("GeneratedCampaign"));
		check(campaign != NULL, "generated campaign factory did not return an owner");
		Mission *mission = campaign ? campaign->newMission(AsciiString("GeneratedMission")) : NULL;
		check(mission != NULL, "generated mission factory did not return an owner");
		if (mission)
		{
			mission->m_mapName.set("GeneratedMap");
			mission->m_movieLabel.set("GeneratedMovie");
			mission->m_missionObjectivesLabel[0].set("GeneratedObjective");
			mission->m_unitNames[0].set("GeneratedUnit");
			mission->m_locationNameLabel.set("GeneratedLocation");
			mission->m_voiceLength = generation + 1;
		}
		campaigns.setCampaignAndMission(AsciiString("generatedcampaign"), AsciiString("generatedmission"));
		check(campaigns.getCurrentCampaign() == campaign && campaigns.getCurrentMission() == mission,
			"generated campaign selection did not retain source identity");
		check(mission && mission->m_mapName.compare("GeneratedMap") == 0 &&
			mission->m_movieLabel.compare("GeneratedMovie") == 0 &&
			mission->m_missionObjectivesLabel[0].compare("GeneratedObjective") == 0 &&
			mission->m_unitNames[0].compare("GeneratedUnit") == 0 &&
			mission->m_locationNameLabel.compare("GeneratedLocation") == 0 &&
			mission->m_voiceLength == generation + 1,
			"generated mission field descriptor was not retained by the source owner");
		check(campaigns.gotoNextMission() == NULL,
			"generated mission without a next edge did not stop source traversal");
		campaigns.setCampaign(AsciiString("MissingGeneratedCampaign"));
		check(campaigns.getCurrentCampaign() == NULL && campaigns.getCurrentMission() == NULL,
			"unknown generated campaign did not fail closed");
		campaigns.setCampaignAndMission(AsciiString("GENERATEDCAMPAIGN"), AsciiString("generatedmission"));
		check(campaigns.getCurrentMission() == mission,
			"generated campaign did not re-enter after a failed selection");
		check(campaign && campaign->getMission(AsciiString::TheEmptyString) == NULL,
			"empty generated mission lookup did not fail closed");
		Mission *replacement = campaign ? campaign->newMission(AsciiString("GENERATEDMISSION")) : NULL;
		if (replacement)
			replacement->m_movieLabel.set("GeneratedReplacementMovie");
		check(replacement != NULL && campaign->getMission(AsciiString("generatedmission")) == replacement &&
			replacement->m_movieLabel.compare("GeneratedReplacementMovie") == 0,
			"duplicate generated mission did not replace through the source owner");
		campaigns.setCampaignAndMission(AsciiString("GeneratedCampaign"), AsciiString("generatedmission"));
		check(campaigns.getCurrentMission() == replacement,
			"duplicate generated mission was not republished through source selection");
		Campaign *replacementCampaign = campaigns.newCampaign(AsciiString("GENERATEDCAMPAIGN"));
		Mission *replacementMission = replacementCampaign ? replacementCampaign->newMission(AsciiString("GeneratedMission")) : NULL;
		check(replacementCampaign != NULL && replacementMission != NULL,
			"duplicate generated campaign did not replace through the source owner");
		campaigns.setCampaignAndMission(AsciiString("generatedcampaign"), AsciiString("generatedmission"));
		check(campaigns.getCurrentCampaign() == replacementCampaign &&
			campaigns.getCurrentMission() == replacementMission,
			"replacement generated campaign did not publish its owned mission");
		if (replacementMission)
			replacementMission->m_briefingVoice = AudioEventRTS(AsciiString("GeneratedBriefing"));
		AudioEventRTS emptyBriefing;
		audio.friend_forcePlayAudioEventRTS(&emptyBriefing);
		audio.friend_forcePlayAudioEventRTS(replacementMission ? &replacementMission->m_briefingVoice : NULL);
		check(audio.rejectedBriefings == generation + 1 && audio.briefingCount() == generation + 1 &&
			audio.lastBriefing().compare("GeneratedBriefing") == 0,
			"generated source mission briefing did not reach the bounded audio provider");
		AudioEventRTS ambient(AsciiString("GeneratedAmbient"));
		AudioEventRTS ambientCopy(ambient);
		audio.failNextAdd = TRUE;
		check(audio.addAudioEvent(&ambientCopy) == AHSV_Error && audio.activeCount() == 0,
			"injected generated ambient registration failure retained provider state");
		const AudioHandle ambientHandle = audio.addAudioEvent(&ambientCopy);
		check(ambientHandle >= AHSV_FirstHandle && audio.activeCount() == 1,
			"generated ambient did not register through the bounded audio provider");
		audio.removeAudioEvent(AHSV_NoSound);
		check(audio.activeCount() == 1,
			"special generated audio handle mutated active provider state");
		audio.removeAudioEvent(ambientHandle);
		check(audio.activeCount() == 0,
			"generated ambient removal did not release bounded provider state");
		TheCampaignManager = NULL;
	}
	check(TheCampaignManager == NULL,
		"generated campaign/mission provider remained published after both generations");

	const Int audioRejectedAddsBeforeOwner = audio.rejectedAdds;
	const Int audioRemovedBeforeOwner = audio.removed;
	for (int generation = 0; generation != 2; ++generation)
	{
		CampaignManager campaigns;
		TheCampaignManager = &campaigns;
		Campaign *campaign = campaigns.newCampaign(AsciiString("USA"));
		Mission *mission = campaign ? campaign->newMission(AsciiString("GeneratedLoadMission")) : NULL;
		check(campaign != NULL && mission != NULL,
			"generated SinglePlayer owner fixture did not create its source campaign/mission");
		if (mission)
		{
			mission->m_movieLabel.set("GeneratedLoadMovie");
			mission->m_missionObjectivesLabel[0].set("GeneratedObjective");
			mission->m_unitNames[0].set("GeneratedUnit");
			mission->m_locationNameLabel.set("GeneratedLocation");
			mission->m_voiceLength = 0;
		}
		campaigns.setCampaignAndMission(AsciiString("usa"), AsciiString("generatedloadmission"));
		check(campaigns.getCurrentCampaign() == campaign && campaigns.getCurrentMission() == mission,
			"generated SinglePlayer owner did not publish the selected source mission");

		audio.failNextAdd = TRUE;
		{
			SinglePlayerLoadScreen screen;
			screen.init(NULL);
			check(audio.activeCount() == 0 && audio.rejectedAdds == audioRejectedAddsBeforeOwner + generation + 1,
				"injected SinglePlayer ambient failure retained source audio state");
		}
		windowManager->drainDestroyedWindows();
		check(windowManager->winGetWindowList() == NULL && displayStrings.count() == displayStringBaseline,
			"failed SinglePlayer owner retry setup retained a window or display string");

		const int updatesBeforeOwner = display.updates;
		const int drawsBeforeOwner = display.draws;
		{
			SinglePlayerLoadScreen screen;
			screen.init(NULL);
			GameWindow *rootWindow = windowManager->winGetWindowList();
			GameWindow *progressWindow = windowManager->winGetWindowFromId(rootWindow,
				nameKeys.nameToKey(AsciiString("SinglePlayerLoadScreen.wnd:ProgressLoad")));
			GameWindow *percentWindow = windowManager->winGetWindowFromId(rootWindow,
				nameKeys.nameToKey(AsciiString("SinglePlayerLoadScreen.wnd:Percent")));
			GameWindow *unitWindow = windowManager->winGetWindowFromId(rootWindow,
				nameKeys.nameToKey(AsciiString("SinglePlayerLoadScreen.wnd:StaticTextCameoText0")));
			GameWindow *locationWindow = windowManager->winGetWindowFromId(rootWindow,
				nameKeys.nameToKey(AsciiString("SinglePlayerLoadScreen.wnd:StaticTextCameoText3")));
			GameWindow *backgroundWindow = windowManager->winGetWindowFromId(rootWindow,
				nameKeys.nameToKey(AsciiString("SinglePlayerLoadScreen.wnd:ParentSinglePlayerLoadScreen")));
			check(rootWindow != NULL && progressWindow != NULL && percentWindow != NULL && unitWindow != NULL &&
				locationWindow != NULL && backgroundWindow != NULL,
				"SinglePlayer owner did not retain every required generated source node");
			check(backgroundWindow && backgroundWindow->winGetEnabledImage(0) ==
				mappedImages.findImageByName(AsciiString("MissionLoad_USA")) &&
				progressWindow && progressWindow->winGetEnabledImage(6) ==
				mappedImages.findImageByName(AsciiString("LoadingBar_ProgressCenter2")),
				"SinglePlayer owner did not bind generated USA mapped images through the source route");
			check(unitWindow && GadgetStaticTextGetText(unitWindow) == gameText.fetch("GeneratedUnit") &&
				locationWindow && GadgetStaticTextGetText(locationWindow) == gameText.fetch("GeneratedLocation"),
				"SinglePlayer owner did not translate generated mission text through source gadgets");
			screen.update(25);
			UnicodeString expectedPercent;
			expectedPercent.format(u"%d%%", 42);
			check(progressWindow && progressWindow->winGetUserData() == reinterpret_cast<void *>(42) &&
				percentWindow && GadgetStaticTextGetText(percentWindow) == expectedPercent && mouse.tooltipEmpty(),
				"SinglePlayer owner update did not apply its generated progress/text/mouse state");
			check(audio.activeCount() == 1 && audio.removed == audioRemovedBeforeOwner + generation && videoPlayer.opens == (generation + 1) * 2,
				"SinglePlayer owner did not retry to one ambient/video owner after injected failure");
			check(display.updates > updatesBeforeOwner && display.draws > drawsBeforeOwner,
				"SinglePlayer owner update did not reach the source presentation route");
		}
		windowManager->drainDestroyedWindows();
		check(audio.activeCount() == 0 && windowManager->winGetWindowList() == NULL &&
			displayStrings.count() == displayStringBaseline,
			"SinglePlayer owner destruction did not release generated audio/window/text ownership");
		TheCampaignManager = NULL;
	}
	check(TheCampaignManager == NULL && TheVideoPlayer == &videoPlayer && TheMappedImageCollection == &mappedImages,
		"SinglePlayer owner re-entry retained or replaced a generated provider publication");
	check(windowManager->winGetWindowList() == NULL && displayStrings.count() == displayStringBaseline &&
		TheCampaignManager == NULL && TheGameInfo == NULL && ThePlayerTemplateStore == NULL &&
		TheMultiplayerSettings == NULL && TheMapCache == NULL && TheGameState == NULL &&
		TheMappedImageCollection == &mappedImages,
		"single-player to multiplayer mode transition retained source UI or provider ownership");
	const std::filesystem::path multiplayerIni = input / "generated-multiplayer-loadscreen.ini";
	{
		std::ofstream generated(multiplayerIni);
		generated << "PlayerTemplate FactionAmerica\n"
			" Side = GeneratedAmerica\n"
			" BaseSide = GeneratedAmerica\n"
			" PlayableSide = Yes\n"
			" PreferredColor = R:17 G:34 B:51\n"
			" StartingBuilding = GeneratedBuilding\nEND\n"
			"MultiplayerSettings\n ShowRandomPlayerTemplate = No\n ShowRandomStartPos = No\n ShowRandomColor = No\nEND\n"
			"MultiplayerColor GeneratedColor\n TooltipName = GeneratedColor\n"
			" RGBColor = R:17 G:34 B:51\n RGBNightColor = R:3 G:2 B:1\nEND\n"
			"MapCache Maps\\Generated.map\n isOfficial = No\n isMultiplayer = Yes\n"
			" extentMin = X:0 Y:0 Z:0\n extentMax = X:100 Y:200 Z:0\n numPlayers = 2\n"
			" Player_1_Start = X:10 Y:20 Z:0\n Player_2_Start = X:30 Y:40 Z:0\nEND\n";
	}
	check(windowManager->winCreateFromScript("Menus/MissingMultiplayerLoadScreen.wnd") == NULL,
		"missing generated multiplayer layout was accepted");
	for (int generation = 0; generation != 2; ++generation)
	{
		check(TheGameInfo == NULL && ThePlayerTemplateStore == NULL && TheMultiplayerSettings == NULL &&
			TheMapCache == NULL && TheGameState == NULL,
			"stale generated multiplayer owner/provider survived a generation boundary");
		PlayerTemplateStore templates;
		templates.init();
		ChallengeGenerals challenges;
		MapCache mapCache;
		GameState gameState;
		GameInfo game;
		GameSlot ownedSlots[MAX_SLOTS];
		ImageCollection multiplayerImages;
		auto addMultiplayerImage = [&](const char *name) {
			Image *image = newInstance(Image);
			check(image != NULL, "generated multiplayer mapped-image fixture did not allocate");
			if (image) { image->setName(AsciiString(name)); multiplayerImages.addImage(image); }
		};
		ThePlayerTemplateStore = &templates;
		TheChallengeGenerals = &challenges;
		TheMapCache = &mapCache;
		TheGameState = &gameState;
		TheGameInfo = &game;
		TheMappedImageCollection = &multiplayerImages;
		addMultiplayerImage("LoadingBar_ProgressCenter0");
		addMultiplayerImage("SAFactionLogoLg_US");
		Bool loaded = TRUE;
		try { INI ini; ini.load(AsciiString(multiplayerIni.string().c_str()), INI_LOAD_OVERWRITE, NULL); }
		catch (...) { loaded = FALSE; }
		check(loaded && TheMultiplayerSettings != NULL && templates.getPlayerTemplateCount() == 1 &&
			mapCache.findMap("Maps\\Generated.map") != NULL,
			"generated multiplayer source providers did not parse before owner construction");
		game.init();
		game.setLocalIP(0x7f000001);
		game.enterGame();
		GameSlot local;
		local.setState(SLOT_PLAYER);
		local.setIP(0x7f000001);
		local.setPlayerTemplate(0);
		local.setColor(0);
		local.setStartPos(0);
		local.setTeamNumber(0);
		UnicodeString localName;
		localName.translate(AsciiString("GeneratedLocal"));
		local.setName(localName);
		GameSlot ai;
		ai.setState(SLOT_EASY_AI);
		ai.setPlayerTemplate(0);
		ai.setColor(0);
		ai.setStartPos(1);
		ai.setTeamNumber(1);
		for (Int i = 0; i < MAX_SLOTS; ++i)
		{
			GameSlot closed;
			closed.setState(SLOT_CLOSED);
			game.setSlotPointer(i, &ownedSlots[i]);
			game.setSlot(i, closed);
		}
		game.setSlot(0, local);
		game.setSlot(1, ai);
		game.setMap("Maps\\Generated.map");
		check(game.getLocalSlotNum() == 0 && game.getSlot(0) && game.getSlot(1) && game.getSlot(1)->isAI(),
			"generated multiplayer GameInfo did not publish its local/AI slots");
		const Int windowsBeforeOwner = displayStrings.count();
		{
			MultiPlayerLoadScreen screen;
			screen.init(&game);
			GameWindow *rootWindow = windowManager->winGetWindowList();
			GameWindow *localProgress = windowManager->winGetWindowFromId(rootWindow,
				nameKeys.nameToKey(AsciiString("MultiplayerLoadScreen.wnd:ProgressLoad0")));
			GameWindow *aiProgress = windowManager->winGetWindowFromId(rootWindow,
				nameKeys.nameToKey(AsciiString("MultiplayerLoadScreen.wnd:ProgressLoad1")));
			GameWindow *localNameWindow = windowManager->winGetWindowFromId(rootWindow,
				nameKeys.nameToKey(AsciiString("MultiplayerLoadScreen.wnd:StaticTextPlayer0")));
			GameWindow *previewWindow = windowManager->winGetWindowFromId(rootWindow,
				nameKeys.nameToKey(AsciiString("MultiplayerLoadScreen.wnd:WinMapPreview")));
			check(rootWindow && localProgress && aiProgress && localNameWindow && previewWindow,
				"original multiplayer owner did not retain its generated source nodes");
			check(localProgress && localProgress->winGetEnabledImage(6) == multiplayerImages.findImageByName("LoadingBar_ProgressCenter0"),
				"original multiplayer owner did not bind the generated house progress image");
			check(localNameWindow && GadgetStaticTextGetText(localNameWindow) == localName,
				"original multiplayer owner did not publish the generated local player name");
			check(aiProgress && aiProgress->winIsHidden(),
				"original multiplayer owner did not hide the generated AI progress bar");
			check(previewWindow && previewWindow->winGetEnabledImage(0) == multiplayerImages.findImageByName("maps_maps_generated"),
				"original multiplayer owner did not bind the generated map preview");
			screen.update(37);
			check(mouse.tooltipEmpty(),
				"original multiplayer offline-null update did not clear the source mouse tooltip");
			screen.processProgress(1, 63);
			check(aiProgress->winGetUserData() == reinterpret_cast<void *>(63),
				"original multiplayer mapped AI progress did not reach its source progress bar");
			GameWindow *foreignProgress = windowManager->winGetWindowFromId(rootWindow,
				nameKeys.nameToKey(AsciiString("MultiplayerLoadScreen.wnd:ProgressLoad2")));
			check(foreignProgress && foreignProgress->winIsHidden() && foreignProgress->winGetUserData() == NULL,
				"unoccupied generated multiplayer slot did not remain inactive");
			screen.reset();
			windowManager->winDestroy(rootWindow);
		}
		windowManager->drainDestroyedWindows();
		check(windowManager->winGetWindowList() == NULL && displayStrings.count() == windowsBeforeOwner &&
			multiplayerImages.findImageByName("maps_maps_generated") != NULL && audio.deviceOpens == 0,
			"original multiplayer owner reset/destructor did not release UI ownership or retained a physical device");
		delete TheMultiplayerSettings;
		TheMultiplayerSettings = NULL;
		TheGameInfo = NULL;
		TheGameState = NULL;
		TheMapCache = NULL;
		TheChallengeGenerals = NULL;
		ThePlayerTemplateStore = NULL;
		TheMappedImageCollection = &mappedImages;
	}
	check(TheGameInfo == NULL && ThePlayerTemplateStore == NULL && TheMultiplayerSettings == NULL &&
		TheMapCache == NULL && TheGameState == NULL,
		"generated multiplayer owner left a stale singleton after two generations");
	TheMouse = NULL;

	for (int generation = 0; generation != 2; ++generation)
	{
		BaseLoadScreenProbe loadScreen;
		presentation_step = 0;
		presentation_tracking = true;
		loadScreen.present(0);
		presentation_tracking = false;
		check(presentation_step == 4, "load-screen source presentation did not complete");
		loadScreen.reset();
	}
	Display *savedPresentationDisplay = TheDisplay;
	TheDisplay = NULL;
	Bool missingPresentationRejected = FALSE;
	try { BaseLoadScreenProbe loadScreen; loadScreen.present(0); }
	catch (const std::runtime_error &) { missingPresentationRejected = TRUE; }
	TheDisplay = savedPresentationDisplay;
	check(missingPresentationRejected, "load-screen missing display did not fail closed");
	const int presentationUpdates = display.updates;
	const int presentationDraws = display.draws;

	for (int frame = 0; frame != 2; ++frame) {
		engine->GameEngine::update();
	}
	check(logic.getFrame() == 2, "actual GameLogic frame did not advance twice");
	check(client.getFrame() == 1, "actual GameClient did not observe the source logic frame");
	check(client_ticks == 2, "actual MessageStream did not propagate both frame ticks");
	check(TheWritableGlobalData->m_playIntro && !TheWritableGlobalData->m_afterIntro,
		"headless client unexpectedly entered the unavailable Shell/movie branch");
	check(scripts.updates == 2 && terrain.updates == 2 && ai.updates == 2 && build.updates == 2 &&
		partition->updates == 2 && victory.updates == 2, "logic service update chain was incomplete");
	check(NetworkInterface::createNetwork() == NULL && TheNetwork == NULL,
		"unsupported Linux network entry did not fail offline-null");
	check(display.updates == presentationUpdates + 2 && display.draws == presentationDraws + 2, "headless display edge did not receive client updates");
	check(audio.deviceOpens == 0, "headless lifecycle acquired a physical audio device");
	TheSubsystemList = new SubsystemInterfaceList;
	WindowLayout *unsupportedCallbackLayout = windowManager->winCreateLayout("Menus/CallbackWindow.wnd");
	check(unsupportedCallbackLayout == NULL,
		"callback-bearing WND did not fail at the headless lexicon boundary");
	WindowLayout *coldBlankLayout = windowManager->winCreateLayout("Menus/BlankWindow.wnd");
	check(coldBlankLayout != NULL && coldBlankLayout->getFirstWindow() != NULL,
		"owned BlankWindow fixture did not create an actual window resource");
	if (coldBlankLayout)
	{
		coldBlankLayout->destroyWindows();
		coldBlankLayout->deleteInstance();
	}
	check(windowManager->winGetWindowList() == NULL,
		"cold BlankWindow resource did not detach from the actual manager");
	for (int generation = 0; generation != 2; ++generation)
	{
		WindowLayout *generatedTree = windowManager->winCreateLayout("Menus/GeneratedSinglePlayerTree.wnd");
		check(generatedTree != NULL && generatedTree->getFirstWindow() != NULL,
			"generated named SinglePlayer tree did not parse through the original layout owner");
		if (generatedTree)
		{
			GameWindow *rootWindow = generatedTree->getFirstWindow();
			GameWindow *progressWindow = windowManager->winGetWindowFromId(rootWindow,
				nameKeys.nameToKey(AsciiString("GeneratedSinglePlayer:Progress")));
			GameWindow *objectiveWindow = windowManager->winGetWindowFromId(rootWindow,
				nameKeys.nameToKey(AsciiString("GeneratedSinglePlayer:Objective")));
			check(progressWindow != NULL && objectiveWindow != NULL,
				"generated named SinglePlayer child lookup lost original NameKey identity");
			GadgetProgressBarSetProgress(progressWindow, 37);
			check(progressWindow && progressWindow->winGetUserData() == reinterpret_cast<void *>(37),
				"generated progress gadget did not retain its source state");
			Mission *generatedMission = newInstance(Mission);
			check(generatedMission != NULL, "generated mission label fixture did not allocate");
			if (generatedMission)
				generatedMission->m_missionObjectivesLabel[0].set("GeneratedObjectiveLabel");
			UnicodeString translated = gameText.fetch(generatedMission ?
				generatedMission->m_missionObjectivesLabel[0] : AsciiString::TheEmptyString);
			GadgetStaticTextSetText(objectiveWindow, translated);
			check(GadgetStaticTextGetText(objectiveWindow) == translated,
				"generated mission label did not reach source static TextData");
			UnicodeString replacement;
			replacement.translate(AsciiString("GeneratedReplacementLabel"));
			GadgetStaticTextSetText(objectiveWindow, replacement);
			check(GadgetStaticTextGetText(objectiveWindow) == replacement,
				"duplicate generated static label did not replace source text");
			GadgetStaticTextSetText(objectiveWindow, UnicodeString::TheEmptyString);
			check(GadgetStaticTextGetText(objectiveWindow).getLength() == 0,
				"empty generated label did not clear source static text");
			GadgetStaticTextSetText(NULL, replacement);
			if (generatedMission) generatedMission->deleteInstance();
			generatedTree->destroyWindows();
			windowManager->drainDestroyedWindows();
			generatedTree->deleteInstance();
		}
		check(windowManager->winGetWindowList() == NULL && displayStrings.count() == displayStringBaseline,
			"generated static TextData retained a window or display string after destruction");
	}
	const std::size_t allocationsBeforeReset = zh::original_process::live_raw_allocations();
	engine->GameEngine::reset();
	check(windowManager->winGetWindowList() == NULL,
		"actual BlankWindow resource remained linked after GameEngine reset");
	check(zh::original_process::live_raw_allocations() == allocationsBeforeReset,
		"BlankWindow reset did not release its exact raw allocation count");

	TheGameEngine = NULL;
	TheCDManager = NULL;
	TheAudio = NULL;
	delete radar;
	TheRadar = NULL;
	TheGameLogic = NULL;
	TheVictoryConditions = NULL;
	TheLocomotorStore = NULL;
	TheWeaponStore = NULL;
	TheRecorder = NULL;
	delete inGameUI;
	TheInGameUI = NULL;
	TheEva = NULL;
	TheAnim2DCollection = NULL;
	delete partition;
	ThePartitionManager = NULL;
	TheBuildAssistant = NULL;
	TheAI = NULL;
	TheTerrainLogic = NULL;
	TheScriptEngine = NULL;
	TheTacticalView = NULL;
	TheCommandList = NULL;
	TheMessageStream = NULL;
	delete players;
	ThePlayerList = NULL;
	TheNameKeyGenerator = NULL;
	TheRankInfoStore = NULL;
	TheGameClient = NULL;
	TheVideoPlayer = NULL;
	TheMappedImageCollection = NULL;
	TheMouse = NULL;
	TheDisplay = NULL;
	TheGameText = NULL;
	delete windowManager;
	TheWindowManager = NULL;
	TheDisplayStringManager = NULL;
	TheHeaderTemplateManager = NULL;
	TheFileSystem = NULL;
	TheLocalFileSystem = NULL;
	delete engine;
	delete TheWritableGlobalData;
	messages.reset();
	commands.reset();
	}
	zh::original_process::shutdown_services();
	shutdownMemoryManager();
	const auto services = zh::original_process::service_counts();
	check(services.synchronization == 0 && services.logging == 0 && services.version == 0 && services.workers == 0,
		"process services retained live ownership");
	std::filesystem::remove_all(root);
	std::printf("M20 original headless update: %s frames=2 ticks=2 network=offline-null devices=0\n",
		failures ? "failed" : "ok");
	std::printf("M22 original multiplayer loadscreen: %s generations=2 windows=0 display_strings=0 pixels=0\n",
		failures ? "failed" : "ok");
	std::printf("M22 original mode-exclusive loadscreen aggregate: %s singleplayer_generations=2 multiplayer_generations=2 windows=0 display_strings=0 pixels=0\n",
		failures ? "failed" : "ok");
	return failures ? 1 : 0;
}
