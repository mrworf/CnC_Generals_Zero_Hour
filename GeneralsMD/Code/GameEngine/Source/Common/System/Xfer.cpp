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

// FILE: Xfer.cpp /////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   The Xfer system is capable of setting up operations to work with blocks of data
//				 from other subsystems.  It can work things such as file reading, file writing,
//				 CRC computations etc
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "Common/KindOf.h"
#include "WWMath/matrix3d.h"
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Upgrade.h"
#include "Common/NativeTransferServices.h"
#include "Common/Xfer.h"
#include "Common/BitFlagsIO.h"
#include "Common/NativeTransferWire.h"
#include <limits>

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/** Open */
// ------------------------------------------------------------------------------------------------
void XferBase::open( AsciiString identifier )
{

	// save identifier
	m_identifier = identifier;

}  // end open

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferByte( Byte *byteData )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!byteData) throw XFER_INVALID_PARAMETERS;

	xferImplementation( byteData, sizeof( Byte ) ); 

}  // end xferByte

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferVersion( XferVersion *versionData, XferVersion currentVersion )
{
    FailureScope failure(*this);
    if (m_failed || !versionData) throw XFER_INVALID_PARAMETERS;
    if (getXferMode()==XFER_LOAD) {
        XferVersion candidate=0;
        xferImplementation(&candidate,sizeof candidate);
        if (candidate>currentVersion) throw XFER_INVALID_VERSION;
        *versionData=candidate;
    } else {
        if (*versionData>currentVersion) throw XFER_INVALID_VERSION;
        xferImplementation(versionData,sizeof *versionData);
    }

}  // end xferVersion

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferUnsignedByte( UnsignedByte *unsignedByteData )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!unsignedByteData) throw XFER_INVALID_PARAMETERS;

	xferImplementation( unsignedByteData, sizeof( UnsignedByte ) ); 

}  // end xferUnsignedByte

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferBool( Bool *boolData )
{
    FailureScope failure(*this);
    if (m_failed || !boolData) throw XFER_INVALID_PARAMETERS;
    UnsignedByte encoded=0;
    if (getXferMode()==XFER_LOAD) {
        xferImplementation(&encoded,1);
        if (encoded>1) throw XFER_INVALID_PARAMETERS;
        *boolData=encoded!=0;
    } else {
        encoded=*boolData ? 1 : 0;
        xferImplementation(&encoded,1);
    }

}  // end xferBool

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferInt( Int *intData )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!intData) throw XFER_INVALID_PARAMETERS;

	xferImplementation( intData, sizeof( Int ) );
	
}  // end xferInt

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferInt64( Int64 *int64Data )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!int64Data) throw XFER_INVALID_PARAMETERS;

	xferImplementation( int64Data, sizeof( Int64 ) );

}  // end xferInt64

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferUnsignedInt( UnsignedInt *unsignedIntData )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!unsignedIntData) throw XFER_INVALID_PARAMETERS;

	xferImplementation( unsignedIntData, sizeof( UnsignedInt ) );
	
}  // end xferUnsignedInt

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferShort( Short *shortData )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!shortData) throw XFER_INVALID_PARAMETERS;

	xferImplementation( shortData, sizeof( Short ) ); 

}  // end xferShort

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferUnsignedShort( UnsignedShort *unsignedShortData )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!unsignedShortData) throw XFER_INVALID_PARAMETERS;

	xferImplementation( unsignedShortData, sizeof( UnsignedShort ) ); 

}  // end xferUnsignedShort

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferReal( Real *realData )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!realData) throw XFER_INVALID_PARAMETERS;

	xferImplementation( realData, sizeof( Real ) ); 
	
}  // end xferReal

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferMapName( AsciiString *mapNameData )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    const auto services=nativeTransferServices();
    if (!mapNameData || ((getXferMode()==XFER_SAVE || getXferMode()==XFER_LOAD)
        && !services.owner)) throw XFER_INVALID_PARAMETERS;
	if (getXferMode() == XFER_SAVE)
	{
		AsciiString tmp = services.encodeMap(services.owner,*mapNameData);
		xferAsciiString(&tmp);
	}
	else if (getXferMode() == XFER_LOAD)
	{
        AsciiString candidate;
        xferAsciiString(&candidate);
        *mapNameData=services.decodeMap(services.owner,candidate);
	}
}  // end xferAsciiString

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferAsciiString( AsciiString *asciiStringData )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!asciiStringData) throw XFER_INVALID_PARAMETERS;

	xferImplementation( (void *)asciiStringData->str(), sizeof( Byte ) * asciiStringData->getLength() );

}  // end xferAsciiString

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferMarkerLabel( AsciiString asciiStringData )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
}  // end xferMarkerLabel

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferUnicodeString( UnicodeString *unicodeStringData )
{
    FailureScope failure(*this);
    if (m_failed || !unicodeStringData) throw XFER_INVALID_PARAMETERS;
    auto wire=nativeTransferUTF16(*unicodeStringData,
        static_cast<std::size_t>(std::numeric_limits<Int>::max())/2);
    xferImplementation(wire.data(),static_cast<Int>(wire.size()));

}  // end xferUnicodeString

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferCoord3D( Coord3D *coord3D )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!coord3D) throw XFER_INVALID_PARAMETERS;
    Coord3D candidate{};
    auto* target=getXferMode()==XFER_LOAD ? &candidate : coord3D;

	xferReal( &target->x );
	xferReal( &target->y );
	xferReal( &target->z );

    if (getXferMode()==XFER_LOAD) *coord3D=candidate;

}  // end xferCoord3D

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferICoord3D( ICoord3D *iCoord3D )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!iCoord3D) throw XFER_INVALID_PARAMETERS;
    ICoord3D candidate{};
    auto* target=getXferMode()==XFER_LOAD ? &candidate : iCoord3D;

	xferInt( &target->x );
	xferInt( &target->y );
	xferInt( &target->z );

    if (getXferMode()==XFER_LOAD) *iCoord3D=candidate;

}  // end xferICoor3D

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferRegion3D( Region3D *region3D )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!region3D) throw XFER_INVALID_PARAMETERS;
    Region3D candidate{};
    auto* target=getXferMode()==XFER_LOAD ? &candidate : region3D;

	xferCoord3D( &target->lo );
	xferCoord3D( &target->hi );

    if (getXferMode()==XFER_LOAD) *region3D=candidate;

}  // end xferRegion3D

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferIRegion3D( IRegion3D *iRegion3D )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!iRegion3D) throw XFER_INVALID_PARAMETERS;
    IRegion3D candidate{};
    auto* target=getXferMode()==XFER_LOAD ? &candidate : iRegion3D;

	xferICoord3D( &target->lo );
	xferICoord3D( &target->hi );

    if (getXferMode()==XFER_LOAD) *iRegion3D=candidate;

}  // end xferIRegion3D

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferCoord2D( Coord2D *coord2D )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!coord2D) throw XFER_INVALID_PARAMETERS;
    Coord2D candidate{};
    auto* target=getXferMode()==XFER_LOAD ? &candidate : coord2D;

	xferReal( &target->x );
	xferReal( &target->y );

    if (getXferMode()==XFER_LOAD) *coord2D=candidate;

}  // end xferCoord2D

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferICoord2D( ICoord2D *iCoord2D )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!iCoord2D) throw XFER_INVALID_PARAMETERS;
    ICoord2D candidate{};
    auto* target=getXferMode()==XFER_LOAD ? &candidate : iCoord2D;

	xferInt( &target->x );
	xferInt( &target->y );

    if (getXferMode()==XFER_LOAD) *iCoord2D=candidate;

}  // end xferICoord2D

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferRegion2D( Region2D *region2D )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!region2D) throw XFER_INVALID_PARAMETERS;
    Region2D candidate{};
    auto* target=getXferMode()==XFER_LOAD ? &candidate : region2D;

	xferCoord2D( &target->lo );
	xferCoord2D( &target->hi );

    if (getXferMode()==XFER_LOAD) *region2D=candidate;

}  // end xferRegion2D

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferIRegion2D( IRegion2D *iRegion2D )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!iRegion2D) throw XFER_INVALID_PARAMETERS;
    IRegion2D candidate{};
    auto* target=getXferMode()==XFER_LOAD ? &candidate : iRegion2D;

	xferICoord2D( &target->lo );
	xferICoord2D( &target->hi );

    if (getXferMode()==XFER_LOAD) *iRegion2D=candidate;

}  // end xferIRegion2D

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferRealRange( RealRange *realRange )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!realRange) throw XFER_INVALID_PARAMETERS;
    RealRange candidate{};
    auto* target=getXferMode()==XFER_LOAD ? &candidate : realRange;

	xferReal( &target->lo );
	xferReal( &target->hi );

    if (getXferMode()==XFER_LOAD) *realRange=candidate;

}  // end xferRealRange

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferColor( Color *color )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!color) throw XFER_INVALID_PARAMETERS;

	xferImplementation( color, sizeof( Color ) );

}  // end xferColor

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferRGBColor( RGBColor *rgbColor )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!rgbColor) throw XFER_INVALID_PARAMETERS;
    RGBColor candidate{};
    auto* target=getXferMode()==XFER_LOAD ? &candidate : rgbColor;

	xferReal( &target->red );
	xferReal( &target->green );
	xferReal( &target->blue );

    if (getXferMode()==XFER_LOAD) *rgbColor=candidate;

}  // end xferRGBColor

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferRGBAColorReal( RGBAColorReal *rgbaColorReal )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!rgbaColorReal) throw XFER_INVALID_PARAMETERS;
    RGBAColorReal candidate{};
    auto* target=getXferMode()==XFER_LOAD ? &candidate : rgbaColorReal;

	xferReal( &target->red );
	xferReal( &target->green );
	xferReal( &target->blue );
	xferReal( &target->alpha );

    if (getXferMode()==XFER_LOAD) *rgbaColorReal=candidate;

}  // end xferRGBAColorReal

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferRGBAColorInt( RGBAColorInt *rgbaColorInt )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!rgbaColorInt) throw XFER_INVALID_PARAMETERS;
    RGBAColorInt candidate{};
    auto* target=getXferMode()==XFER_LOAD ? &candidate : rgbaColorInt;

	xferUnsignedInt( &target->red );
	xferUnsignedInt( &target->green );
	xferUnsignedInt( &target->blue );
	xferUnsignedInt( &target->alpha );

    if (getXferMode()==XFER_LOAD) *rgbaColorInt=candidate;

}  // end xferRGBAColorInt

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferObjectID( ObjectID *objectID )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!objectID) throw XFER_INVALID_PARAMETERS;

	xferImplementation( objectID, sizeof( ObjectID ) );

}  // end xferObjeftID

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferDrawableID( DrawableID *drawableID )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!drawableID) throw XFER_INVALID_PARAMETERS;

	xferImplementation( drawableID, sizeof( DrawableID ) );

}  // end xferDrawableID


