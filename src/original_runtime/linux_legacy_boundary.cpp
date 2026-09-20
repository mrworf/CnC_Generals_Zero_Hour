#include "PreRTS.h"

#include "Common/MapObject.h"
#include "Common/CDManager.h"
#include "Common/Debug.h"
#include "Common/GameLOD.h"
#include "Common/FunctionLexicon.h"
#include "Common/OSDisplay.h"
#include "Common/ThingTemplate.h"
#include "Common/WellKnownKeys.h"
#include "GameClient/IMEManager.h"
#include "GameClient/Keyboard.h"
#include "GameClient/DisconnectMenu.h"
#include "GameNetwork/GameInfo.h"
#include "GameNetwork/LANAPICallbacks.h"
#include "GameNetwork/GameSpy/StagingRoomGameInfo.h"
#include "GameNetwork/GameSpy/GameResultsThread.h"

#include <stdexcept>
#include <cstdio>

// These globals remain null in the native offline profile.  Their Win32 owners
// are UI/network entry points that M20 deliberately rejects rather than
// silently constructing a transport or platform window.
IMEManagerInterface *TheIMEManager = NULL;
Keyboard *TheKeyboard = NULL;
GameSpyStagingRoom *TheGameSpyGame = NULL;
Int NET_CRC_INTERVAL = 100;
// Focused fixture executables may provide their own retail-free localization
// path constants.  Production uses these defaults.
const Char *g_strFile __attribute__((weak)) = "data/Generals.str";
const Char *g_csfFile __attribute__((weak)) = "data/%s/Generals.csf";

namespace {
class PosixCDManager final : public CDManager
{
protected:
	CDDriveInterface *createDrive() override { return new CDDrive; }
};

class OfflineGameResults final : public GameResultsInterface
{
public:
	void init() override {}
	void reset() override {}
	void update() override {}
	void startThreads() override {}
	void endThreads() override {}
	Bool areThreadsRunning() override { return FALSE; }
	void addRequest(const GameResultsRequest&) override
	{
		throw std::runtime_error("online game-results transport is unavailable");
	}
	Bool getRequest(GameResultsRequest&) override { return FALSE; }
	void addResponse(const GameResultsResponse&) override
	{
		throw std::runtime_error("online game-results transport is unavailable");
	}
	Bool getResponse(GameResultsResponse&) override { return FALSE; }
	Bool areGameResultsBeingSent() override { return FALSE; }
};
}

GameResultsInterface *TheGameResultsQueue = NULL;

GameResultsInterface *GameResultsInterface::createNewGameResultsInterface()
{
	return new OfflineGameResults;
}

CDManagerInterface *CreateCDManager()
{
	return new PosixCDManager;
}

OSDisplayButtonType OSDisplayWarningBox(AsciiString prompt, AsciiString message,
	UnsignedInt, UnsignedInt)
{
	std::fprintf(stderr, "%s: %s\n", prompt.str(), message.str());
	return OSDBT_OK;
}

Bool testMinimumRequirements(ChipsetType *videoChipType, CpuType *cpuType, Int *cpuFreq,
	Int *numRAM, Real *intBenchIndex, Real *floatBenchIndex, Real *memBenchIndex)
{
	if (videoChipType) *videoChipType = DC_UNKNOWN;
	if (cpuType) *cpuType = XX;
	if (cpuFreq) *cpuFreq = 0;
	if (numRAM) *numRAM = 0;
	if (intBenchIndex) *intBenchIndex = 0.0f;
	if (floatBenchIndex) *floatBenchIndex = 0.0f;
	if (memBenchIndex) *memBenchIndex = 0.0f;
	return FALSE;
}

extern "C" void ReleaseCrash(const char *reason)
{
	throw std::runtime_error(reason ? reason : "original runtime failure");
}

extern "C" void ReleaseCrashLocalized(const AsciiString& prompt, const AsciiString& message)
{
	AsciiString detail(prompt);
	detail.concat(": ");
	detail.concat(message);
	throw std::runtime_error(detail.str());
}

IMEManagerInterface *CreateIMEManagerInterface()
{
	return NULL;
}

