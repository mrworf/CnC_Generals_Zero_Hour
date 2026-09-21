#include "PreRTS.h"

#include "LinuxBIGArchive.h"
#include "Common/GameAudio.h"
#include "Common/GameEngine.h"
#include "Common/GameState.h"
#include "Common/FunctionLexicon.h"
#include "Common/GlobalData.h"
#if defined(ZH_M22_FULL_DRAW_TEST)
#include "Common/DrawModule.h"
#endif
#include "Common/ModuleFactory.h"
#include "Common/NameKeyGenerator.h"
#include "Common/OriginalMapLoader.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"
#include "Common/PlayerTemplate.h"
#include "Common/Recorder.h"
#include "Common/RandomValue.h"
#include "Common/XferCRC.h"
#include "Common/AudioRandomValue.h"
#include "W3DDevice/Common/W3DModuleFactory.h"
#if defined(ZH_M22_FULL_DRAW_TEST)
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DFileSystem.h"
#include "W3DDevice/GameClient/Module/W3DModelDraw.h"
#include "W3DDevice/GameClient/Module/W3DDependencyModelDraw.h"
#include "W3DDevice/GameClient/Module/W3DSupplyDraw.h"
#include "W3DDevice/GameClient/Module/W3DTankDraw.h"
#include "W3DDevice/GameClient/Module/W3DTankTruckDraw.h"
#include "W3DDevice/GameClient/Module/W3DTruckDraw.h"
#include "W3DDevice/GameClient/Module/W3DOverlordAircraftDraw.h"
#include "W3DDevice/GameClient/Module/W3DOverlordTankDraw.h"
#include "W3DDevice/GameClient/Module/W3DOverlordTruckDraw.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "WW3D2/RendObj.h"
#include "WW3D2/HLod.h"
#endif
#include "Common/Radar.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameClient/Display.h"
#include "GameClient/DisplayString.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameClient.h"
#include "GameClient/GameFont.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/InGameUI.h"
#include "GameClient/Keyboard.h"
#include "GameClient/MapUtil.h"
#include "GameClient/Mouse.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/Snow.h"
#include "GameClient/TerrainVisual.h"
#include "GameClient/VideoPlayer.h"
#include "GameClient/View.h"
#include "GameNetwork/GameInfo.h"
#include "GameClient/ClientRandomValue.h"
#include "GameLogic/AI.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#if defined(ZH_M22_FULL_DRAW_TEST)
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#endif
#include "GameLogic/Object.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/SidesList.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/VictoryConditions.h"
#include "PosixDevice/Common/PosixLocalFileSystem.h"

#include <cstdlib>
#include <array>
#include <filesystem>
#include <stdexcept>
#if defined(ZH_M22_FULL_DRAW_TEST)
#include <string>
#include <typeinfo>
#endif
#include <vector>

namespace {

#if defined(ZH_M22_FULL_DRAW_TEST)
// The material probe is a distinct TU compiled under the canonical WW3D CPU
// layout; this GameClient host TU only passes the opaque RenderObj owner.
extern "C" void zh_probe_retail_material_families(RenderObjClass *object);
#endif

extern "C" UnsignedInt zh_original_ai_update_count();
extern "C" UnsignedInt zh_original_script_engine_update_count();

struct LifecycleReport
{
	UnsignedInt firstLogicBefore = 0;
	UnsignedInt firstLogicAfter = 0;
	UnsignedInt firstClientBefore = 0;
	UnsignedInt firstClientAfter = 0;
	UnsignedInt logicAfterReset = 0;
	UnsignedInt clientAfterReset = 0;
	UnsignedInt finalLogic = 0;
	UnsignedInt finalClient = 0;
	Bool resetRan = FALSE;
	Bool complete = FALSE;
};

LifecycleReport g_lifecycleReport;
struct ScenarioSetupReport
{
	UnsignedInt mode = GAME_NONE;
	UnsignedInt players = 0;
	UnsignedInt teams = 0;
	UnsignedInt objects = 0;
	UnsignedInt props = 0;
	UnsignedInt modelPreloads = 0;
	UnsignedInt texturePreloads = 0;
	UnsignedInt recorderControls = 0;
	Bool complete = FALSE;
};
ScenarioSetupReport g_scenarioSetupReport;
#if defined(ZH_M22_FULL_DRAW_TEST)
UnsignedInt g_logicSupplyBones = 0;
UnsignedInt g_clientBeforeLogicBones = 0;
#endif
struct SimulationReport
{
	UnsignedInt frameBefore = 0;
	UnsignedInt frameAfter = 0;
	UnsignedInt aiUpdates = 0;
	UnsignedInt scriptUpdates = 0;
	Int startX = 0;
	Int movedX = 0;
	ObjectID actor = INVALID_ID;
	ObjectID target = INVALID_ID;
	Bool moved = FALSE;
	Bool attacked = FALSE;
	Bool invalidRejected = FALSE;
	Bool terminal = FALSE;
	Int targetHealthBefore = 0;
	Int targetHealthAfter = 0;
	Bool complete = FALSE;
};
SimulationReport g_simulationReport;
struct ReentryReport
{
	UnsignedInt resetObjects = 0;
	UnsignedInt resetMapObjects = 0;
	UnsignedInt resetProps = 0;
	UnsignedInt resetModelPreloads = 0;
	UnsignedInt resetTexturePreloads = 0;
	UnsignedInt resetRecorderControls = 0;
	Int firstStartX = 0;
	Int secondStartX = 0;
	UnsignedInt secondObjects = 0;
	UnsignedInt secondProps = 0;
	UnsignedInt secondRecorderControls = 0;
	Bool complete = FALSE;
};
ReentryReport g_reentryReport;
Int g_benchmarkTimer = -1;
UnsignedInt g_deviceAcquisitionAttempts = 0;
UnsignedInt g_propCount = 0;
UnsignedInt g_modelPreloadCount = 0;
UnsignedInt g_texturePreloadCount = 0;
UnsignedInt g_activePropCount = 0;
UnsignedInt g_activeModelPreloadCount = 0;
UnsignedInt g_activeTexturePreloadCount = 0;

class LinuxDisplay final : public Display
{
public:
	LinuxDisplay() { setWidth(800); setHeight(600); }
	void reset() override
	{
		Display::reset();
		m_modelPreloads.clear();
		m_texturePreloads.clear();
		g_activeModelPreloadCount = 0;
		g_activeTexturePreloadCount = 0;
	}
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
	void preloadModelAssets(AsciiString name) override
	{
		m_modelPreloads.push_back(name.str());
		++g_modelPreloadCount;
		g_activeModelPreloadCount = static_cast<UnsignedInt>(m_modelPreloads.size());
	}
	void preloadTextureAssets(AsciiString name) override
	{
		m_texturePreloads.push_back(name.str());
		++g_texturePreloadCount;
		g_activeTexturePreloadCount = static_cast<UnsignedInt>(m_texturePreloads.size());
	}
	void takeScreenShot() override {}
	void toggleMovieCapture() override {}
	void toggleLetterBox() override {}
	void enableLetterBox(Bool) override {}
	Real getAverageFPS() override { return 0.0f; }
	Int getLastFrameDrawCalls() override { return 0; }
private:
	std::vector<std::string> m_modelPreloads;
	std::vector<std::string> m_texturePreloads;
};

class LinuxView final : public View
{
public:
	Drawable *pickDrawable(const ICoord2D *, Bool, PickType) override { return NULL; }
	Int iterateDrawablesInRegion(IRegion2D *, Bool (*)(Drawable *, void *), void *) override { return 0; }
	void forceRedraw() override {}
	const Coord3D& get3DCameraPosition() const override { return m_position; }
	WorldToScreenReturn worldToScreenTriReturn(const Coord3D *, ICoord2D *) override { return WTS_INVALID; }
	void screenToWorld(const ICoord2D *, Coord3D *) override {}
	void screenToTerrain(const ICoord2D *, Coord3D *) override {}
	void screenToWorldAtZ(const ICoord2D *, Coord3D *, Real) override {}
	void drawView() override {}
	void updateView() override {}
	void setGuardBandBias(const Coord2D *) override {}
private:
	Coord3D m_position{};
};

class LinuxInGameUI final : public InGameUI
{
public:
	void draw() override {}
protected:
	View *createView() override { return new LinuxView; }
};

class LinuxGameWindow final : public GameWindow
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(LinuxGameWindow, "W3DGameWindow")
public:
	void winDrawBorder() override {}
};
LinuxGameWindow::~LinuxGameWindow() = default;

