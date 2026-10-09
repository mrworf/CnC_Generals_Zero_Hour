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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// PolygonTrigger.cpp
// Class to encapsulate polygon trigger areas.
// Author: John Ahlquist, November 2001

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/GlobalData.h"

#include "Common/DataChunk.h"
#include "Common/MapObject.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/Xfer.h"
#include "GameLogic/PolygonTrigger.h"
#include "GameLogic/TerrainLogic.h"
#include <cmath>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

/* ********* PolygonTrigger class ****************************/
PolygonTrigger *PolygonTrigger::ThePolygonTriggerListPtr = NULL;
Int PolygonTrigger::s_currentID = 1;

PolygonTrigger::WorldState::~WorldState() noexcept
{
	if (m_head) m_head->deleteInstance();
}

void PolygonTrigger::exchangeWorldState(WorldState& state) noexcept
{
	std::swap(ThePolygonTriggerListPtr, state.m_head);
	std::swap(s_currentID, state.m_nextID);
}
/**
 PolygonTrigger - Constructor.
*/
PolygonTrigger::PolygonTrigger(Int initialAllocation) :
m_nextPolygonTrigger(NULL),
m_points(NULL),
m_numPoints(0),
m_sizePoints(0),
m_bounds{},
m_radius(0),
m_boundsNeedsUpdate(true),
m_exportWithScripts(false),
m_isWaterArea(false),
m_shouldRender(true),
m_selected(false),
//Added By Sadullah Nader
//Initializations inserted
m_isRiver(FALSE),
m_riverStart(0)
//
{
	if (initialAllocation < 2) initialAllocation = 2;
	if (s_currentID == std::numeric_limits<Int>::max()) throw ERROR_BAD_ARG;
	m_points = NEW ICoord3D[initialAllocation];		// pool[]ify
	m_sizePoints = initialAllocation;
	m_triggerID = s_currentID++;

	m_waterHandle.m_polygon = this;

}


/**
 PolygonTrigger - Destructor - note - if linked, deletes linked items.
*/
PolygonTrigger::~PolygonTrigger(void)
{
	if (m_points) {
		delete [] m_points;
		m_points = NULL;
	}
	if (m_nextPolygonTrigger) {
		PolygonTrigger *cur = m_nextPolygonTrigger;
		PolygonTrigger *next;
		while (cur) {
			next = cur->getNext();
			cur->setNextPoly(NULL); // prevents recursion. 
			cur->deleteInstance();
			cur = next; 
		}
	}
}


/**
 PolygonTrigger::reallocate - increases the size of the points list.
 NOTE: It is expected that this will only get called in the editor, as in the game
 the poly triggers don't change.
*/
void PolygonTrigger::reallocate(void)
{	
	DEBUG_ASSERTCRASH(m_numPoints <= m_sizePoints, ("Invalid m_numPoints."));
	if (m_numPoints == m_sizePoints) {
		// Reallocate.
		if (m_sizePoints > std::numeric_limits<Int>::max()/2) throw ERROR_OUT_OF_MEMORY;
		const Int nextSize = m_sizePoints * 2;
		ICoord3D *newPts = NEW ICoord3D[nextSize];
		Int i;
		for (i=0; i<m_numPoints; i++) {
			newPts[i] = m_points[i];
		}
		delete [] m_points;
		m_points = newPts;
		m_sizePoints = nextSize;
	}
}

/**
* Find the polygon trigger with the matching ID
*/
PolygonTrigger *PolygonTrigger::getPolygonTriggerByID(Int triggerID)
{

	for( PolygonTrigger *poly = PolygonTrigger::getFirstPolygonTrigger();
			 poly; poly = poly->getNext() )
		if( poly->getID() == triggerID )
			return poly;

	// not found
	return NULL;

}