Bool Keyboard::isShift() { return FALSE; }
Bool Keyboard::isCtrl() { return FALSE; }
Bool Keyboard::isAlt() { return FALSE; }
WideChar Keyboard::getPrintableKey(UnsignedByte, Int) { return 0; }

MapObject::MapObject(Coord3D loc, AsciiString name, Real angle, Int flags, const Dict *props,
	const ThingTemplate *thingTemplate)
{
	m_objectName = name;
	m_thingTemplate = thingTemplate;
	m_nextMapObject = NULL;
	m_location = loc;
	m_angle = normalizeAngle(angle);
	m_color = 0xff << 8;
	m_flags = flags;
	m_renderObj = NULL;
	m_shadowObj = NULL;
	m_runtimeFlags = 0;
	if (props) {
		m_properties = *props;
	} else {
		m_properties.setInt(TheKey_objectInitialHealth, 100);
		m_properties.setBool(TheKey_objectEnabled, true);
		m_properties.setBool(TheKey_objectIndestructible, false);
		m_properties.setBool(TheKey_objectUnsellable, false);
		m_properties.setBool(TheKey_objectPowered, true);
		m_properties.setBool(TheKey_objectRecruitableAI, true);
		m_properties.setBool(TheKey_objectTargetable, false);
	}
	for (Int i = 0; i < BRIDGE_MAX_TOWERS; ++i)
		setBridgeRenderObject(static_cast<BridgeTowerType>(i), NULL);
}

MapObject::~MapObject()
{
	m_renderObj = NULL;
	m_shadowObj = NULL;
	while (m_nextMapObject)
	{
		MapObject *next = m_nextMapObject->getNext();
		m_nextMapObject->setNextMap(NULL);
		m_nextMapObject->deleteInstance();
		m_nextMapObject = next;
	}
}

void MapObject::setRenderObj(RenderObjClass *renderObject)
{
	if (renderObject)
		throw std::runtime_error("map render objects require a renderer provider");
	m_renderObj = NULL;
}

void MapObject::setBridgeRenderObject(BridgeTowerType type, RenderObjClass *renderObject)
{
	if (renderObject)
		throw std::runtime_error("bridge render objects require a renderer provider");
	if (type >= 0 && type < BRIDGE_MAX_TOWERS)
		m_bridgeTowers[type] = NULL;
}

RenderObjClass *MapObject::getBridgeRenderObject(BridgeTowerType type)
{
	return type >= 0 && type < BRIDGE_MAX_TOWERS ? m_bridgeTowers[type] : NULL;
}

MapObject *MapObject::duplicate()
{
	MapObject *copy = newInstance(MapObject)(m_location, m_objectName, m_angle, m_flags,
		&m_properties, m_thingTemplate);
	copy->m_color = m_color;
	copy->m_runtimeFlags = m_runtimeFlags;
	return copy;
}

void MapObject::setThingTemplate(const ThingTemplate *thing)
{
	m_thingTemplate = thing;
	m_objectName = thing ? thing->getName() : AsciiString::TheEmptyString;
}

void MapObject::setName(AsciiString name)
{
	m_objectName = name;
}

const ThingTemplate *MapObject::getThingTemplate() const
{
	return m_thingTemplate ? static_cast<const ThingTemplate *>(m_thingTemplate->getFinalOverride()) : NULL;
}

WaypointID MapObject::getWaypointID()
{
	return static_cast<WaypointID>(getProperties()->getInt(TheKey_waypointID));
}

AsciiString MapObject::getWaypointName()
{
	return getProperties()->getAsciiString(TheKey_waypointName);
}

void MapObject::setWaypointID(Int id)
{
	getProperties()->setInt(TheKey_waypointID, id);
}

void MapObject::setWaypointName(AsciiString name)
{
	getProperties()->setAsciiString(TheKey_waypointName, name);
}

[[noreturn]] static void unsupported_device_entry(const char *entry)
{
	throw std::runtime_error(entry);
}

void ReloadAllTextures()
{
	unsupported_device_entry("texture reload requires a renderer provider");
}

void oversizeTheTerrain(Int)
{
	unsupported_device_entry("terrain oversize requires a renderer provider");
}

void doSkyBoxSet(Bool)
{
	unsupported_device_entry("skybox control requires a renderer provider");
}