class LinuxWindowManager final : public GameWindowManager
{
public:
	GameWindow *allocateNewWindow() override { return newInstance(LinuxGameWindow); }
#define ZH_LINUX_DRAW_GETTER(name) GameWinDrawFunc name() override { return NULL; }
	ZH_LINUX_DRAW_GETTER(getPushButtonImageDrawFunc)
	ZH_LINUX_DRAW_GETTER(getPushButtonDrawFunc)
	ZH_LINUX_DRAW_GETTER(getCheckBoxImageDrawFunc)
	ZH_LINUX_DRAW_GETTER(getCheckBoxDrawFunc)
	ZH_LINUX_DRAW_GETTER(getRadioButtonImageDrawFunc)
	ZH_LINUX_DRAW_GETTER(getRadioButtonDrawFunc)
	ZH_LINUX_DRAW_GETTER(getTabControlImageDrawFunc)
	ZH_LINUX_DRAW_GETTER(getTabControlDrawFunc)
	ZH_LINUX_DRAW_GETTER(getListBoxImageDrawFunc)
	ZH_LINUX_DRAW_GETTER(getListBoxDrawFunc)
	ZH_LINUX_DRAW_GETTER(getComboBoxImageDrawFunc)
	ZH_LINUX_DRAW_GETTER(getComboBoxDrawFunc)
	ZH_LINUX_DRAW_GETTER(getHorizontalSliderImageDrawFunc)
	ZH_LINUX_DRAW_GETTER(getHorizontalSliderDrawFunc)
	ZH_LINUX_DRAW_GETTER(getVerticalSliderImageDrawFunc)
	ZH_LINUX_DRAW_GETTER(getVerticalSliderDrawFunc)
	ZH_LINUX_DRAW_GETTER(getProgressBarImageDrawFunc)
	ZH_LINUX_DRAW_GETTER(getProgressBarDrawFunc)
	ZH_LINUX_DRAW_GETTER(getStaticTextImageDrawFunc)
	ZH_LINUX_DRAW_GETTER(getStaticTextDrawFunc)
	ZH_LINUX_DRAW_GETTER(getTextEntryImageDrawFunc)
	ZH_LINUX_DRAW_GETTER(getTextEntryDrawFunc)
#undef ZH_LINUX_DRAW_GETTER
	void winDrawImage(const Image *, Int, Int, Int, Int, Color) override {}
	void winFillRect(Color, Real, Int, Int, Int, Int) override {}
	void winOpenRect(Color, Real, Int, Int, Int, Int) override {}
	void winDrawLine(Color, Real, Int, Int, Int, Int) override {}
	Color winMakeColor(UnsignedByte r, UnsignedByte g, UnsignedByte b, UnsignedByte a) override { return GameMakeColor(r,g,b,a); }
	const Image *winFindImage(const char *) override { return NULL; }
	Int winFontHeight(GameFont *) override { return 0; }
	Int winIsDigit(Int value) override { return std::isdigit(static_cast<unsigned char>(value)); }
	Int winIsAscii(Int value) override { return value >= 0 && value <= 127; }
	Int winIsAlNum(Int value) override { return std::isalnum(static_cast<unsigned char>(value)); }
	void winFormatText(GameFont *, UnicodeString, Color, Int, Int, Int, Int) override {}
	void winGetTextSize(GameFont *, UnicodeString, Int *w, Int *h, Int) override { if (w) *w=0; if (h) *h=0; }
	GameFont *winFindFont(AsciiString, Int, Bool) override { return NULL; }
};

class LinuxKeyboard final : public Keyboard
{
public:
	Bool getCapsState() override { return FALSE; }
protected:
	void getKey(KeyboardIO *key) override { if (key) key->key = KEY_NONE; }
};

class LinuxMouse final : public Mouse
{
public:
	void initCursorResources() override {}
	void setCursor(MouseCursor cursor) override { m_currentCursor = cursor; }
	void capture() override {}
	void releaseCapture() override {}
protected:
	UnsignedByte getMouseEvent(MouseIO *, Bool) override { return 0; }
};

class LinuxFontLibrary final : public FontLibrary
{
protected:
	Bool loadFontData(GameFont *) override { return FALSE; }
};

class LinuxDisplayString final : public DisplayString
{
	MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(LinuxDisplayString, "LinuxDisplayString", 8, 8)
public:
	void setWordWrap(Int width) override { m_wrap = width; }
	void setWordWrapCentered(Bool centered) override { m_centered = centered; }
	void draw(Int, Int, Color, Color) override {}
	void draw(Int, Int, Color, Color, Int, Int) override {}
	void getSize(Int *width, Int *height) override
	{
		if (width) *width = getWidth();
		if (height) *height = 16;
	}
	Int getWidth(Int charPos = -1) override
	{
		const Int count = charPos < 0 ? getTextLength() : std::min(charPos, getTextLength());
		return count * 8;
	}
	void setUseHotkey(Bool useHotkey, Color color) override
	{
		m_useHotkey = useHotkey;
		m_hotkeyColor = color;
	}
private:
	Int m_wrap = 0;
	Bool m_centered = FALSE;
	Bool m_useHotkey = FALSE;
	Color m_hotkeyColor = 0;
};
LinuxDisplayString::~LinuxDisplayString() = default;

class LinuxDisplayStringManager final : public DisplayStringManager
{
public:
	~LinuxDisplayStringManager() override
	{
		while (m_stringList)
			freeDisplayString(m_stringList);
	}
	DisplayString *newDisplayString() override
	{
		DisplayString *value = newInstance(LinuxDisplayString);
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
};

class LinuxTerrainVisual final : public TerrainVisual
{
public:
	~LinuxTerrainVisual() override { reset(); }
	void reset() override
	{
		TerrainVisual::reset();
		m_props.clear();
		g_activePropCount = 0;
	}
	void getTerrainColorAt(Real, Real, RGBColor *color) override { if (color) *color = RGBColor{}; }
	TerrainType *getTerrainTile(Real, Real) override { return NULL; }
	void enableWaterGrid(Bool) override {}
	void setWaterGridHeightClamps(const WaterHandle *, Real, Real) override {}
	void setWaterAttenuationFactors(const WaterHandle *, Real, Real, Real, Real) override {}
	void setWaterTransform(const WaterHandle *, Real, Real, Real, Real) override {}
	void setWaterTransform(const Matrix3D *) override {}
	void getWaterTransform(const WaterHandle *, Matrix3D *) override {}
	void setWaterGridResolution(const WaterHandle *, Real, Real, Real) override {}
	void getWaterGridResolution(const WaterHandle *, Real *x, Real *y, Real *size) override { if(x)*x=0; if(y)*y=0; if(size)*size=0; }
	void changeWaterHeight(Real, Real, Real) override {}
	void addWaterVelocity(Real, Real, Real, Real) override {}
	Bool getWaterGridHeight(Real, Real, Real *) override { return FALSE; }
	void setTerrainTracksDetail() override {}
	void setShoreLineDetail() override {}
	void addFactionBib(Object *, Bool, Real) override {}
	void removeFactionBib(Object *) override {}
	void addFactionBibDrawable(Drawable *, Bool, Real) override {}
	void removeFactionBibDrawable(Drawable *) override {}
	void removeAllBibs() override {}
	void removeBibHighlighting() override {}
	void removeTreesAndPropsForConstruction(const Coord3D *, const GeometryInfo&, Real) override {}
	void addProp(const ThingTemplate *thing, const Coord3D *position, Real) override
	{
		if (!thing || !position)
			throw std::runtime_error("invalid terrain prop request");
		m_props.push_back(thing->getName().str());
		++g_propCount;
		g_activePropCount = static_cast<UnsignedInt>(m_props.size());
	}
	void setRawMapHeight(const ICoord2D *, Int) override {}
	Int getRawMapHeight(const ICoord2D *) override { return 0; }
	void replaceSkyboxTextures(const AsciiString *[NumSkyboxTextures], const AsciiString *[NumSkyboxTextures]) override {}
private:
	std::vector<std::string> m_props;
};

class LinuxParticleManager final : public ParticleSystemManager
{
public:
	Int getOnScreenParticleCount() override { return 0; }
	void doParticles(RenderInfoClass &) override {}
	void queueParticleRender() override {}
};

class LinuxSnowManager final : public SnowManager
{
public:
	void update() override {}
};

class LinuxRadar final : public Radar
{
public:
	void draw(Int, Int, Int, Int) override {}
	void clearShroud() override {}
	void setShroudLevel(Int, Int, CellShroudStatus) override {}
};

class LinuxAudio final : public AudioManager
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
	Bool isMusicAlreadyLoaded() const override { return TRUE; }
	Bool hasMusicTrackCompleted(const AsciiString&, Int) const override { return FALSE; }
	AsciiString getMusicTrackName() const override { return AsciiString::TheEmptyString; }
	void openDevice() override
	{
		++g_deviceAcquisitionAttempts;
		throw std::runtime_error("physical audio is unavailable in bounded Linux startup");
	}
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
protected:
	void setDeviceListenerPosition() override {}
};

