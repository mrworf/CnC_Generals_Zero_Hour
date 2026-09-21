#include "PreRTS.h"

#include "Common/MapReaderWriterInfo.h"
#include "Common/MapObject.h"
#include "Common/NameKeyGenerator.h"
#include "Common/OriginalMapLoader.h"
#include "Common/ThingFactory.h"
#include "GameLogic/SidesList.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string_view>
#include <vector>

class GameEngine;
GameEngine *TheGameEngine = NULL;

namespace {

class MemoryChunkInput final : public ChunkInputStream
{
public:
	explicit MemoryChunkInput(const std::vector<std::uint8_t>& bytes) : m_bytes(bytes) {}
	Int read(void *data, Int count) override
	{
		if (count < 0) return 0;
		const std::size_t available = m_bytes.size() - m_position;
		const std::size_t size = std::min<std::size_t>(available, static_cast<std::size_t>(count));
		std::memcpy(data, m_bytes.data() + m_position, size);
		m_position += size;
		return static_cast<Int>(size);
	}
	UnsignedInt tell() override { return static_cast<UnsignedInt>(m_position); }
	Bool absoluteSeek(UnsignedInt position) override
	{
		if (position > m_bytes.size()) return FALSE;
		m_position = position;
		return TRUE;
	}
	Bool eof() override { return m_position == m_bytes.size(); }
private:
	const std::vector<std::uint8_t>& m_bytes;
	std::size_t m_position = 0;
};

void appendInt(std::vector<std::uint8_t>& bytes, std::uint32_t value)
{
	for (unsigned shift = 0; shift != 32; shift += 8)
		bytes.push_back(static_cast<std::uint8_t>(value >> shift));
}

void appendShort(std::vector<std::uint8_t>& bytes, std::uint16_t value)
{
	bytes.push_back(static_cast<std::uint8_t>(value));
	bytes.push_back(static_cast<std::uint8_t>(value >> 8));
}

void appendReal(std::vector<std::uint8_t>& bytes, Real value)
{
	std::uint32_t representation = 0;
	static_assert(sizeof(value) == sizeof(representation));
	std::memcpy(&representation, &value, sizeof(value));
	appendInt(bytes, representation);
}

void appendAscii(std::vector<std::uint8_t>& bytes, std::string_view value)
{
	appendShort(bytes, static_cast<std::uint16_t>(value.size()));
	bytes.insert(bytes.end(), value.begin(), value.end());
}

void appendMapping(std::vector<std::uint8_t>& bytes, std::string_view name, std::uint32_t id)
{
	bytes.push_back(static_cast<std::uint8_t>(name.size()));
	bytes.insert(bytes.end(), name.begin(), name.end());
	appendInt(bytes, id);
}

void appendChunk(std::vector<std::uint8_t>& bytes, std::uint32_t id, std::uint16_t version,
	const std::vector<std::uint8_t>& payload)
{
	appendInt(bytes, id);
	appendShort(bytes, version);
	appendInt(bytes, static_cast<std::uint32_t>(payload.size()));
	bytes.insert(bytes.end(), payload.begin(), payload.end());
}

std::vector<std::uint8_t> fixture(Int declaredSize = 6,
	std::string_view objectName = "OwnedMissionObject")
{
	std::vector<std::uint8_t> height;
	appendInt(height, 3); // width
	appendInt(height, 2); // height
	appendInt(height, 1); // border
	appendInt(height, 1); // boundary count
	appendInt(height, 1);
	appendInt(height, 1);
	appendInt(height, static_cast<std::uint32_t>(declaredSize));
	height.insert(height.end(), {0, 16, 32, 48, 64, 80});
	std::vector<std::uint8_t> object;
	appendReal(object, 10.0f);
	appendReal(object, 20.0f);
	appendReal(object, 30.0f);
	appendReal(object, 0.5f);
	appendInt(object, 0);
	appendAscii(object, objectName);
	appendShort(object, 0); // empty Dict
	std::vector<std::uint8_t> objects;
	appendChunk(objects, 5, K_OBJECTS_VERSION_3, object);
	std::vector<std::uint8_t> sides;
	appendInt(sides, 1); // authored neutral side
	appendShort(sides, 0); // empty side Dict represents neutral
	appendInt(sides, 0); // no build-list entries
	appendInt(sides, 0); // no authored teams
	std::vector<std::uint8_t> playerScripts;
	appendChunk(playerScripts, 8, 1, {}); // one empty original ScriptList
	appendChunk(sides, 7, 1, playerScripts);

	std::vector<std::uint8_t> bytes{'C', 'k', 'M', 'p'};
	appendInt(bytes, 8);
	appendMapping(bytes, "HeightMapData", 1);
	appendMapping(bytes, "UnknownFutureChunk", 2);
	appendMapping(bytes, "WorldInfo", 3);
	appendMapping(bytes, "ObjectsList", 4);
	appendMapping(bytes, "Object", 5);
	appendMapping(bytes, "SidesList", 6);
	appendMapping(bytes, "PlayerScriptsList", 7);
	appendMapping(bytes, "ScriptList", 8);
	appendChunk(bytes, 1, K_HEIGHT_MAP_VERSION_4, height);
	appendChunk(bytes, 3, K_WORLDDICT_VERSION_1, {0, 0}); // empty Dict
	appendChunk(bytes, 4, K_OBJECTS_VERSION_3, objects);
	appendChunk(bytes, 6, 2, sides); // K_SIDES_DATA_VERSION_2
	appendChunk(bytes, 2, 1, {1, 2, 3});
	return bytes;
}

int failures = 0;
void check(bool condition, std::string_view message)
{
	if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

} // namespace

int main()
{
	initMemoryManager();
	NameKeyGenerator nameKeys;
	TheNameKeyGenerator = &nameKeys;
	nameKeys.init();
	SidesList sides;
	TheSidesList = &sides;
	ThingFactory things;
	TheThingFactory = &things;

	auto validBytes = fixture();
	MemoryChunkInput validStream(validBytes);
	OriginalMapLoader map;
	check(map.load(&validStream), "owned chunky map parses through original CPU loader");
	check(map.width() == 3 && map.height() == 2 && map.borderSize() == 1,
		"height-map dimensions and border are source-owned state");
	check(map.boundaries().size() == 1 && map.boundaries()[0].x == 1 && map.boundaries()[0].y == 1,
		"playable boundary parses");
	check(map.heights() == std::vector<UnsignedByte>({0, 16, 32, 48, 64, 80}),
		"all terrain samples parse without renderer state");
	check(MapObject::getFirstMapObject() != NULL &&
		MapObject::getFirstMapObject()->getName() == AsciiString("OwnedMissionObject"),
		"object list publishes an original MapObject");
	check(MapObject::getFirstMapObject() && MapObject::getFirstMapObject()->getLocation()->x == 10.0f,
		"object coordinates remain source-owned state");
	check(sides.getNumSides() == 1 && sides.getNumTeams() == 1,
		"original side validation publishes neutral player and team state");
	check(sides.getSideInfo(0)->getScriptList() != NULL,
		"side retains its original script-list state");

	auto skirmishBytes = fixture(6, "OwnedSkirmishObject");
	MemoryChunkInput skirmishStream(skirmishBytes);
	check(map.load(&skirmishStream) && MapObject::getFirstMapObject() &&
		MapObject::getFirstMapObject()->getName() == AsciiString("OwnedSkirmishObject") &&
		MapObject::getFirstMapObject()->getNext() == NULL,
		"skirmish fixture replaces mission parser state instead of appending it");

	auto malformedBytes = fixture(5);
	MemoryChunkInput malformedStream(malformedBytes);
	bool malformedRejected = false;
	try { OriginalMapLoader malformed; malformedRejected = !malformed.load(&malformedStream); }
	catch (...) { malformedRejected = true; }
	check(malformedRejected, "inconsistent height payload is rejected");
	check(MapObject::getFirstMapObject() == NULL && sides.getNumSides() == 0,
		"failed parse rolls back published object and side state");

	const std::vector<std::uint8_t> truncatedBytes{'C', 'k', 'M'};
	MemoryChunkInput truncatedStream(truncatedBytes);
	bool truncatedRejected = false;
	try { OriginalMapLoader truncated; truncatedRejected = !truncated.load(&truncatedStream); }
	catch (...) { truncatedRejected = true; }
	check(truncatedRejected, "truncated chunk stream is rejected");
	OriginalMapLoader missing;
	check(!missing.load(NULL) && MapObject::getFirstMapObject() == NULL && sides.getNumSides() == 0,
		"missing map fails with empty published state");

	TheSidesList = NULL;
	if (MapObject::TheMapObjectListPtr) MapObject::TheMapObjectListPtr->deleteInstance();
	MapObject::TheMapObjectListPtr = NULL;
	TheThingFactory = NULL;
	TheNameKeyGenerator = NULL;
	shutdownMemoryManager();
	if (failures == 0)
		std::cout << "M21 original map: dimensions=3x2 boundary=1 heights=6 malformed=closed\n";
	return failures == 0 ? 0 : 1;
}