// ------------------------------------------------------------------------------------------------
void XferBase::xferSTLObjectIDVector( std::vector<ObjectID> *objectIDVectorData )
{
    FailureScope failure(*this);
    if (m_failed || !objectIDVectorData) throw XFER_INVALID_PARAMETERS;
    const auto mode=getXferMode();
    if (mode!=XFER_LOAD && mode!=XFER_SAVE && mode!=XFER_CRC) throw XFER_MODE_UNKNOWN;
    if (mode==XFER_LOAD && !objectIDVectorData->empty()) throw XFER_LIST_NOT_EMPTY;
    if (mode!=XFER_LOAD && objectIDVectorData->size()>std::numeric_limits<UnsignedShort>::max())
        throw XFER_INVALID_PARAMETERS;
    XferVersion version=1;
    xferVersion(&version,1);
    UnsignedShort count=mode==XFER_LOAD ? 0 : static_cast<UnsignedShort>(objectIDVectorData->size());
    xferUnsignedShort(&count);
    if (mode==XFER_LOAD) {
        std::vector<ObjectID> candidate;
        for (UnsignedInt i=0;i<count;++i) {
            ObjectID value{};
            xferObjectID(&value);
            candidate.push_back(value);
        }
        objectIDVectorData->swap(candidate);
    } else {
        for (auto value : *objectIDVectorData) xferObjectID(&value);
    }
}