class LinuxGameClient final : public GameClient
{
public:
	~LinuxGameClient() override
	{
		// GameClient::reset owns and releases every drawable before shutdown.
		// Retail object teardown can later leave a dead dependency link in the
		// intrusive head; never revisit that already-released storage from the
		// base destructor.
		m_drawableList = NULL;
	}
	void createRayEffectByTemplate(const Coord3D *, const Coord3D *, const ThingTemplate *) override {}
	void addScorch(const Coord3D *, Real, Scorches) override {}
	Drawable *friend_createDrawable(const ThingTemplate *thing, DrawableStatus status) override
	{
		return thing ? newInstance(Drawable)(thing, status) : NULL;
	}
	void setTeamColor(Int, Int, Int) override {}
	void adjustLOD(Int) override {}
	void notifyTerrainObjectMoved(Object *) override {}
private:
	Display *createGameDisplay() override { return new LinuxDisplay; }
	InGameUI *createInGameUI() override { return new LinuxInGameUI; }
	GameWindowManager *createWindowManager() override { return new LinuxWindowManager; }
	FontLibrary *createFontLibrary() override { return new LinuxFontLibrary; }
	DisplayStringManager *createDisplayStringManager() override { return new LinuxDisplayStringManager; }
	VideoPlayerInterface *createVideoPlayer() override { return new VideoPlayer; }
	TerrainVisual *createTerrainVisual() override { return new LinuxTerrainVisual; }
	Keyboard *createKeyboard() override { return new LinuxKeyboard; }
	Mouse *createMouse() override { return new LinuxMouse; }
	SnowManager *createSnowManager() override { return new LinuxSnowManager; }
	void setFrameRate(Real) override {}
};

class LinuxTerrainLogic final : public TerrainLogic
{
public:
	~LinuxTerrainLogic() override { reset(); }
	void reset() override
	{
		TerrainLogic::reset();
		m_heights.clear();
		m_mapDX = 0;
		m_mapDY = 0;
		m_boundaries.clear();
		m_activeBoundary = 0;
		if (MapObject::TheMapObjectListPtr)
			MapObject::TheMapObjectListPtr->deleteInstance();
		MapObject::TheMapObjectListPtr = NULL;
		MapObject::getWorldDict()->clear();
	}

	Bool loadMap(AsciiString filename, Bool query) override
	{
		CachedFileInputStream stream;
		if (!stream.open(filename))
		{
			if (query) return FALSE;
			throw std::runtime_error("original map is missing");
		}
		OriginalMapLoader loader;
		try
		{
			if (!loader.load(&stream))
			{
				if (query) return FALSE;
				throw std::runtime_error("original map is malformed");
			}
		}
		catch (...)
		{
			if (query) return FALSE;
			throw;
		}
		m_mapDX = loader.width();
		m_mapDY = loader.height();
		m_borderSize = loader.borderSize();
		m_boundaries.assign(loader.boundaries().begin(), loader.boundaries().end());
		m_activeBoundary = 0;
		m_heights.assign(loader.heights().begin(), loader.heights().end());
		return TerrainLogic::loadMap(filename, query);
	}

	Real getGroundHeight(Real x, Real y, Coord3D *normal = NULL) const override
	{
		if (normal) normal->set(0.0f, 0.0f, 1.0f);
		if (m_heights.empty() || m_mapDX <= 0 || m_mapDY <= 0)
			return 0.0f;
		Int ix = static_cast<Int>(x / MAP_XY_FACTOR);
		Int iy = static_cast<Int>(y / MAP_XY_FACTOR);
		ix = std::max(0, std::min(ix, m_mapDX - 1));
		iy = std::max(0, std::min(iy, m_mapDY - 1));
		return m_heights[static_cast<std::size_t>(iy * m_mapDX + ix)] * MAP_HEIGHT_SCALE;
	}

	Real getLayerHeight(Real x, Real y, PathfindLayerEnum, Coord3D *normal = NULL,
		Bool = TRUE) const override { return getGroundHeight(x, y, normal); }
	Bool isCliffCell(Real, Real) const override { return FALSE; }
	void getExtent(Region3D *extent) const override
	{
		extent->lo.set(0.0f, 0.0f, 0.0f);
		extent->hi.set(m_mapDX * MAP_XY_FACTOR, m_mapDY * MAP_XY_FACTOR, 0.0f);
		if (!m_boundaries.empty())
		{
			extent->hi.x = m_boundaries[m_activeBoundary].x * MAP_XY_FACTOR;
			extent->hi.y = m_boundaries[m_activeBoundary].y * MAP_XY_FACTOR;
		}
		for (UnsignedByte value : m_heights)
			extent->hi.z = std::max(extent->hi.z, value * MAP_HEIGHT_SCALE);
	}
	void getMaximumPathfindExtent(Region3D *extent) const override { getExtent(extent); }
	void getExtentIncludingBorder(Region3D *extent) const override
	{
		const Real border = m_borderSize * MAP_XY_FACTOR;
		extent->lo.set(-border, -border, 0.0f);
		extent->hi.set(m_mapDX * MAP_XY_FACTOR - border,
			m_mapDY * MAP_XY_FACTOR - border, 0.0f);
		for (UnsignedByte value : m_heights)
			extent->hi.z = std::max(extent->hi.z, value * MAP_HEIGHT_SCALE);
	}

private:
	Int m_borderSize = 0;
	std::vector<UnsignedByte> m_heights;
};

class LinuxGameLogic final : public GameLogic
{
protected:
	TerrainLogic *createTerrainLogic() override { return new LinuxTerrainLogic; }
};