/**
* PolygonTrigger::ParsePolygonTriggersDataChunk - read a polygon triggers chunk.
* Format is the newer CHUNKY format.
*	See PolygonTrigger::WritePolygonTriggersDataChunk for the writer.
*	Input: DataChunkInput 
*		
*/
Bool PolygonTrigger::ParsePolygonTriggersDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	if (!info || info->version < K_TRIGGERS_VERSION_1 || info->version > K_TRIGGERS_VERSION_4)
		throw ERROR_CORRUPT_FILE_FORMAT;
	// Original consumers see a legitimate candidate publication while parsing.
	// No callback/allocation occurs during rollback or ownership exchange.
	struct Transaction {
		WorldState previous;
		Bool keep = false;
		Transaction() { exchangeWorldState(previous); }
		~Transaction() noexcept { if (!keep) exchangeWorldState(previous); }
	} transaction;
	Int count;
	Int numPoints;
	Int triggerID;
	Int maxTriggerId = 0;
	Bool isWater;
	Bool isRiver;
	Int riverStart;
	AsciiString triggerName;
	AsciiString layerName;
	PolygonTrigger *pPrevTrig = NULL;
	ICoord3D loc;
	count = file.readInt(); 
	const unsigned minimumRecord = 10 + (info->version >= 2 ? 1 : 0) +
		(info->version >= 3 ? 5 : 0) + (info->version >= 4 ? 2 : 0);
	if (count < 0 || static_cast<unsigned>(count) > file.getChunkDataSizeLeft()/minimumRecord)
		throw ERROR_CORRUPT_FILE_FORMAT;
	while (count>0) {
		count--;
		triggerName = file.readAsciiString();
		if (info->version >= K_TRIGGERS_VERSION_4) {
			layerName = file.readAsciiString();
		}
		triggerID = file.readInt();
		if (triggerID == std::numeric_limits<Int>::max()) throw ERROR_CORRUPT_FILE_FORMAT;
		isWater = false;
		if (info->version >= K_TRIGGERS_VERSION_2) {
			isWater = file.readByte();
		}
		isRiver = false;
		riverStart = 0;
		if (info->version >= K_TRIGGERS_VERSION_3) {
			isRiver = file.readByte();
			riverStart = file.readInt();
		}

		numPoints = file.readInt(); 
		if (numPoints < 0 || static_cast<unsigned>(numPoints) > file.getChunkDataSizeLeft()/12)
			throw ERROR_CORRUPT_FILE_FORMAT;
		// Read physical points before growing; an envelope is not physical backing.
		struct Retire { void operator()(PolygonTrigger* p) const noexcept { if (p) p->deleteInstance(); } };
		std::unique_ptr<PolygonTrigger, Retire> owner(newInstance(PolygonTrigger)(2));
		PolygonTrigger *pTrig = owner.get();
		pTrig->setTriggerName(triggerName);
		if (info->version >= K_TRIGGERS_VERSION_4) {
			pTrig->setLayerName(layerName);
		}
		pTrig->setWaterArea(isWater);
		pTrig->setRiver(isRiver);
		pTrig->setRiverStart(riverStart);
		pTrig->m_triggerID = triggerID;
		if (triggerID > maxTriggerId) {
			maxTriggerId = triggerID;
		}
		Int i;
		for (i=0; i<numPoints; i++) {
			loc.x = file.readInt();
			loc.y = file.readInt();
			loc.z = file.readInt();
			pTrig->addPoint(loc);
		}
		if (numPoints<2) {
			continue;
		}
		if (pPrevTrig) {
			pPrevTrig->setNextPoly(pTrig);
		} else {
			PolygonTrigger::addPolygonTrigger(pTrig);
		}
		pPrevTrig = pTrig;
		owner.release();
	}
	if (info->version == K_TRIGGERS_VERSION_1) 
	{
		// before water areas existed, so create a default one.
		if (!TheGlobalData || maxTriggerId >= std::numeric_limits<Int>::max()-1)
			throw ERROR_CORRUPT_FILE_FORMAT;
		auto extent = [](Real value) -> Int {
			const Real result = 30*MAP_XY_FACTOR + value;
			if (!std::isfinite(result) || static_cast<double>(result) < std::numeric_limits<Int>::min() ||
				static_cast<double>(result) > std::numeric_limits<Int>::max()) throw ERROR_CORRUPT_FILE_FORMAT;
			return static_cast<Int>(result);
		};
		const Int right = extent(TheGlobalData->m_waterExtentX);
		const Int top = extent(TheGlobalData->m_waterExtentY);
		struct Retire { void operator()(PolygonTrigger* p) const noexcept { if (p) p->deleteInstance(); } };
		std::unique_ptr<PolygonTrigger, Retire> owner(newInstance(PolygonTrigger)(4));
		PolygonTrigger *pTrig = owner.get();
		pTrig->setWaterArea(true);
#ifdef _DEBUG
		pTrig->setTriggerName("AutoAddedWaterAreaTrigger");
#endif
		pTrig->m_triggerID = maxTriggerId++;
		loc.x = -30*MAP_XY_FACTOR;
		loc.y = -30*MAP_XY_FACTOR;
		loc.z = 7;  // The old water position.
		pTrig->addPoint(loc);
		loc.x = right;
		pTrig->addPoint(loc);
		loc.y = top;
		pTrig->addPoint(loc);
		loc.x = -30*MAP_XY_FACTOR;
		pTrig->addPoint(loc);
		if (pPrevTrig) {
			pPrevTrig->setNextPoly(pTrig);
		} else {
			PolygonTrigger::addPolygonTrigger(pTrig);
		}
		pPrevTrig = pTrig;
		owner.release();
	}
	s_currentID = maxTriggerId+1;
	if (!file.atEndOfChunk()) throw ERROR_CORRUPT_FILE_FORMAT;
	transaction.keep = true;
	return true;
}

