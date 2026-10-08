/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "PreRTS.h"
#include "Common/MapObject.h"
#include "Common/ThingTemplate.h"
#include "Common/WellKnownKeys.h"
#include <stack>
#include <cerrno>
#include <limits>
#include <vector>

namespace {
NameKeyGenerator& mapNamespace() {
    if(!TheNameKeyGenerator) throw ERROR_BAD_ARG;
    return *TheNameKeyGenerator;
}
class MapPropertiesTransaction {
    NameKeyTransaction m_names;
    MapObject& m_owner;
    Dict m_before;
    bool m_keep=false;
public:
    explicit MapPropertiesTransaction(MapObject& owner)
        :m_names(mapNamespace()),m_owner(owner),m_before(*owner.getProperties()) {}
    ~MapPropertiesTransaction() noexcept { if(!m_keep) m_owner.getProperties()->swap(m_before); }
    void commit() noexcept { m_names.commit(); m_keep=true; }
};
}

// -----------------------------------------------------------
static AsciiString validateName(AsciiString n, Int flags)
{

	return n;

}

/* ********* MapObject class ****************************/
/*static*/ MapObject *MapObject::TheMapObjectListPtr = NULL;
/*static*/ Dict MapObject::TheWorldDict;

MapObject::MapObject(Coord3D loc, AsciiString name, Real angle, Int flags, const Dict* props,
										 const ThingTemplate *thingTemplate )
{
	m_objectName = validateName( name, flags );
	m_thingTemplate = thingTemplate;
	m_nextMapObject = NULL;
	m_location = loc;
	m_angle = normalizeAngle(angle);
	m_color = (0xff)<<8; // Bright green.
	m_flags = flags;
	m_shadowObj = NULL;
	m_runtimeFlags = 0;
	// Note - do NOT set TheKey_objectSelectable on creation - allow it to follow the .ini value unless specified by user action.  jba. [3/20/2003]
	if (props)
	{
		m_properties = *props;
	}
	else
	{
        NameKeyTransaction names(mapNamespace());
        Dict defaults;
        defaults.setInt(TheKey_objectInitialHealth,100);
        defaults.setBool(TheKey_objectEnabled,true);
        defaults.setBool(TheKey_objectIndestructible,false);
        defaults.setBool(TheKey_objectUnsellable,false);
        defaults.setBool(TheKey_objectPowered,true);
        defaults.setBool(TheKey_objectRecruitableAI,true);
        defaults.setBool(TheKey_objectTargetable,false);
        m_properties.swap(defaults);
        names.commit();
	}

	for( Int i = 0; i < BRIDGE_MAX_TOWERS; ++i )
		setBridgeRenderObject( (BridgeTowerType)i, NULL );

}


MapObject::~MapObject(void)
{
	setRenderObj(NULL);
	setShadowObj(NULL);
	if (m_nextMapObject) {
		MapObject *cur = m_nextMapObject;
		MapObject *next;
		while (cur) {
			next = cur->getNext();
			cur->setNextMap(NULL); // prevents recursion.
			cur->deleteInstance();
			cur = next;
		}
	}
	for( Int i = 0; i < BRIDGE_MAX_TOWERS; ++i )
		setBridgeRenderObject( (BridgeTowerType)i, NULL );

}

MapObject *MapObject::duplicate(void)
{
	MapObject *pObj = newInstance( MapObject)(m_location, m_objectName, m_angle, m_flags, &m_properties, m_thingTemplate);
	pObj->setColor(getColor());
	pObj->m_runtimeFlags = m_runtimeFlags;
	return pObj;
}

void MapObject::setRenderObj(RenderObjClass *pObj,NativeMapRenderOwnership owner)
{
	m_renderObj.reset(pObj,owner);
}

void MapObject::setBridgeRenderObject(BridgeTowerType type,RenderObjClass* renderObj,NativeMapRenderOwnership owner)
{

	if( type >= 0 && type < BRIDGE_MAX_TOWERS )
		m_bridgeTowers[type].reset(renderObj,owner);

}

RenderObjClass* MapObject::getBridgeRenderObject( BridgeTowerType type )
{

	if( type >= 0 && type < BRIDGE_MAX_TOWERS )
		return m_bridgeTowers[type].get();
	return NULL;

}

void MapObject::validate(void)
{
	MapPropertiesTransaction transaction(*this);
	verifyValidTeam();
	verifyValidUniqueID();
	transaction.commit();
}