class LinuxGameEngine final : public GameEngine
{
public:
	void init(int argc, char *argv[]) override
	{
		GameEngine::init(argc, argv);
		if (m_boundedProfile)
		{
			// The bounded M20 profile is deliberately not a shell or match. Keep
			// normal parsed defaults intact outside this explicit test profile,
			// but prevent retail intro/shell state from enqueueing a new game.
			TheWritableGlobalData->m_shellMapOn = FALSE;
			TheWritableGlobalData->m_playIntro = FALSE;
			TheWritableGlobalData->m_playSizzle = FALSE;
			TheWritableGlobalData->m_afterIntro = FALSE;
		}
		if (m_scenarioProfile)
		{
			const char *map = std::getenv("ZH_M21_MAP");
			if (!map || !*map)
				throw std::runtime_error("ZH_M21_MAP is required");
			TheWritableGlobalData->m_shellMapOn = FALSE;
			TheWritableGlobalData->m_playIntro = FALSE;
			TheWritableGlobalData->m_playSizzle = FALSE;
			TheWritableGlobalData->m_afterIntro = FALSE;
			TheWritableGlobalData->m_preloadAssets = TRUE;
			TheWritableGlobalData->m_preloadEverything = FALSE;
			TheWritableGlobalData->m_mapName = map;
			TheGameLogic->setGameMode(m_scenarioMode);
		}
	}
	void update() override
	{
		if (m_scenarioProfile && !m_scenarioStarted)
		{
			m_scenarioStarted = TRUE;
#if defined(ZH_M22_FULL_DRAW_TEST)
			struct OriginalDrawOwners
			{
				W3DAssetManager *assets = NULL;
				RTS3DScene *scene = NULL;
				W3DFileSystem *fileSystem = NULL;
				explicit OriginalDrawOwners(Bool enabled)
				{
					if (!enabled) return;
					fileSystem = new W3DFileSystem;
					assets = new W3DAssetManager;
					scene = new RTS3DScene;
					W3DDisplay::m_assetManager = assets;
					W3DDisplay::m_3DScene = scene;
				}
				~OriginalDrawOwners()
				{
					W3DDisplay::m_assetManager = NULL;
					W3DDisplay::m_3DScene = NULL;
					if (scene) scene->Release_Ref();
					if (assets) { assets->Free_Assets(); delete assets; }
					delete fileSystem;
				}
			} originalDrawOwners(std::getenv("ZH_M22_DRAW_PROFILE") != NULL);
			try {
			if (const char *retailModel = std::getenv("ZH_M22_RETAIL_MODEL"))
			{
				std::string filename = std::string(retailModel) + ".w3d";
				if (!W3DDisplay::m_assetManager->Load_3D_Assets(filename.c_str()))
					throw std::runtime_error("required original retail W3D preload failed");
				RenderObjClass *retail = W3DDisplay::m_assetManager->Create_Render_Obj(
					retailModel, 1.0f, 0);
				if (!retail) throw std::runtime_error("required original retail W3D model is missing");
				std::printf("original retail W3D model: %s class=%d subobjects=%d\n",
					retailModel, retail->Class_ID(), retail->Get_Num_Sub_Objects());
				zh_probe_retail_material_families(retail);
				retail->Release_Ref();
			}
#endif
			const Bool replayScenario = std::getenv("ZH_M24_RECORD_REPLAY") ||
				std::getenv("ZH_M24_REPLAY_EXISTING");
			if (replayScenario) InitRandom(0);
			if (replayScenario &&
				!TheMapCache->addScenarioMapForReplay(TheGlobalData->m_mapName))
				throw std::runtime_error("original replay scenario map cache registration failed");
			const UnsignedInt replayInitialSeed = GetGameLogicRandomSeed();
			if (replayScenario)
			{
				TheSkirmishGameInfo = NEW SkirmishGameInfo;
				TheSkirmishGameInfo->reset();
				TheSkirmishGameInfo->enterGame();
				TheSkirmishGameInfo->setMap(TheGlobalData->m_mapName);
				TheSkirmishGameInfo->setSeed(replayInitialSeed);
				GameSlot *human = TheSkirmishGameInfo->getSlot(0);
				human->setState(SLOT_PLAYER, UnicodeString(u"playerA"), 1);
				TheSkirmishGameInfo->setLocalIP(1);
				human->setPlayerTemplate(ThePlayerTemplateStore->getTemplateNumByName("FactionPlayerA"));
				human->setColor(0);
				human->setStartPos(0);
			}
			TheGameLogic->startNewGame(FALSE);
			// The original start path is intentionally two-phase: the first call
			// requests/loads the map and the second finishes scenario construction.
			TheGameLogic->startNewGame(FALSE);
#if defined(ZH_M22_FULL_DRAW_TEST)
			if (std::getenv("ZH_M22_DRAW_PROFILE"))
			{
				for (Drawable *drawable = TheGameClient->firstDrawable(); drawable;
					drawable = drawable->getNextDrawable())
				{
					for (DrawModule **module = drawable->getDrawModules(); *module; ++module)
					{
						if (dynamic_cast<W3DSupplyDraw *>(*module))
							g_clientBeforeLogicBones += drawable->getPristineBonePositions(
								"SUPPLY", 1, NULL, NULL, INT_MAX);
					}
				}
				TheGameLogic->update();
			}
#endif
			captureScenarioSetup();
			if (const char *replayName = std::getenv("ZH_M24_REPLAY_EXISTING"))
			{
				const UnsignedInt beforeFrame = TheGameLogic->getFrame();
				const UnsignedInt beforeObjects = TheGameLogic->getObjectCount();
				const UnsignedInt beforeCRC = TheGameLogic->getCRC(CRC_RECALC);
				const RecorderModeType beforeMode = TheRecorder->getMode();
				const Bool accepted = TheRecorder->playbackFile(AsciiString(replayName));
				if (std::getenv("ZH_M24_REPLAY_REJECT"))
				{
					if (accepted || TheGameLogic->getFrame() != beforeFrame ||
						TheGameLogic->getObjectCount() != beforeObjects ||
						TheGameLogic->getCRC(CRC_RECALC) != beforeCRC ||
						TheRecorder->getMode() != beforeMode)
						throw std::runtime_error("invalid original replay changed live source state");
					std::printf("original replay rejected: frame=%u crc=%u mode=%d\n",
						beforeFrame, beforeCRC, beforeMode);
				}
				else
				{
					if (!accepted) throw std::runtime_error("valid original replay was rejected");
					const char *frameText = std::getenv("ZH_M24_REPLAY_EXPECTED_FRAME");
					const char *crcText = std::getenv("ZH_M24_REPLAY_EXPECTED_CRC");
					const char *seedText = std::getenv("ZH_M24_REPLAY_EXPECTED_SEED_CRC");
					if (!frameText || !crcText || !seedText)
						throw std::runtime_error("expected replay checkpoint is required");
					const UnsignedInt expectedFrame = static_cast<UnsignedInt>(std::strtoul(frameText, NULL, 10));
					const UnsignedInt expectedCRC = static_cast<UnsignedInt>(std::strtoul(crcText, NULL, 10));
					const UnsignedInt expectedSeedCRC = static_cast<UnsignedInt>(std::strtoul(seedText, NULL, 10));
					for (UnsignedInt attempt = 0; attempt < expectedFrame + 8 &&
						TheGameLogic->getFrame() < expectedFrame; ++attempt)
						GameEngine::update();
					const UnsignedInt actualCRC = TheGameLogic->getCRC(CRC_RECALC);
					if (TheGameLogic->getFrame() != expectedFrame || actualCRC != expectedCRC ||
						GetGameLogicRandomSeedCRC() != expectedSeedCRC ||
						TheGameLogic->getGameMode() != GAME_REPLAY)
						throw std::runtime_error("separate-process original replay CRC diverged");
					std::printf("original replay existing: frame=%u crc=%u seedcrc=%u mode=%d\n",
						expectedFrame, actualCRC, expectedSeedCRC, TheGameLogic->getGameMode());
				}
				GameEngine::reset();
				setQuitting(TRUE);
				return;
			}
#if defined(ZH_M22_FULL_DRAW_TEST)
			if (std::getenv("ZH_M22_DRAW_PROFILE"))
			{
				UnsignedInt drawn = 0;
				UnsignedInt hlods = 0;
				UnsignedInt animations = 0;
				UnsignedInt supplyTransitions = 0;
				UnsignedInt dependencyBlocks = 0;
				UnsignedInt dependencyReleases = 0;
				UnsignedInt treadScrolls = 0;
				UnsignedInt wheelControls = 0;
				UnsignedInt riderDependencies = 0;
				for (Drawable *drawable = TheGameClient->firstDrawable(); drawable;
					drawable = drawable->getNextDrawable())
				{
					if (Object *actor = drawable->getObject())
					{
						if (PhysicsBehavior *physics = actor->getPhysics())
						{
							Coord3D velocity{18.0f, 0.0f, 0.0f};
							Coord3D force{0.0f, 0.0f, 0.0f};
							physics->addVelocityTo(&velocity);
							physics->scrubVelocity2D(18.0f);
							physics->applyMotiveForce(&force);
							if (physics->getVelocityMagnitude() != 18.0f || !physics->isMotive())
								throw std::runtime_error("original motive physics state missing");
						}
					}
					for (DrawModule **module = drawable->getDrawModules(); *module; ++module)
					{
						std::printf("original full draw module: %s\n", typeid(**module).name());
						if (auto *model = dynamic_cast<W3DModelDraw *>(*module))
						{
							RenderObjClass *render = model->getRenderObject();
							if (render && render->Class_ID() == RenderObjClass::CLASSID_HLOD
								&& render->Get_Num_Sub_Objects() == 2) ++hlods;
						}
					}
					drawable->draw(NULL);
				for (DrawModule **module = drawable->getDrawModules(); *module; ++module)
				{
					if (typeid(**module) == typeid(W3DOverlordTankDraw)
						|| typeid(**module) == typeid(W3DOverlordAircraftDraw)
						|| typeid(**module) == typeid(W3DOverlordTruckDraw))
					{
						Object *owner = drawable->getObject();
						const Object *rider = owner && owner->getContain()
							? owner->getContain()->friend_getRider() : NULL;
						Drawable *riderDraw = rider ? rider->getDrawable() : NULL;
					if (!owner->getContain()) throw std::runtime_error("original Overlord contain missing");
						if (!rider) throw std::runtime_error("original Overlord rider missing");
						if (!riderDraw) throw std::runtime_error("original Overlord rider Drawable missing");
						if (!riderDraw->getDrawModules()[0])
							throw std::runtime_error("original Overlord rider DrawModule missing");
						auto *dependent = dynamic_cast<W3DDependencyModelDraw *>(riderDraw->getDrawModules()[0]);
						RenderObjClass *riderModel = dependent ? dependent->getRenderObject() : NULL;
						if (!riderModel) throw std::runtime_error("original rider dependency model is missing");
						Matrix3D sentinel = *riderDraw->getTransformMatrix();
						sentinel.Translate_X(333.0f);
						riderModel->Set_Transform(sentinel);
						dependent->doDrawModule(riderDraw->getTransformMatrix());
						if (riderModel->Get_Transform().Get_Translation().X != sentinel.Get_Translation().X)
							throw std::runtime_error("original rider dependency prematurely released");
						(*module)->doDrawModule(drawable->getTransformMatrix());
						if (riderModel->Get_Transform().Get_Translation().X == sentinel.Get_Translation().X)
							throw std::runtime_error("original Overlord did not draw its dependent rider");
						++riderDependencies;
					}
					if (auto *dependency = dynamic_cast<W3DDependencyModelDraw *>(*module))
						{
							RenderObjClass *model = dependency->getRenderObject();
							if (!model) throw std::runtime_error("original dependency model missing");
							const Vector3 before = model->Get_Transform().Get_Translation();
							Matrix3D moved = *drawable->getTransformMatrix();
							moved.Translate_X(3.0f);
							const Vector3 target = moved.Get_Translation();
							dependency->doDrawModule(&moved);
							const Vector3 stillBlocked = model->Get_Transform().Get_Translation();
							if (stillBlocked.X != before.X || stillBlocked.Y != before.Y)
								throw std::runtime_error("original dependency draw was not gated");
							++dependencyBlocks;
							dependency->notifyDrawModuleDependencyCleared();
							dependency->doDrawModule(&moved);
							const Vector3 after = model->Get_Transform().Get_Translation();
							if (after.X != target.X || after.Y != target.Y)
								throw std::runtime_error("original dependency gate did not release");
							++dependencyReleases;
						}
						if (auto *tank = dynamic_cast<W3DTankDraw *>(*module))
						{
							RenderObjClass *render = tank->getRenderObject();
							RenderObjClass *tread = render ? render->Get_Sub_Object_By_Name("TREADSL01") : NULL;
							if (!tread || !tread->Get_User_Data())
								throw std::runtime_error("original tank tread material override missing");
							auto *material = static_cast<RenderObjClass::Material_Override *>(tread->Get_User_Data());
							if (material->customUVOffset.X == 0.0f)
								throw std::runtime_error("original motive tread UV scrolling missing");
							++treadScrolls;
							tread->Release_Ref();
						}
						if (auto *wheels = dynamic_cast<W3DTankTruckDraw *>(*module))
						{
							RenderObjClass *model = wheels->getRenderObject();
							int front = model ? model->Get_Bone_Index("TIRE_FL") : 0;
							int rear = model ? model->Get_Bone_Index("TIRE_RL") : 0;
							if (!front || !rear || !model->Is_Bone_Captured(front)
								|| !model->Is_Bone_Captured(rear))
								throw std::runtime_error("original tank-truck wheel control missing");
							++wheelControls;
						}
						if (auto *wheels = dynamic_cast<W3DTruckDraw *>(*module))
						{
							RenderObjClass *model = wheels->getRenderObject();
							int front = model ? model->Get_Bone_Index("TIRE_FL") : 0;
							int rear = model ? model->Get_Bone_Index("TIRE_RL") : 0;
							if (!front || !rear || !model->Is_Bone_Captured(front)
								|| !model->Is_Bone_Captured(rear))
								throw std::runtime_error("original truck wheel control missing");
							++wheelControls;
						}
						if (auto *supply = dynamic_cast<W3DSupplyDraw *>(*module))
						{
							RenderObjClass *model = supply->getRenderObject();
							RenderObjClass *bone = model ? model->Get_Sub_Object_By_Name("SUPPLY01") : NULL;
							if (drawable->getPristineBonePositions("SUPPLY", 1, NULL, NULL, INT_MAX) != 1)
								throw std::runtime_error("original supply pristine bone decision missing");
							if (!bone) throw std::runtime_error("original supply bone is missing");
							supply->updateDrawModuleSupplyStatus(10, 0);
							if (!bone->Is_Hidden()) throw std::runtime_error("original supply hide decision missing");
							supply->updateDrawModuleSupplyStatus(10, 10);
							if (bone->Is_Hidden()) throw std::runtime_error("original supply show decision missing");
							bone->Release_Ref();
							supplyTransitions += 2;
						}
						if (typeid(**module) != typeid(W3DModelDraw)) continue;
						auto *model = static_cast<W3DModelDraw *>(*module);
						if (!model->getRenderObject() ||
							model->getRenderObject()->Class_ID() != RenderObjClass::CLASSID_HLOD) continue;
						float frame = 0, multiplier = 0;
						int numFrames = 0, mode = 0;
						auto *hlod = static_cast<HLodClass *>(model->getRenderObject());
						if (hlod->Peek_Animation_And_Info(frame, numFrames, mode, multiplier)
							&& numFrames == 2) ++animations;
					}
					++drawn;
				}
				std::printf("original full draw: drawables=%u hlods=%u animations=%u "
					"supply-transitions=%u logic-bones=%u client-before-logic-bones=%u "
					"dependency-blocks=%u dependency-releases=%u tread-scrolls=%u wheel-controls=%u rider-dependencies=%u\n",
					drawn, hlods, animations, supplyTransitions,
					g_logicSupplyBones, g_clientBeforeLogicBones,
					dependencyBlocks, dependencyReleases, treadScrolls, wheelControls, riderDependencies);
			}
#endif
			if (m_simulationProfile)
			{
				const Bool recordingReplay = std::getenv("ZH_M24_RECORD_REPLAY") != NULL;
				if (recordingReplay) TheRecorder->beginScenarioRecording(replayInitialSeed);
				if (recordingReplay) runOriginalReplayCommands();
				else runOriginalSimulation();
				if (recordingReplay) TheRecorder->stopRecording();
				if (recordingReplay)
				{
					const auto replayState = []() {
						Int actorX = -1, targetHealth = -1;
						for (Object *object = TheGameLogic->getFirstObject(); object; object = object->getNextObject())
						{
							if (object->getTemplate()->getName() == "LogicFixture")
								actorX = static_cast<Int>(object->getPosition()->x * 1000.0f);
							if (object->getTemplate()->getName() == "EnemyFixture")
								targetHealth = static_cast<Int>(object->getBodyModule()->getHealth() * 1000.0f);
						}
						return std::array<Int, 3>{actorX, targetHealth,
							static_cast<Int>(GetGameLogicRandomSeedCRC())};
					};
					const auto beforeReplay = replayState();
					const UnsignedInt expectedFrame = TheGameLogic->getFrame();
					const UnsignedInt expectedCRC = TheGameLogic->getCRC(CRC_RECALC);
					if (!TheRecorder->playbackFile(AsciiString("00000000.rep")))
						throw std::runtime_error("original replay playback rejected source recording");
					for (UnsignedInt attempt = 0; attempt < expectedFrame + 8 &&
						TheGameLogic->getFrame() < expectedFrame; ++attempt)
						GameEngine::update();
					const UnsignedInt replayCRC = TheGameLogic->getCRC(CRC_RECALC);
					const auto afterReplay = replayState();
					if (TheGameLogic->getFrame() != expectedFrame || replayCRC != expectedCRC ||
						afterReplay[2] != beforeReplay[2] || TheGameLogic->getGameMode() != GAME_REPLAY)
					{
						char detail[320];
						std::snprintf(detail, sizeof(detail),
							"original replay divergence: frame %u>%u crc %u>%u objects=%u mode=%d actorX %d>%d targetHealth %d>%d seedcrc %d>%d",
							expectedFrame, TheGameLogic->getFrame(), expectedCRC, replayCRC,
							TheGameLogic->getObjectCount(), TheGameLogic->getGameMode(),
							beforeReplay[0], afterReplay[0], beforeReplay[1], afterReplay[1],
							beforeReplay[2], afterReplay[2]);
						throw std::runtime_error(detail);
					}
					std::printf("original replay checkpoint: frame=%u crc=%u seedcrc=%u mode=%d\n",
						expectedFrame, replayCRC, static_cast<UnsignedInt>(afterReplay[2]),
						TheGameLogic->getGameMode());
				}
				if (const char *saveName = std::getenv("ZH_M24_SAVE_FILENAME"))
				{
					const Bool loadExisting = std::getenv("ZH_M24_LOAD_EXISTING") != NULL;
					const AsciiString savedLeaf(*saveName ? saveName : "00000000.sav");
					SaveGameInfo savedInfo;
					if (!loadExisting)
					{
						if (const char *testMap = std::getenv("ZH_M24_TEST_EMBEDDED_MAP"))
							TheWritableGlobalData->m_mapName = testMap;
						if (TheGameState->saveGame(AsciiString(saveName), UnicodeString(u"Original scenario"),
							SAVE_FILE_TYPE_NORMAL) != SC_OK)
							throw std::runtime_error("original scenario save failed");
						TheGameState->getSaveGameInfoFromFile(
							TheGameState->getFilePathInSaveDirectory(savedLeaf), &savedInfo);
						if (savedInfo.saveFileType != SAVE_FILE_TYPE_NORMAL ||
							savedInfo.description.getLength() == 0 || savedInfo.mapLabel.isEmpty())
							throw std::runtime_error("original save metadata did not round-trip");
					}
					else savedInfo.saveFileType = SAVE_FILE_TYPE_NORMAL;
					if (std::getenv("ZH_M24_LOAD_AFTER_SAVE"))
					{
						const auto checkpoint = []() {
							return std::array<UnsignedInt, 5>{
								TheGameLogic->getFrame(), TheGameLogic->getObjectCount(),
								static_cast<UnsignedInt>(ThePlayerList->getPlayerCount()),
								static_cast<UnsignedInt>(TheSidesList->getNumTeams()),
								TheGameLogic->getCRC(CRC_RECALC)};
						};
						const auto before = checkpoint();
						const auto components = []() {
							const auto snapshotCRC = [](Snapshot *snapshot) {
								XferCRC crc;
								crc.open("M24");
								crc.xferSnapshot(snapshot);
								return crc.getCRC();
							};
							XferCRC objects;
							objects.open("M24-objects");
							std::array<UnsignedInt, 3> individual{};
							std::size_t objectIndex = 0;
							for (Object *object = TheGameLogic->getFirstObject(); object; object = object->getNextObject())
							{
								objects.xferSnapshot(object);
								if (objectIndex < individual.size()) individual[objectIndex] = snapshotCRC(object);
								++objectIndex;
							}
							return std::array<UnsignedInt, 8>{GetGameLogicRandomSeedCRC(), objects.getCRC(),
								snapshotCRC(ThePartitionManager), snapshotCRC(ThePlayerList), snapshotCRC(TheAI),
								individual[0], individual[1], individual[2]};
						};
						const auto partsBefore = components();
						AvailableGameInfo game;
						game.filename = savedLeaf;
						game.saveGameInfo = savedInfo;
						const SaveCode loadCode = TheGameState->loadGame(game);
						const Bool fault = loadExisting || std::getenv("ZH_M24_TEST_LOAD_FAULT") != NULL;
						if ((fault && loadCode != SC_INVALID_DATA) || (!fault && loadCode != SC_OK))
							throw std::runtime_error("original scenario load returned unexpected status");
						const auto after = checkpoint();
						const auto partsAfter = components();
						if (before != after)
						{
							char detail[768];
							std::snprintf(detail, sizeof(detail),
								"original scenario load changed checkpoint: frame %u>%u objects %u>%u players %u>%u teams %u>%u crc %u>%u seed %u>%u objcrc %u>%u part %u>%u player %u>%u ai %u>%u individual %u>%u %u>%u %u>%u",
								before[0], after[0], before[1], after[1], before[2], after[2],
								before[3], after[3], before[4], after[4],
								partsBefore[0], partsAfter[0], partsBefore[1], partsAfter[1],
								partsBefore[2], partsAfter[2], partsBefore[3], partsAfter[3],
								partsBefore[4], partsAfter[4], partsBefore[5], partsAfter[5],
								partsBefore[6], partsAfter[6], partsBefore[7], partsAfter[7]);
							throw std::runtime_error(detail);
						}
						std::printf("original persistence load: objects=%u players=%u teams=%u crc=%u rollback=%u\n",
							TheGameLogic->getObjectCount(), ThePlayerList->getPlayerCount(),
							TheSidesList->getNumTeams(), after[4], fault);
					}
				}
				if (m_reentryProfile)
					runOriginalReentry();
				GameEngine::reset();
				setQuitting(TRUE);
				return;
			}
			GameEngine::reset();
			setQuitting(TRUE);
			return;
#if defined(ZH_M22_FULL_DRAW_TEST)
			} catch (...) {
				// Preserve the original CPU presentation owners until the original
				// GameClient has released its partially built draw modules.
				if (originalDrawOwners.scene) GameEngine::reset();
				throw;
			}
#endif
		}
		if (m_boundedProfile && m_updates == 0)
		{
			g_benchmarkTimer = TheGlobalData->m_benchmarkTimer;
			g_lifecycleReport.firstLogicBefore = TheGameLogic->getFrame();
			g_lifecycleReport.firstClientBefore = TheGameClient->getFrame();
		}
		GameEngine::update();
		if (!m_boundedProfile) return;
		++m_updates;
		if (m_updates == 1)
		{
			g_lifecycleReport.firstLogicAfter = TheGameLogic->getFrame();
			g_lifecycleReport.firstClientAfter = TheGameClient->getFrame();
			GameEngine::reset();
			g_lifecycleReport.logicAfterReset = TheGameLogic->getFrame();
			g_lifecycleReport.clientAfterReset = TheGameClient->getFrame();
			g_lifecycleReport.resetRan = TRUE;
		}
		if (m_updates >= 2)
		{
			g_lifecycleReport.finalLogic = TheGameLogic->getFrame();
			g_lifecycleReport.finalClient = TheGameClient->getFrame();
			g_lifecycleReport.complete = TRUE;
			setQuitting(TRUE);
		}
	}
	void serviceWindowsOS() override
	{
		++m_services;
	}
protected:
	void captureScenarioSetup()
	{
		g_scenarioSetupReport.mode = m_scenarioMode;
		g_scenarioSetupReport.players = ThePlayerList->getPlayerCount();
		g_scenarioSetupReport.teams = TheSidesList->getNumTeams();
		g_scenarioSetupReport.objects = TheGameLogic->getObjectCount();
		g_scenarioSetupReport.props = g_activePropCount;
		g_scenarioSetupReport.modelPreloads = g_activeModelPreloadCount;
		g_scenarioSetupReport.texturePreloads = g_activeTexturePreloadCount;
		g_scenarioSetupReport.recorderControls = TheRecorder->getControlsInitCount();
		g_scenarioSetupReport.complete = TRUE;
	}
	void appendSelection(PlayerIndex player, ObjectID object)
	{
		GameMessage *message = newInstance(GameMessage)(GameMessage::MSG_CREATE_SELECTED_GROUP);
		message->friend_setPlayerIndex(player);
		message->appendBooleanArgument(TRUE);
		message->appendObjectIDArgument(object);
		TheCommandList->appendMessage(message);
	}
	void appendMove(PlayerIndex player, const Coord3D& destination)
	{
		GameMessage *message = newInstance(GameMessage)(GameMessage::MSG_DO_MOVETO);
		message->friend_setPlayerIndex(player);
		message->appendLocationArgument(destination);
		TheCommandList->appendMessage(message);
	}
	void appendAttack(PlayerIndex player, ObjectID target)
	{
		GameMessage *message = newInstance(GameMessage)(GameMessage::MSG_DO_FORCE_ATTACK_OBJECT);
		message->friend_setPlayerIndex(player);
		message->appendObjectIDArgument(target);
		TheCommandList->appendMessage(message);
	}
	void appendSelfDestruct(PlayerIndex player)
	{
		GameMessage *message = newInstance(GameMessage)(GameMessage::MSG_SELF_DESTRUCT);
		message->friend_setPlayerIndex(player);
		message->appendBooleanArgument(FALSE);
		TheCommandList->appendMessage(message);
	}
	void runOriginalReplayCommands()
	{
		Object *actor = NULL, *target = NULL;
		for (Object *object = TheGameLogic->getFirstObject(); object; object = object->getNextObject())
		{
			if (object->getTemplate()->getName() == "LogicFixture") actor = object;
			if (object->getTemplate()->getName() == "EnemyFixture") target = object;
		}
		if (!actor || !target || !actor->getControllingPlayer())
			throw std::runtime_error("original replay source command actors are unavailable");
		// The source replay start consumes frame zero for MSG_NEW_GAME; network
		// commands recorded on that frame would be discarded during map setup.
		GameEngine::update();
		const PlayerIndex player = actor->getControllingPlayer()->getPlayerIndex();
		g_simulationReport.frameBefore = TheGameLogic->getFrame();
		g_simulationReport.startX = static_cast<Int>(actor->getPosition()->x * 1000.0f);
		g_simulationReport.actor = actor->getID();
		g_simulationReport.target = target->getID();
		appendSelection(player, actor->getID());
		Coord3D destination = *actor->getPosition();
		destination.x += 15.0f;
		appendMove(player, destination);
		for (Int i = 0; i < 12; ++i) GameEngine::update();
		g_simulationReport.movedX = static_cast<Int>(actor->getPosition()->x * 1000.0f);
		g_simulationReport.moved = g_simulationReport.movedX != g_simulationReport.startX;
		g_simulationReport.targetHealthBefore =
			static_cast<Int>(target->getBodyModule()->getHealth() * 1000.0f);
		appendSelection(player, actor->getID());
		appendAttack(player, target->getID());
		for (Int i = 0; i < 4; ++i) GameEngine::update();
		g_simulationReport.targetHealthAfter =
			static_cast<Int>(target->getBodyModule()->getHealth() * 1000.0f);
		g_simulationReport.attacked =
			g_simulationReport.targetHealthAfter < g_simulationReport.targetHealthBefore;
		g_simulationReport.frameAfter = TheGameLogic->getFrame();
		g_simulationReport.aiUpdates = zh_original_ai_update_count();
		g_simulationReport.scriptUpdates = zh_original_script_engine_update_count();
		g_simulationReport.complete = TRUE;
	}
	void runOriginalSimulation()
	{
		Object *actor = NULL;
		Object *target = NULL;
		for (Object *object = TheGameLogic->getFirstObject(); object; object = object->getNextObject())
		{
			const AsciiString& name = object->getTemplate()->getName();
			if (name == "LogicFixture") actor = object;
			if (name == "EnemyFixture") target = object;
		}
		Player *playerA = ThePlayerList->findPlayerWithNameKey(TheNameKeyGenerator->nameToKey("playerA"));
		Player *playerB = ThePlayerList->findPlayerWithNameKey(TheNameKeyGenerator->nameToKey("playerB"));
		if (!actor) throw std::runtime_error("original simulation actor is missing");
		if (!target) throw std::runtime_error("original simulation target is missing");
		if (!playerA) throw std::runtime_error("original simulation playerA is missing");
		if (!playerB) throw std::runtime_error("original simulation playerB is missing");
		if (!actor->getAIUpdateInterface()) throw std::runtime_error("original simulation actor AI is missing");
		Player *actorOwner = actor->getControllingPlayer();
		if (!actorOwner) throw std::runtime_error("original simulation actor owner is missing");

		const unsigned clientBurn = std::getenv("ZH_M21_CLIENT_RANDOM_BURN") ?
			static_cast<unsigned>(std::strtoul(std::getenv("ZH_M21_CLIENT_RANDOM_BURN"), NULL, 10)) : 0;
		const unsigned audioBurn = std::getenv("ZH_M21_AUDIO_RANDOM_BURN") ?
			static_cast<unsigned>(std::strtoul(std::getenv("ZH_M21_AUDIO_RANDOM_BURN"), NULL, 10)) : 0;
		for (unsigned i = 0; i < clientBurn; ++i) (void)GameClientRandomValue(0, 1000000);
		for (unsigned i = 0; i < audioBurn; ++i) (void)GameAudioRandomValue(0, 1000000);

		g_simulationReport.frameBefore = TheGameLogic->getFrame();
		g_simulationReport.startX = static_cast<Int>(actor->getPosition()->x * 1000.0f);
		g_simulationReport.actor = actor->getID();
		g_simulationReport.target = target->getID();
		const PlayerIndex actorPlayer = actorOwner->getPlayerIndex();
		appendSelection(actorPlayer, actor->getID());
		Coord3D destination = *actor->getPosition();
		destination.x += 15.0f;
		appendMove(actorPlayer, destination);
		for (Int frame = 0; frame < 12; ++frame) GameEngine::update();
		AIUpdateInterface *ai = actor->getAIUpdateInterface();
		g_simulationReport.movedX = static_cast<Int>(actor->getPosition()->x * 1000.0f);
		g_simulationReport.moved = g_simulationReport.movedX != g_simulationReport.startX || ai->isMoving();

		ai->aiIdle(CMD_FROM_AI);
		TheGameLogic->selectObject(actor, TRUE, actorOwner->getPlayerMask());
		appendAttack(actorPlayer, INVALID_ID);
		GameEngine::update();
		g_simulationReport.invalidRejected = ai->getLastCommandSource() == CMD_FROM_AI;

		g_simulationReport.targetHealthBefore = static_cast<Int>(target->getBodyModule()->getHealth() * 1000.0f);
		TheGameLogic->selectObject(actor, TRUE, actorOwner->getPlayerMask());
		appendAttack(actorPlayer, target->getID());
		GameEngine::update();
		g_simulationReport.targetHealthAfter = static_cast<Int>(target->getBodyModule()->getHealth() * 1000.0f);
		g_simulationReport.attacked = ai->getLastCommandSource() == CMD_FROM_PLAYER;

		appendSelfDestruct(playerB->getPlayerIndex());
		for (Int frame = 0; frame < 3; ++frame) GameEngine::update();
		g_simulationReport.terminal = m_scenarioMode == GAME_SKIRMISH ?
			TheVictoryConditions->hasAchievedVictory(playerA) : TheVictoryConditions->hasSinglePlayerBeenDefeated(playerB);
		g_simulationReport.frameAfter = TheGameLogic->getFrame();
		g_simulationReport.aiUpdates = zh_original_ai_update_count();
		g_simulationReport.scriptUpdates = zh_original_script_engine_update_count();
		g_simulationReport.complete = TRUE;
	}
	void runOriginalReentry()
	{
		const SimulationReport first = g_simulationReport;
		GameEngine::reset();
		g_reentryReport.resetObjects = TheGameLogic->getObjectCount();
		g_reentryReport.resetMapObjects = MapObject::getFirstMapObject() ? 1U : 0U;
		g_reentryReport.resetProps = g_activePropCount;
		g_reentryReport.resetModelPreloads = g_activeModelPreloadCount;
		g_reentryReport.resetTexturePreloads = g_activeTexturePreloadCount;
		g_reentryReport.resetRecorderControls = TheRecorder->getControlsInitCount();

		const char *map = std::getenv("ZH_M21_REENTRY_MAP");
		if (!map || !*map)
			throw std::runtime_error("ZH_M21_REENTRY_MAP is required");
		TheWritableGlobalData->m_mapName = map;
		m_scenarioMode = GAME_SKIRMISH;
		TheGameLogic->setGameMode(m_scenarioMode);
		TheGameLogic->startNewGame(FALSE);
		TheGameLogic->startNewGame(FALSE);
		captureScenarioSetup();
		g_simulationReport = SimulationReport{};
		runOriginalSimulation();

		g_reentryReport.firstStartX = first.startX;
		g_reentryReport.secondStartX = g_simulationReport.startX;
		g_reentryReport.secondObjects = TheGameLogic->getObjectCount();
		g_reentryReport.secondProps = g_activePropCount;
		g_reentryReport.secondRecorderControls = TheRecorder->getControlsInitCount();
		g_reentryReport.complete = first.complete && g_simulationReport.complete;
	}
	LocalFileSystem *createLocalFileSystem() override
	{
		const char *root = std::getenv("ZH_DATA_ROOT");
		if (!root || !*root) throw std::runtime_error("ZH_DATA_ROOT is required");
		return new PosixLocalFileSystem(std::filesystem::path(root));
	}
	ArchiveFileSystem *createArchiveFileSystem() override { return zh::original_runtime::createLinuxBIGArchiveFileSystem(); }
	GameLogic *createGameLogic() override { return new LinuxGameLogic; }
	GameClient *createGameClient() override { return new LinuxGameClient; }
	ModuleFactory *createModuleFactory() override { return new W3DModuleFactory; }
	ThingFactory *createThingFactory() override { return new ThingFactory; }
	FunctionLexicon *createFunctionLexicon() override { return new FunctionLexicon; }
	Radar *createRadar() override { return new LinuxRadar; }
	WebBrowser *createWebBrowser() override { return NULL; }
	ParticleSystemManager *createParticleSystemManager() override { return new LinuxParticleManager; }
	AudioManager *createAudioManager() override { return new LinuxAudio; }
private:
	Bool m_boundedProfile = std::getenv("ZH_M20_HEADLESS_PROFILE") != NULL;
	const char *m_scenarioName = std::getenv("ZH_M21_SCENARIO");
	Bool m_scenarioProfile = m_scenarioName != NULL;
	Int m_scenarioMode = m_scenarioName && std::strcmp(m_scenarioName, "skirmish") == 0 ?
		GAME_SKIRMISH : GAME_SINGLE_PLAYER;
	Bool m_scenarioStarted = FALSE;
	Bool m_simulationProfile = std::getenv("ZH_M21_SIMULATION") != NULL;
	Bool m_reentryProfile = std::getenv("ZH_M21_REENTRY") != NULL;
	unsigned m_updates = 0;
	unsigned m_services = 0;
};

} // namespace

