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

// FILE: Xfer.h ///////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   The Xfer system is capable of setting up operations to work with blocks of data
//				 from other subsystems.  It can work things such as file reading, file writing,
//				 CRC computations etc
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __XFER_H_
#define __XFER_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/STLTypedefs.h"
#include "Common/Science.h"
#include "Common/Upgrade.h"
#include <exception>

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Snapshot;
typedef Int Color;
enum ObjectID : UnsignedInt;
enum DrawableID : UnsignedInt;
enum KindOfType : Int;
#include "Common/ScienceType.h"
class Matrix3D;

// ------------------------------------------------------------------------------------------------
typedef UnsignedByte XferVersion;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
enum XferMode
{
	XFER_INVALID = 0,

	XFER_SAVE,
	XFER_LOAD,
	XFER_CRC,

	NUM_XFER_TYPES  // please keep this last
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
enum XferStatus
{
	XFER_STATUS_INVALID = 0,
	
	XFER_OK,														///< all is green and good
	XFER_EOF,														///< end of file encountered
	XFER_FILE_NOT_FOUND,								///< requested file does not exist
	XFER_FILE_NOT_OPEN,									///< file was not open
	XFER_FILE_ALREADY_OPEN,							///< this xfer is already open
	XFER_READ_ERROR,										///< error reading from file
	XFER_WRITE_ERROR,										///< error writing to file
	XFER_MODE_UNKNOWN,									///< unknown xfer mode
	XFER_SKIP_ERROR,										///< error skipping file
	XFER_BEGIN_END_MISMATCH,						///< mismatched pair calls of begin/end block
	XFER_OUT_OF_MEMORY,									///< out of memory
	XFER_STRING_ERROR,									///< error with strings
	XFER_INVALID_VERSION,								///< invalid version encountered
	XFER_INVALID_PARAMETERS,						///< invalid parameters
	XFER_LIST_NOT_EMPTY,								///< trying to xfer into a list that should be empty, but isn't
	XFER_UNKNOWN_STRING,								///< unrecognized string value
	
	XFER_ERROR_UNKNOWN,									///< unknown error (isn't that useful!)

	NUM_XFER_STATUS  // please keep this last
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
enum XferOptions
{
	XO_NONE										= 0x00000000,
	XO_NO_POST_PROCESSING			= 0x00000001,

