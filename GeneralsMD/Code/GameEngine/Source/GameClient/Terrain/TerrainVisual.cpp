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

// FILE: TerrainVisual.cpp ////////////////////////////////////////////////////////////////////////
// Interface for visual representation of terrain on the client
// Author: Colin Day, April 2001
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Xfer.h"
#include "GameClient/TerrainVisual.h"
#include <limits>
#include <vector>


#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif



// GLOBALS ////////////////////////////////////////////////////////////////////////////////////////
TerrainVisual *TheTerrainVisual = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINITIONS
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
TerrainVisual::TerrainVisual()
{

}  // end TerrainVisual

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
TerrainVisual::~TerrainVisual()
{

}  // end ~TerrainVisual

//-------------------------------------------------------------------------------------------------
/** initialize the device independent functionality of the visual terrain */
//-------------------------------------------------------------------------------------------------
void TerrainVisual::init( void )
{

}  // end init

//-------------------------------------------------------------------------------------------------
/** Reset */ 
//-------------------------------------------------------------------------------------------------
void TerrainVisual::reset( void )
{

	m_filenameString.clear();

}  // end reset

//-------------------------------------------------------------------------------------------------
/** Update */
//-------------------------------------------------------------------------------------------------
void TerrainVisual::update( void )
{
	// All the interesting stuff happens in load.  jba.
}  // end update

//-------------------------------------------------------------------------------------------------
/** device independent implementation for common terrain visual systems */
//-------------------------------------------------------------------------------------------------
Bool TerrainVisual::load( AsciiString filename )
{

	// save the filename
	if (filename.isEmpty())
		return FALSE;

	m_filenameString = filename;

	return TRUE;;  // success

}  // end load

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void TerrainVisual::crc( Xfer *xfer )
{

}  // end CRC

// ------------------------------------------------------------------------------------------------
/** Xfer
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void TerrainVisual::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void TerrainVisual::loadPostProcess( void )
{

}  // end loadPostProcess







SeismicSimulationFilterBase::SeismicSimStatusCode DomeStyleSeismicFilter::filterCallback( WorldHeightMapInterfaceClass *heightMap, const SeismicSimulationNode *node )
{

  if ( heightMap == NULL || node == NULL )
    return SEISMIC_STATUS_INVALID;
  const UnsignedInt life = node->m_life;

  if ( life == 0 )
    return SEISMIC_STATUS_ACTIVE;
  if ( life < 15 )
  {
    // ADD HEIGHT BECAUSE THE EXPLOSION IS PUSHING DIRT UP

    Real magnitude = node->m_magnitude;

    Real offsScalar =  magnitude / (Real)life; // real-life, get it?
    if (!std::isfinite(magnitude) || node->m_radius>UnsignedInt(std::numeric_limits<Int>::max()/2))
      return SEISMIC_STATUS_INVALID;
    const Int radius = Int(node->m_radius);
    Int border = heightMap->getBorderSize();
    const auto wideX=std::int64_t(node->m_center.x)+border;
    const auto wideY=std::int64_t(node->m_center.y)+border;
    for (const auto center:{wideX,wideY})
      if (center-radius<std::numeric_limits<Int>::min() ||
          center+radius>std::numeric_limits<Int>::max()) return SEISMIC_STATUS_INVALID;
    const Int centerX=Int(wideX),centerY=Int(wideY);
    const Int workspaceWidth=radius*2;
    const auto width=std::size_t(workspaceWidth);
    if (width && width>std::vector<Real>().max_size()/width) return SEISMIC_STATUS_INVALID;
    std::vector<Real> workspace(width*width,0.0f);

    for (Int x = 0; x < radius; ++x)
    {
      for (Int y = 0; y < radius; ++y)
      {

        Real distance = sqrt( Real(x)*Real(x) + Real(y)*Real(y) );//Pythagoras
    
        if ( distance < radius )
        {
          Real distScalar = cos( ( distance / radius * (PI/2) ) );
          Real height = (offsScalar * distScalar); 

          workspace[ (radius + x) +  width * (radius + y) ] = height + heightMap->getBilinearSampleSeismicZVelocity( centerX + x,  centerY + y ) ;//kaleidoscope
          
          if ( x != 0 ) // non-zero test prevents cross-shaped double stamp 
          {
            workspace[ (radius - x) + width * (radius + y) ] = height + heightMap->getBilinearSampleSeismicZVelocity( centerX - x,  centerY + y ) ;
            if ( y != 0 )
              workspace[ (radius - x) + width * (radius - y) ] =  height + heightMap->getBilinearSampleSeismicZVelocity( centerX - x,  centerY - y ) ;
          }
          if ( y != 0 )
            workspace[ (radius + x) + width * (radius - y) ] = height + heightMap->getBilinearSampleSeismicZVelocity( centerX + x,  centerY - y ) ;
        }
      }
    }

    // stuff the values from the workspace into the heightmap's velocities
    for (Int x = 0; x < workspaceWidth; ++x)
      for (Int y = 0; y < workspaceWidth; ++y)
        heightMap->setSeismicZVelocity( centerX - radius + x, centerY - radius + y, std::min(9.0f, workspace[x + width * y]) );

    return SEISMIC_STATUS_ACTIVE;
  }
  else
    return SEISMIC_STATUS_ZERO_ENERGY;
}

Real DomeStyleSeismicFilter::applyGravityCallback( Real velocityIn )
{
  Real velocityOut = velocityIn;
  velocityOut -= 1.5f;
  return velocityOut;
}
