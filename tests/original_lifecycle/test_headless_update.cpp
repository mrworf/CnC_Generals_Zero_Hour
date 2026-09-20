#include "PreRTS.h"
#include "Common/BuildAssistant.h"
#include "Common/CDManager.h"
#include "Common/GameAudio.h"
#include "Common/GameEngine.h"
#include "Common/FileSystem.h"
#include "Common/FunctionLexicon.h"
#include "Common/GlobalData.h"
#include "Common/MessageStream.h"
#include "Common/NameKeyGenerator.h"
#include "Common/PlayerList.h"
#include "Common/Recorder.h"
#include "Common/Radar.h"
#include "GameClient/Display.h"
#include "GameClient/Anim2D.h"
#include "GameClient/Eva.h"
#include "GameClient/GameClient.h"
#include "GameClient/GameText.h"
#include "GameClient/InGameUI.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GUICallbacks.h"
#include "GameClient/HeaderTemplate.h"
#include "GameClient/View.h"
#include "GameLogic/AI.h"
#include "GameLogic/GameLogic.h"
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

void check(bool condition, const char *message)
{
	if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}

class HeadlessDisplay final : public Display
{
public:
	HeadlessDisplay() { setWidth(800); setHeight(600); }
	void update() override { ++updates; }
	void draw() override { ++draws; }
	Bool isMoviePlaying() override { return TRUE; }
	void doSmartAssetPurgeAndPreload(const char *) override {}
#if defined(_DEBUG) || defined(_INTERNAL)
	void dumpAssetUsage(const char *) override {}
	void dumpModelAssets(const char *) override {}
#endif
	VideoBuffer *createVideoBuffer() override { return NULL; }
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
	Bool isObjectPlayingVoice(UnsignedInt) const override { return FALSE; }
	void adjustVolumeOfPlayingAudio(AsciiString, Real) override {}
	void removePlayingAudio(AsciiString) override {}
	void removeAllDisabledAudio() override {}
	Bool has3DSensitiveStreamsPlaying() const override { return FALSE; }
	void *getHandleForBink() override { return NULL; }
	void releaseHandleForBink() override {}
	void friend_forcePlayAudioEventRTS(const AudioEventRTS *) override {}
	void setPreferredProvider(AsciiString) override {}
	void setPreferredSpeaker(AsciiString) override {}
	Real getFileLengthMS(AsciiString) const override { return 0.0f; }
	void closeAnySamplesUsingFile(const void *) override {}
	int deviceOpens = 0;
protected:
	void setDeviceListenerPosition() override {}
};

class HeadlessCDManager final : public CDManager
{
protected:
	CDDriveInterface *createDrive() override { return new CDDrive; }
};

class HeadlessEngine final : public GameEngine
{
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
	std::filesystem::create_directories(input / "Menus");
	{
		std::ofstream blank(input / "Menus/BlankWindow.wnd");
		blank << "FILE_VERSION = 2\n"
			"STARTLAYOUTBLOCK\nENDLAYOUTBLOCK\n"
			"WINDOW\nWINDOWTYPE = USER;\n"
			"SCREENRECT = UPPERLEFT: 0 0 BOTTOMRIGHT: 800 600 CREATIONRESOLUTION: 800 600;\n"
			"NAME = \"BlankWindow\";\nSTATUS = ENABLED IMAGE;\nSTYLE = USER;\nEND\n";
	}
	{
		std::ofstream callback(input / "Menus/CallbackWindow.wnd");
		callback << "FILE_VERSION = 2\nSTARTLAYOUTBLOCK\n"
			"LAYOUTINIT = MainMenuInit;\nENDLAYOUTBLOCK\n";
	}
	char diagnostic[128]{};
	check(zh::original_process::initialize_services(0, diagnostic, sizeof(diagnostic)), diagnostic);
	initMemoryManager();
	{
	TheWritableGlobalData = new GlobalData;
	TheWritableGlobalData->m_playIntro = TRUE;
	TheWritableGlobalData->m_playSizzle = FALSE;
	HeadlessDisplay display;
	HeadlessClient client;
	NameKeyGenerator nameKeys;
	TheNameKeyGenerator = &nameKeys;
	nameKeys.init();
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
	PosixLocalFileSystem localFiles(input);
	FileSystem files;
	TheLocalFileSystem = &localFiles;
	TheFileSystem = &files;
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
	HeadlessCDManager cdManager;
	HeadlessEngine *engine = new HeadlessEngine;

	TheDisplay = &display;
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
	check(display.updates == 2 && display.draws == 2, "headless display edge did not receive client updates");
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
	TheDisplay = NULL;
	TheGameText = NULL;
	delete windowManager;
	TheWindowManager = NULL;
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
	return failures ? 1 : 0;
}