// Invoked only by the test-build observer inside the original
// GameLogic::update latch. This does not set, bypass, or emulate logic time.
#if defined(ZH_M22_FULL_DRAW_TEST)
void zh_linux_w3d_logic_witness()
{
	if (!std::getenv("ZH_M22_DRAW_PROFILE") || !TheGameClient || !W3DDisplay::m_3DScene)
		return;
	if (!TheGameLogic || !TheGameLogic->isInGameLogicUpdate())
		throw std::runtime_error("original logic-phase draw witness escaped update");
	for (Drawable *drawable = TheGameClient->firstDrawable(); drawable;
		drawable = drawable->getNextDrawable())
	{
		for (DrawModule **module = drawable->getDrawModules(); *module; ++module)
		{
			if (dynamic_cast<W3DSupplyDraw *>(*module))
				g_logicSupplyBones += drawable->getPristineBonePositions(
					"SUPPLY", 1, NULL, NULL, INT_MAX);
		}
	}
}
#endif

// The Linux keyboard is a physical-device edge. It intentionally produces no
// events in the bounded startup profile, while retaining the original
// GameClient ownership and update calls.
Keyboard::Keyboard()
{
	std::memset(m_keys, 0, sizeof(m_keys));
	std::memset(m_keyStatus, 0, sizeof(m_keyStatus));
	std::memset(m_keyNames, 0, sizeof(m_keyNames));
	m_modifiers = KEY_STATE_NONE;
	m_shift2Key = KEY_NONE;
	m_inputFrame = 0;
}
Keyboard::~Keyboard() = default;
void Keyboard::init() { m_inputFrame = 0; }
void Keyboard::reset() {}
void Keyboard::update() { ++m_inputFrame; }
void Keyboard::createStreamMessages() {}