/**
* PolygonTrigger::WritePolygonTriggersDataChunk - Writes a Polygon triggers chunk.
* Format is the newer CHUNKY format.
*	See PolygonTrigger::ParsePolygonTriggersDataChunk for the reader.
*	Input: DataChunkInput 
*		
*/
void PolygonTrigger::WritePolygonTriggersDataChunk(DataChunkOutput &chunkWriter)
{
	chunkWriter.openDataChunk("PolygonTriggers", 	K_TRIGGERS_VERSION_4);
		
		PolygonTrigger *pTrig;
		Int count = 0;
		for (pTrig=PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
			count++;
		}
		chunkWriter.writeInt(count); 
		for (pTrig=PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
			chunkWriter.writeAsciiString(pTrig->getTriggerName());	
			chunkWriter.writeAsciiString(pTrig->getLayerName());	
			chunkWriter.writeInt(pTrig->getID()); 
			chunkWriter.writeByte(pTrig->isWaterArea());
			chunkWriter.writeByte(pTrig->isRiver());
			chunkWriter.writeInt(pTrig->getRiverStart());
			chunkWriter.writeInt(pTrig->getNumPoints()); 
			Int i;
			for (i=0; i<pTrig->getNumPoints(); i++) {
				ICoord3D loc = *pTrig->getPoint(i);
				chunkWriter.writeInt( loc.x);
				chunkWriter.writeInt( loc.y);
				chunkWriter.writeInt( loc.z);
			}
		}

	chunkWriter.closeDataChunk();
}

/**
 PolygonTrigger::updateBounds - Updates the bounds.
*/
void PolygonTrigger::updateBounds(void)	const
{	
	const Int BIG_INT=0x7ffff0;
	m_bounds.lo.x = m_bounds.lo.y = BIG_INT;
	m_bounds.hi.x = m_bounds.hi.y = -BIG_INT;
	Int i;
	for (i=0; i<m_numPoints; i++) {
		if (m_points[i].x < m_bounds.lo.x) m_bounds.lo.x = m_points[i].x;
		if (m_points[i].y < m_bounds.lo.y) m_bounds.lo.y = m_points[i].y;
		if (m_points[i].x > m_bounds.hi.x) m_bounds.hi.x = m_points[i].x;
		if (m_points[i].y > m_bounds.hi.y) m_bounds.hi.y = m_points[i].y;
	}
	m_boundsNeedsUpdate = 0;
	// Preserve the source radius formula, including its Y sum, without signed UB.
	Real halfWidth = (static_cast<double>(m_bounds.hi.x) - m_bounds.lo.x) / 2.0f;
	Real halfHeight = (static_cast<double>(m_bounds.hi.y) + m_bounds.lo.y) / 2.0f;

	m_radius = sqrt(halfHeight*halfHeight + halfWidth*halfWidth);
}


/**
 PolygonTrigger::addPolygonTrigger adds a trigger to the list of triggers.
*/
void PolygonTrigger::addPolygonTrigger(PolygonTrigger *pTrigger)
{	
	PolygonTrigger *pTrig = getFirstPolygonTrigger();
	for (; pTrig; pTrig = pTrig->getNext()) {
		DEBUG_ASSERTCRASH(pTrig != pTrigger, ("Attempting to add trigger already in list."));
		if (pTrig==pTrigger) return;
	}
	pTrigger->m_nextPolygonTrigger = ThePolygonTriggerListPtr;
	ThePolygonTriggerListPtr = pTrigger;
}