	XO_ALL										= 0xFFFFFFFF  // keep this last please
};

///////////////////////////////////////////////////////////////////////////////////////////////////
typedef Int XferBlockSize;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class Xfer
{

public:
	class FailureScope {
		Xfer& m_owner;
		int m_exceptions;
	public:
		explicit FailureScope(Xfer& owner) noexcept
			: m_owner(owner), m_exceptions(std::uncaught_exceptions()) {}
		~FailureScope() noexcept {
			if (std::uncaught_exceptions()>m_exceptions) m_owner.m_failed=true;
		}
	};

	Xfer() : m_options(XO_NONE), m_xferMode(XFER_INVALID) {}
	virtual ~Xfer() = default;

	virtual XferMode getXferMode( void ) { return m_xferMode; }
	AsciiString getIdentifier( void ) { return m_identifier; }

	// xfer management
	virtual void setOptions( UnsignedInt options ) { BitSet( m_options, options ); }
	virtual void clearOptions( UnsignedInt options ) { BitClear( m_options, options ); }
	virtual UnsignedInt getOptions( void ) { return m_options; }
	virtual void open( AsciiString identifier ) = 0;		///< xfer open event
	virtual void close( void ) = 0;											///< xfer close event
	virtual Int beginBlock( void ) = 0;									///< xfer begin block event
	virtual void endBlock( void ) = 0;									///< xfer end block event
	virtual void skip( Int dataSize ) = 0;							///< xfer skip data

	virtual void xferSnapshot( Snapshot *snapshot ) = 0;		///< entry point for xfering a snapshot

	//
	// default transfer methods, these call the implementation method with the data
	// parameters.  You may use the default, or derive and create new ways to xfer each
	// of these types of data
	//
	virtual void xferVersion( XferVersion *versionData, XferVersion currentVersion ) = 0;
	virtual void xferByte( Byte *byteData ) = 0;
	virtual void xferUnsignedByte( UnsignedByte *unsignedByteData ) = 0;
	virtual void xferBool( Bool *boolData ) = 0;
	virtual void xferInt( Int *intData ) = 0;
	virtual void xferInt64( Int64 *int64Data ) = 0;
	virtual void xferUnsignedInt( UnsignedInt *unsignedIntData ) = 0;
	virtual void xferShort( Short *shortData ) = 0;
	virtual void xferUnsignedShort( UnsignedShort *unsignedShortData ) = 0;
	virtual void xferReal( Real *realData ) = 0;
	virtual void xferMarkerLabel( AsciiString asciiStringData ) = 0; // This is purely for readability purposes - it is explicitly discarded on load.
	virtual void xferAsciiString( AsciiString *asciiStringData ) = 0;
	virtual void xferUnicodeString( UnicodeString *unicodeStringData ) = 0;
	virtual void xferCoord3D( Coord3D *coord3D ) = 0;
	virtual void xferICoord3D( ICoord3D *iCoord3D ) = 0;
	virtual void xferRegion3D( Region3D *region3D ) = 0;
	virtual void xferIRegion3D( IRegion3D *iRegion3D ) = 0;
	virtual void xferCoord2D( Coord2D *coord2D ) = 0;
	virtual void xferICoord2D( ICoord2D *iCoord2D ) = 0;
	virtual void xferRegion2D( Region2D *region2D ) = 0;
	virtual void xferIRegion2D( IRegion2D *iRegion2D ) = 0;
	virtual void xferRealRange( RealRange *realRange ) = 0;
	virtual void xferColor( Color *color ) = 0;
	virtual void xferRGBColor( RGBColor *rgbColor ) = 0;
	virtual void xferRGBAColorReal( RGBAColorReal *rgbaColorReal ) = 0;
	virtual void xferRGBAColorInt( RGBAColorInt *rgbaColorInt ) = 0;
	virtual void xferObjectID( ObjectID *objectID ) = 0;
	virtual void xferDrawableID( DrawableID *drawableID ) = 0;
	virtual void xferSTLObjectIDVector( std::vector<ObjectID> *objectIDVectorData ) = 0;
	virtual void xferSTLObjectIDList( std::list< ObjectID > *objectIDListData ) = 0;
	virtual void xferSTLIntList( std::list< Int > *intListData ) = 0;
	virtual void xferScienceType( ScienceType *science ) = 0;
	virtual void xferScienceVec( ScienceVec *scienceVec ) = 0;
	virtual void xferKindOf( KindOfType *kindOfData ) = 0;
	virtual void xferUpgradeMask( UpgradeMaskType *upgradeMaskData ) = 0;
	virtual void xferUser( void *data, Int dataSize ) = 0;
	virtual void xferMatrix3D( Matrix3D* mtx ) = 0;
	virtual void xferMapName( AsciiString *mapNameData ) = 0;

protected:

	// this is the actual xfer impelmentation that each derived class should implement
	virtual void xferImplementation( void *data, Int dataSize ) = 0;

	UnsignedInt m_options;					///< xfer options
	bool m_failed=false;
	XferMode m_xferMode;						///< the current xfer mode
	AsciiString m_identifier;				///< the string identifier
			
};

// Actual original default adapters; typed consumers need only Xfer's contract.
// The transport providers retain all existing default method implementations.
class XferBase : public Xfer
{
public:
    XferBase() = default;
    ~XferBase() override = default;
    void open(AsciiString identifier) override;
    void xferVersion( XferVersion *versionData, XferVersion currentVersion ) override;
    void xferByte( Byte *byteData ) override;
    void xferUnsignedByte( UnsignedByte *unsignedByteData ) override;
    void xferBool( Bool *boolData ) override;
    void xferInt( Int *intData ) override;
    void xferInt64( Int64 *int64Data ) override;
    void xferUnsignedInt( UnsignedInt *unsignedIntData ) override;
    void xferShort( Short *shortData ) override;
    void xferUnsignedShort( UnsignedShort *unsignedShortData ) override;
    void xferReal( Real *realData ) override;
    void xferMarkerLabel( AsciiString asciiStringData ) override;
    void xferAsciiString( AsciiString *asciiStringData ) override;
    void xferUnicodeString( UnicodeString *unicodeStringData ) override;
    void xferCoord3D( Coord3D *coord3D ) override;
    void xferICoord3D( ICoord3D *iCoord3D ) override;
    void xferRegion3D( Region3D *region3D ) override;
    void xferIRegion3D( IRegion3D *iRegion3D ) override;
    void xferCoord2D( Coord2D *coord2D ) override;
    void xferICoord2D( ICoord2D *iCoord2D ) override;
    void xferRegion2D( Region2D *region2D ) override;
    void xferIRegion2D( IRegion2D *iRegion2D ) override;
    void xferRealRange( RealRange *realRange ) override;
    void xferColor( Color *color ) override;
    void xferRGBColor( RGBColor *rgbColor ) override;
    void xferRGBAColorReal( RGBAColorReal *rgbaColorReal ) override;
    void xferRGBAColorInt( RGBAColorInt *rgbaColorInt ) override;
    void xferObjectID( ObjectID *objectID ) override;
    void xferDrawableID( DrawableID *drawableID ) override;
    void xferSTLObjectIDVector( std::vector<ObjectID> *objectIDVectorData ) override;
    void xferSTLObjectIDList( std::list< ObjectID > *objectIDListData ) override;
    void xferSTLIntList( std::list< Int > *intListData ) override;
    void xferScienceType( ScienceType *science ) override;
    void xferScienceVec( ScienceVec *scienceVec ) override;
    void xferKindOf( KindOfType *kindOfData ) override;
    void xferUpgradeMask( UpgradeMaskType *upgradeMaskData ) override;
    void xferUser( void *data, Int dataSize ) override;
    void xferMatrix3D( Matrix3D* mtx ) override;
    void xferMapName( AsciiString *mapNameData ) override;
};

#endif // __XFER_H_