// ------------------------------------------------------------------------------------------------
/** STL Object ID list (cause it's a common data structure we use a lot)
	* Version Info;
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void XferBase::xferSTLObjectIDList( std::list<ObjectID> *objectIDListData )
{
    FailureScope failure(*this);
    if (m_failed || !objectIDListData) throw XFER_INVALID_PARAMETERS;
    const auto mode=getXferMode();
    if (mode!=XFER_LOAD && mode!=XFER_SAVE && mode!=XFER_CRC) throw XFER_MODE_UNKNOWN;
    if (mode==XFER_LOAD && !objectIDListData->empty()) throw XFER_LIST_NOT_EMPTY;
    if (mode!=XFER_LOAD && objectIDListData->size()>std::numeric_limits<UnsignedShort>::max())
        throw XFER_INVALID_PARAMETERS;
    XferVersion version=1;
    xferVersion(&version,1);
    UnsignedShort count=mode==XFER_LOAD ? 0 : static_cast<UnsignedShort>(objectIDListData->size());
    xferUnsignedShort(&count);
    if (mode==XFER_LOAD) {
        std::list<ObjectID> candidate;
        for (UnsignedInt i=0;i<count;++i) {
            ObjectID value{};
            xferObjectID(&value);
            candidate.push_back(value);
        }
        objectIDListData->swap(candidate);
    } else {
        for (auto value : *objectIDListData) xferObjectID(&value);
    }
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferSTLIntList( std::list<Int> *intListData )
{
    FailureScope failure(*this);
    if (m_failed || !intListData) throw XFER_INVALID_PARAMETERS;
    const auto mode=getXferMode();
    if (mode!=XFER_LOAD && mode!=XFER_SAVE && mode!=XFER_CRC) throw XFER_MODE_UNKNOWN;
    if (mode==XFER_LOAD && !intListData->empty()) throw XFER_LIST_NOT_EMPTY;
    if (mode!=XFER_LOAD && intListData->size()>std::numeric_limits<UnsignedShort>::max())
        throw XFER_INVALID_PARAMETERS;
    XferVersion version=1;
    xferVersion(&version,1);
    UnsignedShort count=mode==XFER_LOAD ? 0 : static_cast<UnsignedShort>(intListData->size());
    xferUnsignedShort(&count);
    if (mode==XFER_LOAD) {
        std::list<Int> candidate;
        for (UnsignedInt i=0;i<count;++i) {
            Int value{};
            xferInt(&value);
            candidate.push_back(value);
        }
        intListData->swap(candidate);
    } else {
        for (auto value : *intListData) xferInt(&value);
    }
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferScienceType( ScienceType *science )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    const auto services=nativeTransferServices();
    if (!science || (getXferMode()!=XFER_CRC && !services.owner)) throw XFER_INVALID_PARAMETERS;

	// sanity
	DEBUG_ASSERTCRASH( science != NULL, ("xferScienceType - Invalid parameters\n") );

	AsciiString scienceName;

	if( getXferMode() == XFER_SAVE )
	{
		// translate to string
		scienceName = services.encodeScience(services.owner,*science);

		// write the string
		xferAsciiString( &scienceName );

	}  // end if, save
	else if( getXferMode() == XFER_LOAD )
	{
		xferAsciiString( &scienceName );

		// translate to science
		const auto candidate = services.decodeScience(services.owner,scienceName);
		if( candidate == SCIENCE_INVALID )
		{

			DEBUG_CRASH(( "xferScienceType - Unknown science '%s'\n", scienceName.str() ));
			throw XFER_UNKNOWN_STRING;

		}  // end if
			
        *science=candidate;
	}  // end else if, load
	else if( getXferMode() == XFER_CRC )
	{
			xferImplementation( science, sizeof( *science ) );

	}  // end else if, crc
	else
	{

		DEBUG_CRASH(( "xferScienceType - Unknown xfer mode '%d'\n", getXferMode() ));
		throw XFER_MODE_UNKNOWN;

	}  // end else

}  // end xferScienceType

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferScienceVec( ScienceVec *scienceVec )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!scienceVec) throw XFER_INVALID_PARAMETERS;

	// sanity
	DEBUG_ASSERTCRASH( scienceVec != NULL, ("xferScienceVec - Invalid parameters\n") );

	// this deserves a version number
	const XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xferVersion( &version, currentVersion );

	// count of vector
	if (getXferMode()!=XFER_LOAD && scienceVec->size()>std::numeric_limits<UnsignedShort>::max())
        throw XFER_INVALID_PARAMETERS;
    UnsignedShort count = getXferMode()==XFER_LOAD ? 0 : static_cast<UnsignedShort>(scienceVec->size());
	xferUnsignedShort( &count );

	if( getXferMode() == XFER_SAVE )
	{
		for( ScienceVec::const_iterator it = scienceVec->begin(); it != scienceVec->end(); ++it )
		{
			ScienceType science = *it;
			xferScienceType(&science);
		}
	}
	else if( getXferMode() == XFER_LOAD )
	{
        ScienceVec candidate;
        for (UnsignedInt i=0;i<count;++i) {
            ScienceType science=SCIENCE_INVALID;
            xferScienceType(&science);
            candidate.push_back(science);
        }
        scienceVec->swap(candidate);
    }

	else if( getXferMode() == XFER_CRC )
	{
		for( ScienceVec::const_iterator it = scienceVec->begin(); it != scienceVec->end(); ++it )
		{
			ScienceType science = *it;
			xferImplementation( &science, sizeof( ScienceType ) );
		}  // end for, it
	}  // end else if, crc
	else
	{

		DEBUG_CRASH(( "xferScienceVec - Unknown xfer mode '%d'\n", getXferMode() ));
		throw XFER_MODE_UNKNOWN;

	}  // end else

}  // end xferScienceVec

// ------------------------------------------------------------------------------------------------
/** kind of type, for load/save it is xfered as a string so we can reorder the 
	* kindofs if we like
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void XferBase::xferKindOf( KindOfType *kindOfData )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!kindOfData) throw XFER_INVALID_PARAMETERS;

	// this deserves a version number
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xferVersion( &version, currentVersion );

	// check which type of xfer we're doing
	if( getXferMode() == XFER_SAVE )
	{

		// save as an ascii string
		AsciiString kindOfName = KindOfMaskType::getNameFromSingleBit(*kindOfData);
		xferAsciiString( &kindOfName );

	}  // end if, save
	else if( getXferMode() == XFER_LOAD )
	{
		
		// read ascii string from file
		AsciiString kindOfName;
		xferAsciiString( &kindOfName );

		// turn kind of name into an enum value
		Int bit = KindOfMaskType::getSingleBitFromName(kindOfName.str());
		if (bit != -1)
			*kindOfData = (KindOfType)bit;
				
	}  // end else if, load
	else if( getXferMode() == XFER_CRC )
	{

		// just call the xfer implementation on the data values
		xferImplementation( kindOfData, sizeof( KindOfType ) );

	}  // end else if, crc
	else
	{

		DEBUG_CRASH(( "xferKindOf - Unknown xfer mode '%d'\n", getXferMode() ));
		throw XFER_MODE_UNKNOWN;

	}  // end else

}  // end xferKindOf

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferUpgradeMask( UpgradeMaskType *upgradeMaskData )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    const auto services=nativeTransferServices();
    if (!upgradeMaskData || (getXferMode()!=XFER_CRC && !services.owner)) throw XFER_INVALID_PARAMETERS;

	// this deserves a version number
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xferVersion( &version, currentVersion );

	//Kris: The Upgrade system has been converted from Int64 to BitFlags. However because the 
	//names of upgrades are saved to preserve order reassignments (inserting a new upgrade in
	//the INI file will skew the bit values), we must continue saving the names of the upgrades
	//in order to recalculate the actual bit value of said upgrade.
	//---------------------------------------------------------------------------------------------
	//NOTE: The xfer code didn't have to change with the bitset upgrades, because either way, we're 
	//converting data <-> Ascii, so the minor syntax works with the before and after code!

	// check which type of xfer we're doing
	if( getXferMode() == XFER_SAVE )
	{
        auto names=services.encodeUpgrades(services.owner,*upgradeMaskData);
        if (names.size()>std::numeric_limits<UnsignedShort>::max()) throw XFER_INVALID_PARAMETERS;
        auto count=static_cast<UnsignedShort>(names.size());
        xferUnsignedShort(&count);
        for (auto& name : names) xferAsciiString(&name);
	}  // end if, save
	else if( getXferMode() == XFER_LOAD )
	{
		AsciiString upgradeName;


		// how many strings are we going to read from the file
		UnsignedShort count;
		xferUnsignedShort( &count );

		// zero the mask data
		UpgradeMaskType candidate;
        candidate.clear();

		// read all the strings and set the mask vaules
		for( UnsignedShort i = 0; i < count; ++i )
		{

			// read the string
			xferAsciiString( &upgradeName );

            candidate.set(services.decodeUpgrade(services.owner,upgradeName));

		}  // end for i

        *upgradeMaskData=candidate;
	}  // end else if, load
	else if( getXferMode() == XFER_CRC )
	{

        std::array<UnsignedInt,(UPGRADE_MAX_COUNT+31)/32> words{};
        static_assert(sizeof(UnsignedInt)==4);
        for (Int i=0;i<UPGRADE_MAX_COUNT;++i)
            if (upgradeMaskData->test(i)) words[i/32]|=UnsignedInt{1}<<(i%32);
        xferImplementation(words.data(),static_cast<Int>(words.size()*sizeof(UnsignedInt)));

	}  // end else if, crc
	else
	{

		DEBUG_CRASH(( "xferUpgradeMask - Unknown xfer mode '%d'\n", getXferMode() ));
		throw XFER_MODE_UNKNOWN;

	}  // end else
	
}  // end xferUpgradeMask

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferUser( void *data, Int dataSize )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;

	xferImplementation( data, dataSize );

}  // end xferUser

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void XferBase::xferMatrix3D( Matrix3D* mtx )
{
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (!mtx) throw XFER_INVALID_PARAMETERS;
    Matrix3D candidate(true);
    auto* target=getXferMode()==XFER_LOAD ? &candidate : mtx;
	// this deserves a version number
	const XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xferVersion( &version, currentVersion );

    Vector4& tmp0 = (*target)[0];
    Vector4& tmp1 = (*target)[1];
    Vector4& tmp2 = (*target)[2];

	xferReal(&tmp0.X);
	xferReal(&tmp0.Y);
	xferReal(&tmp0.Z);
	xferReal(&tmp0.W);

	xferReal(&tmp1.X);
	xferReal(&tmp1.Y);
	xferReal(&tmp1.Z);
	xferReal(&tmp1.W);

	xferReal(&tmp2.X);
	xferReal(&tmp2.Y);
	xferReal(&tmp2.Z);
	xferReal(&tmp2.W);
    if (getXferMode()==XFER_LOAD) *mtx=candidate;
}