/**
 PolygonTrigger::removePolygonTrigger removes a trigger to the list of 
	triggers.  note - does NOT delete pTrigger.
*/
void PolygonTrigger::removePolygonTrigger(PolygonTrigger *pTrigger)
{	
	PolygonTrigger *pPrev = NULL;
	PolygonTrigger *pTrig = getFirstPolygonTrigger();
	for (; pTrig; pTrig = pTrig->getNext()) {
		if (pTrig==pTrigger) break;
		pPrev = pTrig;
	}
	DEBUG_ASSERTCRASH(pTrig, ("Attempting to remove a polygon not in the list."));
	if (pTrig) {
		if (pPrev) {
			DEBUG_ASSERTCRASH(pTrigger==pPrev->m_nextPolygonTrigger, ("Logic errror.  jba."));
			pPrev->m_nextPolygonTrigger = pTrig->m_nextPolygonTrigger;
		} else {
			DEBUG_ASSERTCRASH(pTrigger==ThePolygonTriggerListPtr, ("Logic errror.  jba."));
			ThePolygonTriggerListPtr = pTrig->m_nextPolygonTrigger;
		}
	}
	pTrigger->m_nextPolygonTrigger = NULL;
}

/**
 PolygonTrigger::deleteTriggers Deletes list of triggers.
*/
void PolygonTrigger::deleteTriggers(void)
{
	PolygonTrigger *pList = ThePolygonTriggerListPtr;	
	ThePolygonTriggerListPtr = NULL;
	s_currentID = 1;
	if (pList) pList->deleteInstance();
}

/**
 PolygonTrigger::addPoint adds a point at the end of the polygon.
 NOTE: It is expected that this will only get called in the editor, as in the game
 the poly triggers don't change.
*/
void PolygonTrigger::addPoint(const ICoord3D &point)
{	
	DEBUG_ASSERTCRASH(m_numPoints <= m_sizePoints, ("Invalid m_numPoints."));
	if (m_numPoints == m_sizePoints) {
		reallocate();
	}
	m_points[m_numPoints] = point;
	m_numPoints++;
	m_boundsNeedsUpdate = true;
}

/**
 PolygonTrigger::setPoint sets the point at index ndx.
 NOTE: It is expected that this will only get called in the editor, as in the game
 the poly triggers don't change.
*/
void PolygonTrigger::setPoint(const ICoord3D &point, Int ndx)
{	
	DEBUG_ASSERTCRASH(ndx>=0 && ndx <= m_numPoints, ("Invalid ndx."));
	if (ndx<0) return;
	if (ndx == m_numPoints) {	// we are setting first available unused point
		addPoint(point);
		return;
	}
	if (ndx>m_numPoints) { // Can't skip points.
		return;
	}
	m_points[ndx] = point;
	m_boundsNeedsUpdate = true;
}

/**
 PolygonTrigger::insertPoint .
 NOTE: It is expected that this will only get called in the editor, as in the game
 the poly triggers don't change.
*/
void PolygonTrigger::insertPoint(const ICoord3D &point, Int ndx)
{	
	DEBUG_ASSERTCRASH(ndx>=0 && ndx <= m_numPoints, ("Invalid ndx."));
	if (ndx<0) return;
	if (ndx>m_numPoints) return;
	if (ndx == m_numPoints) {	// we are setting first available unused point
		addPoint(point);
		return;
	}
	if (m_numPoints == m_sizePoints) {
		reallocate();
	}
	Int i;
	for (i=m_numPoints; i>ndx; i--) {
		m_points[i] = m_points[i-1];
	}
	m_points[ndx] = point;
	m_numPoints++;
	m_boundsNeedsUpdate = true;
}

/**
 PolygonTrigger::deletePoint .
 NOTE: It is expected that this will only get called in the editor, as in the game
 the poly triggers don't change.
*/
void PolygonTrigger::deletePoint(Int ndx)
{	
	DEBUG_ASSERTCRASH(ndx>=0 && ndx < m_numPoints, ("Invalid ndx."));
	if (ndx<0 || ndx>=m_numPoints) return;
	Int i;
	for (i=ndx; i<m_numPoints-1; i++) {
		m_points[i] = m_points[i+1];
	}
	m_numPoints--;
	m_boundsNeedsUpdate = true;
}

void PolygonTrigger::getCenterPoint(Coord3D* pOutCoord)	const
{
	DEBUG_ASSERTCRASH(pOutCoord != NULL, ("pOutCoord was null. Non-Fatal, but shouldn't happen."));
	if (!pOutCoord) {
		return;
	}

	if (m_boundsNeedsUpdate) {
		updateBounds();
	}
	(*pOutCoord).x = (static_cast<double>(m_bounds.lo.x) + m_bounds.hi.x) / 2.0f;
	(*pOutCoord).y = (static_cast<double>(m_bounds.lo.y) + m_bounds.hi.y) / 2.0f;

	(*pOutCoord).z = TheTerrainLogic->getGroundHeight(pOutCoord->x, pOutCoord->y);
}