void MapObject::verifyValidUniqueID(void)
{
	MapPropertiesTransaction transaction(*this);
	Bool exists;
	AsciiString uniqueID = getProperties()->getAsciiString(TheKey_uniqueID, &exists);
	MapObject *obj = MapObject::getFirstMapObject();

	// -1 is the sentinel
	int highestIndex = -1;

	while (obj) {
		if (obj == this) {
			// the first object is THIS OBJECT, cause we've already been added.
			obj = obj->getNext();
			continue;
		}

		if (obj->isWaypoint()) {
			// waypoints throw this off. Sad but true. :-(
			obj = obj->getNext();
			continue;
		}

		Bool iterateExists;
		AsciiString tempStr = obj->getProperties()->getAsciiString(TheKey_uniqueID, &iterateExists);
		const char* lastSpace = tempStr.reverseFind(' ');

		int testIndex = -1;
		if (lastSpace) {
            errno=0;
            const auto raw=std::strtol(lastSpace,nullptr,10);
            if(errno==ERANGE || raw<std::numeric_limits<Int>::min() || raw>std::numeric_limits<Int>::max())
                throw ERROR_BAD_ARG;
            testIndex=Int(raw);
		}

		if (testIndex > highestIndex) {
			highestIndex = testIndex;
		}
		break;
	}

	if(highestIndex==std::numeric_limits<Int>::max()) throw ERROR_BAD_ARG;
	int indexOfThisObject = highestIndex + 1;

	const char* thingName;
	if (getThingTemplate()) {
		thingName = getThingTemplate()->getName().str();
	} else if (isWaypoint()) {
		thingName = getWaypointName().str();
	} else {
		thingName = getName().str();
	}
	const char* pName = thingName;

	while (*thingName) {
		if ((*thingName) == '/') {
			pName = thingName + 1;
		}
		++thingName;
	}

	AsciiString newID;
	if (isWaypoint()) {
		newID.format("%s", pName);
	} else {
		newID.format("%s %d", pName, indexOfThisObject);
	}
	getProperties()->setAsciiString(TheKey_uniqueID, newID);
	transaction.commit();
}

void MapObject::fastAssignAllUniqueIDs(void)
{
    NameKeyTransaction names(mapNamespace());
    std::vector<std::pair<MapObject*,Dict>> candidates;
    for(auto* object=getFirstMapObject();object;object=object->getNext()) {
        if(candidates.size()==std::size_t(std::numeric_limits<Int>::max())) throw ERROR_OUT_OF_MEMORY;
        candidates.emplace_back(object,*object->getProperties());
    }
    Int index=0;
    // Preserve the source stack's reverse list order, including waypoint ordinals.
    for(auto item=candidates.rbegin();item!=candidates.rend();++item,++index) {
        auto* object=item->first;
        AsciiString name=object->getThingTemplate() ? object->getThingTemplate()->getName()
            : object->isWaypoint() ? object->getWaypointName() : object->getName();
        const char* leaf=name.str();
        for(const char* at=leaf;*at;++at) if(*at=='/') leaf=at+1;
        AsciiString id;
        if(object->isWaypoint()) id.format("%s",leaf); else id.format("%s %d",leaf,index);
        item->second.setAsciiString(TheKey_uniqueID,id);
    }
    // Complete admission precedes nonallocating publication of every dictionary.
    for(auto& item:candidates) item.first->getProperties()->swap(item.second);
    names.commit();
}

void MapObject::setThingTemplate(const ThingTemplate *thing)
{
	if(!thing) throw ERROR_BAD_ARG;
	AsciiString candidate=thing->getName();
	m_objectName.swap(candidate);
	m_thingTemplate = thing;
}


void MapObject::setName(AsciiString name)
{
	m_objectName = name;
}

WaypointID MapObject::getWaypointID() { return (WaypointID)getProperties()->getInt(TheKey_waypointID); }
AsciiString MapObject::getWaypointName() { return getProperties()->getAsciiString(TheKey_waypointName); }
void MapObject::setWaypointID(Int i) {
    MapPropertiesTransaction transaction(*this);
    getProperties()->setInt(TheKey_waypointID,i); transaction.commit();
}
void MapObject::setWaypointName(AsciiString n) {
    MapPropertiesTransaction transaction(*this);
    getProperties()->setAsciiString(TheKey_waypointName,n); transaction.commit();
}

/*static */ Int MapObject::countMapObjectsWithOwner(const AsciiString& n)
{
	Int count = 0;
	for (MapObject *pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext())
	{
		if (pMapObj->getProperties()->getAsciiString(TheKey_originalOwner) == n)
			++count;
	}
	return count;
}

//-------------------------------------------------------------------------------------------------
const ThingTemplate *MapObject::getThingTemplate( void ) const
{
	if (m_thingTemplate)
		return (const ThingTemplate*) m_thingTemplate->getFinalOverride();

	return NULL;
}