GameEngine *CreateGameEngine()
{
	g_lifecycleReport = LifecycleReport{};
	g_scenarioSetupReport = ScenarioSetupReport{};
#if defined(ZH_M22_FULL_DRAW_TEST)
	g_logicSupplyBones = 0;
	g_clientBeforeLogicBones = 0;
#endif
	g_simulationReport = SimulationReport{};
	g_reentryReport = ReentryReport{};
	g_benchmarkTimer = -1;
	g_deviceAcquisitionAttempts = 0;
	g_propCount = 0;
	g_modelPreloadCount = 0;
	g_texturePreloadCount = 0;
	g_activePropCount = 0;
	g_activeModelPreloadCount = 0;
	g_activeTexturePreloadCount = 0;
	return new LinuxGameEngine;
}

extern "C" Int zh_linux_benchmark_timer()
{
	return g_benchmarkTimer;
}

extern "C" UnsignedInt zh_linux_device_acquisition_attempts()
{
	return g_deviceAcquisitionAttempts;
}

extern "C" Bool zh_linux_lifecycle_report(UnsignedInt *values, std::size_t count)
{
	if (!g_lifecycleReport.complete || !g_lifecycleReport.resetRan || count < 8 || values == NULL)
		return FALSE;
	values[0] = g_lifecycleReport.firstLogicBefore;
	values[1] = g_lifecycleReport.firstLogicAfter;
	values[2] = g_lifecycleReport.firstClientBefore;
	values[3] = g_lifecycleReport.firstClientAfter;
	values[4] = g_lifecycleReport.logicAfterReset;
	values[5] = g_lifecycleReport.clientAfterReset;
	values[6] = g_lifecycleReport.finalLogic;
	values[7] = g_lifecycleReport.finalClient;
	return TRUE;
}

