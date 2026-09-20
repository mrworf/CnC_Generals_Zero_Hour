#include "PreRTS.h"

#include "Common/ArchiveFileSystem.h"
#include "Common/ArchiveFile.h"
#include "Common/file.h"
#include "Common/GameAudio.h"
#include "Common/GameEngine.h"
#include "Common/FunctionLexicon.h"
#include "Common/GlobalData.h"
#include "Common/ModuleFactory.h"
#include "W3DDevice/Common/W3DModuleFactory.h"
#include "Common/Radar.h"
#include "Common/ThingFactory.h"
#include "Common/RAMFile.h"
#include "Common/StreamingArchiveFile.h"
#include "GameClient/Display.h"
#include "GameClient/DisplayString.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/GameClient.h"
#include "GameClient/GameFont.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/InGameUI.h"
#include "GameClient/Keyboard.h"
#include "GameClient/Mouse.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/Snow.h"
#include "GameClient/TerrainVisual.h"
#include "GameClient/VideoPlayer.h"
#include "GameClient/View.h"
#include "GameLogic/GameLogic.h"
#include "PosixDevice/Common/PosixLocalFileSystem.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

namespace {

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
Int g_benchmarkTimer = -1;
UnsignedInt g_deviceAcquisitionAttempts = 0;

UnsignedInt read_big_endian(File *file)
{
	UnsignedByte bytes[4]{};
	if (file->read(bytes, sizeof(bytes)) != sizeof(bytes))
		throw std::runtime_error("truncated BIG archive header");
	return (UnsignedInt(bytes[0]) << 24U) | (UnsignedInt(bytes[1]) << 16U) |
		(UnsignedInt(bytes[2]) << 8U) | UnsignedInt(bytes[3]);
}

UnsignedInt read_big_little_endian(File *file)
{
	UnsignedByte bytes[4]{};
	if (file->read(bytes, sizeof(bytes)) != sizeof(bytes))
		throw std::runtime_error("truncated BIG archive header");
	return UnsignedInt(bytes[0]) | (UnsignedInt(bytes[1]) << 8U) |
		(UnsignedInt(bytes[2]) << 16U) | (UnsignedInt(bytes[3]) << 24U);
}

class LinuxBIGFile final : public ArchiveFile
{
public:
	LinuxBIGFile() { m_file = NULL; }
	File *openFile(const Char *filename, Int access = 0) override
	{
		const ArchivedFileInfo *info = find(filename);
		if (!info) return NULL;
		RAMFile *file = BitTest(access, File::STREAMING) ?
			static_cast<RAMFile *>(newInstance(StreamingArchiveFile)) : newInstance(RAMFile);
		file->deleteOnClose();
		if (!file->openFromArchive(m_file, info->m_filename, info->m_offset, info->m_size))
		{
			file->close();
			return NULL;
		}
		if (!(access & File::WRITE)) return file;
		File *local = TheLocalFileSystem->openFile(filename, access);
		if (local) file->copyDataToFile(local);
		file->close();
		return local;
	}
	Bool getFileInfo(const AsciiString& filename, FileInfo *fileInfo) const override
	{
		const ArchivedFileInfo *info = find(filename.str());
		if (!info || !fileInfo) return FALSE;
		if (!TheLocalFileSystem->getFileInfo(AsciiString(m_file->getName()), fileInfo)) return FALSE;
		fileInfo->sizeHigh = 0;
		fileInfo->sizeLow = info->m_size;
		return TRUE;
	}
	void closeAllFiles() override {}
	AsciiString getName() override { return m_name; }
	AsciiString getPath() override { return m_path; }
	void setSearchPriority(Int) override {}
	void close() override {}
	void setIdentity(const Char *path)
	{
		m_path = path;
		m_name = std::filesystem::path(path ? path : "").filename().string().c_str();
	}
	void addEntry(const std::string& logical, const AsciiString& path, const ArchivedFileInfo& info)
	{
		std::string key = logical;
		std::replace(key.begin(), key.end(), '/', '\\');
		std::transform(key.begin(), key.end(), key.begin(),
			[](unsigned char value) { return static_cast<char>(std::tolower(value)); });
		if (!m_entries.emplace(key, info).second)
			throw std::runtime_error("duplicate BIG archive logical path");
		addFile(path, &info);
	}
private:
	const ArchivedFileInfo *find(const Char *filename) const
	{
		if (!filename) return NULL;
		std::string key(filename);
		std::replace(key.begin(), key.end(), '/', '\\');
		std::transform(key.begin(), key.end(), key.begin(),
			[](unsigned char value) { return static_cast<char>(std::tolower(value)); });
		auto found = m_entries.find(key);
		return found == m_entries.end() ? NULL : &found->second;
	}
	std::map<std::string, ArchivedFileInfo> m_entries;
	AsciiString m_name;
	AsciiString m_path;
};

class LinuxArchiveFileSystem final : public ArchiveFileSystem
{
public:
	void init() override { loadBigFilesFromDirectory(AsciiString(""), AsciiString("*.big"), FALSE); }
	void update() override {}
	void reset() override {}
	void postProcessLoad() override {}
	ArchiveFile *openArchiveFile(const Char *filename) override
	{
		File *input = TheLocalFileSystem->openFile(filename, File::READ | File::BINARY);
		if (!input) return NULL;
		try
		{
			const Int signedSize = input->size();
			if (signedSize < 16)
				throw std::runtime_error("truncated BIG archive header");
			const UnsignedInt archiveSize = UnsignedInt(signedSize);
			Char identifier[5]{};
			if (input->read(identifier, 4) != 4 ||
				(std::strcmp(identifier, "BIGF") != 0 && std::strcmp(identifier, "BIG4") != 0))
				throw std::runtime_error("unsupported BIG archive header");
			const UnsignedInt declaredSize = read_big_little_endian(input);
			const UnsignedInt count = read_big_endian(input);
			const UnsignedInt tableEnd = read_big_endian(input);
			if (declaredSize != archiveSize || count > 1000000U ||
				tableEnd < 16U || tableEnd > archiveSize)
				throw std::runtime_error("invalid BIG archive table bounds");

			auto archive = std::make_unique<LinuxBIGFile>();
			archive->setIdentity(filename);
			for (UnsignedInt index = 0; index < count; ++index)
			{
				if (input->position() < 0 || UnsignedInt(input->position()) > tableEnd ||
					tableEnd - UnsignedInt(input->position()) < 9U)
					throw std::runtime_error("truncated BIG archive entry table");
				ArchivedFileInfo info;
				info.m_archiveFilename = filename;
				info.m_offset = read_big_endian(input);
				info.m_size = read_big_endian(input);
				if (info.m_offset > archiveSize || info.m_size > archiveSize - info.m_offset ||
					(info.m_size != 0 && info.m_offset < tableEnd))
					throw std::runtime_error("invalid BIG archive entry bounds");
				std::string logical;
				for (std::size_t length = 0; length != 1024; ++length)
				{
					if (input->position() < 0 || UnsignedInt(input->position()) >= tableEnd)
						throw std::runtime_error("truncated BIG archive entry name");
					Char character = 0;
					if (input->read(&character, 1) != 1)
						throw std::runtime_error("truncated BIG archive entry name");
					if (!character) break;
					logical.push_back(character);
				}
				if (logical.empty() || logical.size() == 1024)
					throw std::runtime_error("invalid BIG archive entry name");
				std::replace(logical.begin(), logical.end(), '/', '\\');
				const std::size_t split = logical.find_last_of('\\');
				std::string path = split == std::string::npos ? "" : logical.substr(0, split + 1);
				std::string name = split == std::string::npos ? logical : logical.substr(split + 1);
				std::transform(name.begin(), name.end(), name.begin(),
					[](unsigned char value) { return static_cast<char>(std::tolower(value)); });
				info.m_filename = name.c_str();
				archive->addEntry(logical, AsciiString(path.c_str()), info);
			}
			archive->attachFile(input);
			return archive.release();
		}
		catch (...)
		{
			input->close();
			throw;
		}
	}
	void closeArchiveFile(const Char *filename) override
	{
		auto found = m_archiveFileMap.find(AsciiString(filename));
		if (found == m_archiveFileMap.end()) return;
		delete found->second;
		m_archiveFileMap.erase(found);
	}
	void closeAllArchiveFiles() override
	{
		for (auto& entry : m_archiveFileMap) delete entry.second;
		m_archiveFileMap.clear();
		m_rootDirectory.clear();
	}
	void closeAllFiles() override
	{
		for (auto& entry : m_archiveFileMap) entry.second->closeAllFiles();
	}
	Bool loadBigFilesFromDirectory(AsciiString directory, AsciiString mask, Bool overwrite) override
	{
		FilenameList files;
		TheLocalFileSystem->getFileListInDirectory(directory, AsciiString(""), mask, files, TRUE);
		Bool loaded = FALSE;
		for (const AsciiString& filename : files)
		{
			ArchiveFile *archive = openArchiveFile(filename.str());
			if (!archive) continue;
			loadIntoDirectoryTree(archive, filename, overwrite);
			m_archiveFileMap[filename] = archive;
			loaded = TRUE;
		}
		return loaded;
	}
};

class LinuxDisplay final : public Display
{
public:
	LinuxDisplay() { setWidth(800); setHeight(600); }
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
	void addProp(const ThingTemplate *, const Coord3D *, Real) override {}
	void setRawMapHeight(const ICoord2D *, Int) override {}
	Int getRawMapHeight(const ICoord2D *) override { return 0; }
	void replaceSkyboxTextures(const AsciiString *[NumSkyboxTextures], const AsciiString *[NumSkyboxTextures]) override {}
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
	void createRayEffectByTemplate(const Coord3D *, const Coord3D *, const ThingTemplate *) override {}
	void addScorch(const Coord3D *, Real, Scorches) override {}
	Drawable *friend_createDrawable(const ThingTemplate *, DrawableStatus) override { return NULL; }
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

class LinuxGameEngine final : public GameEngine
{
public:
	void update() override
	{
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
	LocalFileSystem *createLocalFileSystem() override
	{
		const char *root = std::getenv("ZH_DATA_ROOT");
		if (!root || !*root) throw std::runtime_error("ZH_DATA_ROOT is required");
		return new PosixLocalFileSystem(std::filesystem::path(root));
	}
	ArchiveFileSystem *createArchiveFileSystem() override { return new LinuxArchiveFileSystem; }
	GameLogic *createGameLogic() override { return new GameLogic; }
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
	unsigned m_updates = 0;
	unsigned m_services = 0;
};

} // namespace

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
	g_benchmarkTimer = -1;
	g_deviceAcquisitionAttempts = 0;
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
