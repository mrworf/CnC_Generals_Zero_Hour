#include "PreRTS.h"

#include "Common/OriginalMapLoader.h"

#include "Common/DataChunk.h"
#include "Common/GlobalData.h"
#include "Common/MapObject.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/ThingFactory.h"
#include "Common/WellKnownKeys.h"
#include "GameLogic/PolygonTrigger.h"
#include "GameLogic/SidesList.h"

#include <limits>

namespace {

void freeMapObjects()
{
	if (MapObject::TheMapObjectListPtr)
		MapObject::TheMapObjectListPtr->deleteInstance();
	MapObject::TheMapObjectListPtr = NULL;
	MapObject::getWorldDict()->clear();
}

} // namespace

void OriginalMapLoader::clear()
{
	m_width = 0;
	m_height = 0;
	m_borderSize = 0;
	m_boundaries.clear();
	m_heights.clear();
}

Bool OriginalMapLoader::load(ChunkInputStream *stream)
{
	clear();
	freeMapObjects();
	PolygonTrigger::deleteTriggers();
	TheSidesList->emptySides();
	if (!stream)
		return FALSE;

	try
	{
		DataChunkInput file(stream);
		file.registerParser(AsciiString("HeightMapData"), AsciiString::TheEmptyString, parseHeightMap);
		file.registerParser(AsciiString("WorldInfo"), AsciiString::TheEmptyString, parseWorldInfo);
		file.registerParser(AsciiString("ObjectsList"), AsciiString::TheEmptyString, parseObjects);
		file.registerParser(AsciiString("PolygonTriggers"), AsciiString::TheEmptyString,
			PolygonTrigger::ParsePolygonTriggersDataChunk);
		file.registerParser(AsciiString("SidesList"), AsciiString::TheEmptyString,
			SidesList::ParseSidesDataChunk);
		if (!file.parse(this) || m_width <= 0 || m_height <= 0 || m_heights.empty())
			throw ERROR_CORRUPT_FILE_FORMAT;
		TheSidesList->validateSides();
		return TRUE;
	}
	catch (...)
	{
		clear();
		freeMapObjects();
		PolygonTrigger::deleteTriggers();
		TheSidesList->emptySides();
		throw;
	}
}

Bool OriginalMapLoader::parseHeightMap(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	OriginalMapLoader *self = static_cast<OriginalMapLoader *>(userData);
	self->m_width = file.readInt();
	self->m_height = file.readInt();
	self->m_borderSize = info->version >= K_HEIGHT_MAP_VERSION_3 ? file.readInt() : 0;
	if (self->m_width <= 0 || self->m_height <= 0 ||
		self->m_width > std::numeric_limits<Int>::max() / self->m_height)
		throw ERROR_CORRUPT_FILE_FORMAT;

	if (info->version >= K_HEIGHT_MAP_VERSION_4)
	{
		const Int count = file.readInt();
		if (count < 0 || count > 1024)
			throw ERROR_CORRUPT_FILE_FORMAT;
		self->m_boundaries.resize(static_cast<std::size_t>(count));
		for (Int i = 0; i < count; ++i)
		{
			self->m_boundaries[i].x = file.readInt();
			self->m_boundaries[i].y = file.readInt();
		}
	}
	else
	{
		self->m_boundaries.resize(1);
		self->m_boundaries[0].x = self->m_width - 2 * self->m_borderSize;
		self->m_boundaries[0].y = self->m_height - 2 * self->m_borderSize;
	}

	const Int dataSize = file.readInt();
	if (dataSize <= 0 || dataSize != self->m_width * self->m_height)
		throw ERROR_CORRUPT_FILE_FORMAT;
	self->m_heights.resize(static_cast<std::size_t>(dataSize));
	file.readArrayOfBytes(reinterpret_cast<char *>(self->m_heights.data()), dataSize);

	if (info->version == K_HEIGHT_MAP_VERSION_1)
	{
		const Int newWidth = (self->m_width + 1) / 2;
		const Int newHeight = (self->m_height + 1) / 2;
		for (Int y = 0; y < newHeight; ++y)
			for (Int x = 0; x < newWidth; ++x)
				self->m_heights[y * newWidth + x] = self->m_heights[2 * y * self->m_width + 2 * x];
		self->m_width = newWidth;
		self->m_height = newHeight;
		self->m_heights.resize(static_cast<std::size_t>(newWidth * newHeight));
	}
	return TRUE;
}

Bool OriginalMapLoader::parseWorldInfo(DataChunkInput &file, DataChunkInfo *, void *)
{
	*MapObject::getWorldDict() = file.readDict();
	Bool exists = FALSE;
	const Int weather = MapObject::getWorldDict()->getInt(TheKey_weather, &exists);
	if (exists && TheWritableGlobalData)
		TheWritableGlobalData->m_weather = static_cast<Weather>(weather);
	return TRUE;
}

Bool OriginalMapLoader::parseObjects(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	file.m_currentObject = NULL;
	file.registerParser(AsciiString("Object"), info->label, parseObject);
	return file.parse(userData);
}

Bool OriginalMapLoader::parseObject(DataChunkInput &file, DataChunkInfo *info, void *)
{
	MapObject *previous = static_cast<MapObject *>(file.m_currentObject);
	Coord3D location;
	location.x = file.readReal();
	location.y = file.readReal();
	location.z = file.readReal();
	if (info->version <= K_OBJECTS_VERSION_2)
		location.z = 0;
	const Real angle = file.readReal();
	const Int flags = file.readInt();
	const AsciiString name = file.readAsciiString();
	Dict properties;
	if (info->version >= K_OBJECTS_VERSION_2)
		properties = file.readDict();
	if (location.z < -100 * MAP_XY_FACTOR || location.z > (255 * 10) * MAP_HEIGHT_SCALE)
		return TRUE;
	if (!TheThingFactory)
		throw ERROR_CORRUPT_FILE_FORMAT;

	MapObject *object = newInstance(MapObject)(location, name, angle, flags, &properties,
		TheThingFactory->findTemplate(name, FALSE));
	if (object->getProperties()->getType(TheKey_waypointID) == Dict::DICT_INT)
		object->setIsWaypoint();
	if (object->getProperties()->getType(TheKey_lightHeightAboveTerrain) == Dict::DICT_REAL)
		object->setIsLight();
	if (object->getProperties()->getType(TheKey_scorchType) == Dict::DICT_INT)
		object->setIsScorch();
	if (previous)
		previous->setNextMap(object);
	else
		MapObject::TheMapObjectListPtr = object;
	file.m_currentObject = object;
	return TRUE;
}