extern "C" Bool zh_linux_scenario_setup_report(UnsignedInt *values, std::size_t count)
{
	if (!g_scenarioSetupReport.complete || !values || count < 8)
		return FALSE;
	values[0] = g_scenarioSetupReport.mode;
	values[1] = g_scenarioSetupReport.players;
	values[2] = g_scenarioSetupReport.teams;
	values[3] = g_scenarioSetupReport.objects;
	values[4] = g_scenarioSetupReport.props;
	values[5] = g_scenarioSetupReport.modelPreloads;
	values[6] = g_scenarioSetupReport.texturePreloads;
	values[7] = g_scenarioSetupReport.recorderControls;
	return TRUE;
}

extern "C" Bool zh_linux_simulation_report(Int *values, std::size_t count)
{
	if (!g_simulationReport.complete || !values || count < 14)
		return FALSE;
	values[0] = static_cast<Int>(g_simulationReport.frameBefore);
	values[1] = static_cast<Int>(g_simulationReport.frameAfter);
	values[2] = static_cast<Int>(g_simulationReport.aiUpdates);
	values[3] = static_cast<Int>(g_simulationReport.scriptUpdates);
	values[4] = g_simulationReport.startX;
	values[5] = g_simulationReport.movedX;
	values[6] = static_cast<Int>(g_simulationReport.actor);
	values[7] = static_cast<Int>(g_simulationReport.target);
	values[8] = g_simulationReport.moved;
	values[9] = g_simulationReport.attacked;
	values[10] = g_simulationReport.invalidRejected;
	values[11] = g_simulationReport.terminal;
	values[12] = g_simulationReport.targetHealthBefore;
	values[13] = g_simulationReport.targetHealthAfter;
	return TRUE;
}

extern "C" Bool zh_linux_reentry_report(Int *values, std::size_t count)
{
	if (!g_reentryReport.complete || !values || count < 11)
		return FALSE;
	values[0] = static_cast<Int>(g_reentryReport.resetObjects);
	values[1] = static_cast<Int>(g_reentryReport.resetMapObjects);
	values[2] = static_cast<Int>(g_reentryReport.resetProps);
	values[3] = static_cast<Int>(g_reentryReport.resetModelPreloads);
	values[4] = static_cast<Int>(g_reentryReport.resetTexturePreloads);
	values[5] = static_cast<Int>(g_reentryReport.resetRecorderControls);
	values[6] = g_reentryReport.firstStartX;
	values[7] = g_reentryReport.secondStartX;
	values[8] = static_cast<Int>(g_reentryReport.secondObjects);
	values[9] = static_cast<Int>(g_reentryReport.secondProps);
	values[10] = static_cast<Int>(g_reentryReport.secondRecorderControls);
	return TRUE;
}