Real PolygonTrigger::getRadius(void)	const
{
	if (m_boundsNeedsUpdate) {
		updateBounds();
	}
	return m_radius;
}


/**
 PolygonTrigger - pointInTrigger.
*/
Bool PolygonTrigger::pointInTrigger(ICoord3D &point) const
{	
	if (m_boundsNeedsUpdate) {
		updateBounds();
	}
	if (point.x < m_bounds.lo.x) return false;
	if (point.y < m_bounds.lo.y) return false;
	if (point.x > m_bounds.hi.x) return false;
	if (point.y > m_bounds.hi.y) return false;

	Bool inside = false;
	Int i;
	for (i=0; i<m_numPoints; i++) {
		ICoord3D pt1 = m_points[i];
		ICoord3D pt2;
		if (i==m_numPoints-1) {
			pt2 = m_points[0];
		} else {
			pt2 = m_points[i+1];
		}
		if (pt1.y == pt2.y) {
			continue; // ignore horizontal lines.
		}
		if (pt1.y < point.y && pt2.y < point.y) continue;
		if (pt1.y >= point.y && pt2.y >= point.y) continue;
		if (pt1.x<point.x && pt2.x < point.x) continue;
		// Line segment crosses ray from point x->infinity.
		// Retain exact source integer products/rounding whenever they are defined.
		const auto dy = static_cast<Int64>(pt2.y)-pt1.y;
		const auto dx = static_cast<Int64>(pt2.x)-pt1.x;
		const auto offset = static_cast<Int64>(point.y)-pt1.y;
		const double product = static_cast<double>(dx)*static_cast<double>(offset);
		Real intersectionX;
		if (dy >= std::numeric_limits<Int>::min() && dy <= std::numeric_limits<Int>::max() &&
			dx >= std::numeric_limits<Int>::min() && dx <= std::numeric_limits<Int>::max() &&
			offset >= std::numeric_limits<Int>::min() && offset <= std::numeric_limits<Int>::max() &&
			product >= std::numeric_limits<Int>::min() && product <= std::numeric_limits<Int>::max())
			intersectionX = pt1.x + (static_cast<Int>(dx)*static_cast<Int>(offset))/static_cast<Real>(dy);
		else intersectionX = static_cast<Real>(pt1.x + product/static_cast<double>(dy));
		if (intersectionX >= point.x) {
			inside = !inside;
		}
	}
	return inside;
}

// ------------------------------------------------------------------------------------------------
const WaterHandle* PolygonTrigger::getWaterHandle(void)	const
{

	if( isWaterArea() )
		return &m_waterHandle;

	return NULL;  // this polygon trigger is not a water area

}

Bool PolygonTrigger::isValid(void) const
{
	if (m_numPoints == 0) {
		return FALSE;
	}

	return TRUE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void PolygonTrigger::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void PolygonTrigger::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// number of data points
	Int count = m_numPoints;
	xfer->xferInt( &count );
	if (count < 0) throw ERROR_CORRUPT_FILE_FORMAT;
	// Serialized counts are not constrained by an internal allocation strategy.
	// Grow only after complete physical records; publish after all fields succeed.
	const Bool loading = xfer->getXferMode() == XFER_LOAD;
	std::vector<ICoord3D> candidate;

	// xfer all data points
	ICoord3D *point;
	for( Int i = 0; i < count; ++i )
	{

		// get this point
		ICoord3D decoded{};
		point = loading ? &decoded : &m_points[ i ];

		// xfer point
		xfer->xferICoord3D( point );
		if (loading) candidate.push_back(decoded);

	}  // end for, i

	// bounds
	IRegion2D bounds = m_bounds;
	xfer->xferIRegion2D( &bounds );

	// radius
	Real radius = m_radius;
	xfer->xferReal( &radius );

	// bounds need update
	Bool needsUpdate = m_boundsNeedsUpdate;
	xfer->xferBool( &needsUpdate );
	if (loading) {
		std::unique_ptr<ICoord3D[]> larger;
		if (count > m_sizePoints) larger.reset(NEW ICoord3D[count]);
		ICoord3D* backing = larger ? larger.get() : m_points;
		for (Int i = 0; i < count; ++i) backing[i] = candidate[i];
		if (larger) {
			delete[] m_points;
			m_points = larger.release();
			m_sizePoints = count;
		}
		m_numPoints = count;
		m_bounds = bounds;
		m_radius = radius;
		m_boundsNeedsUpdate = needsUpdate;
	}

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void PolygonTrigger::loadPostProcess( void )
{

}  // end loadPostProcess
