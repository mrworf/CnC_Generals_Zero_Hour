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

// FILE: W3DModuleFactory.cpp /////////////////////////////////////////////////////////////////////
// Author: Colin Day, April 2001
// Desc:	 W3D specific module
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"
#include "W3DDevice/Common/W3DModuleFactory.h"
#include "W3DDevice/GameClient/Module/W3DDependencyModelDraw.h"
#include "W3DDevice/GameClient/Module/W3DDefaultDraw.h"
#include "W3DDevice/GameClient/Module/W3DModelDraw.h"
#include "W3DDevice/GameClient/Module/W3DLaserDraw.h"
#include "W3DDevice/GameClient/Module/W3DOverlordTankDraw.h"
#include "W3DDevice/GameClient/Module/W3DOverlordTruckDraw.h"
#include "W3DDevice/GameClient/Module/W3DOverlordAircraftDraw.h"
#include "W3DDevice/GameClient/Module/W3DProjectileStreamDraw.h"
#include "W3DDevice/GameClient/Module/W3DSupplyDraw.h"
#include "W3DDevice/GameClient/Module/W3DScienceModelDraw.h"
#include "W3DDevice/GameClient/Module/W3DTankDraw.h"
#include "W3DDevice/GameClient/Module/W3DTruckDraw.h"
#include "W3DDevice/GameClient/Module/W3DTankTruckDraw.h"
#include "W3DDevice/GameClient/Module/W3DTreeDraw.h"
#include "W3DDevice/GameClient/Module/W3DPropDraw.h"

namespace {
template <typename T> ModuleData *newW3DModuleData(INI *ini)
{
	T *data = MSGNEW("AllModuleData") T;
	if (ini != NULL)
		ini->initFromINIMultiProc(data, T::buildFieldParse);
	return data;
}

}

//-------------------------------------------------------------------------------------------------
/** Initialize method */
//-------------------------------------------------------------------------------------------------
void W3DModuleFactory::init( void )
{

	// extending functionality
	ModuleFactory::init();

	// Keep the original complete registration set while separating the
	// configuration schema from physical draw-instance construction. A null
	// instance proc is an explicit unavailable device edge; ModuleFactory
	// rejects attempts to cross it instead of returning placeholder success.
#define addW3DSchema(name, dataType) \
	addModuleInternal(NULL, newW3DModuleData<dataType>, MODULETYPE_DRAW, AsciiString(#name), MODULEINTERFACE_DRAW)
#if defined(ZH_W3D_HEADLESS_INSTANCE) || defined(ZH_W3D_FULL_INSTANCE)
	addModule(W3DDefaultDraw);
#else
	addW3DSchema(W3DDefaultDraw, ModuleData);
#endif
	addW3DSchema(W3DDebrisDraw, ModuleData);
#if defined(ZH_W3D_HEADLESS_INSTANCE) || defined(ZH_W3D_FULL_INSTANCE)
	addModule(W3DModelDraw);
#else
	addW3DSchema(W3DModelDraw, W3DModelDrawModuleData);
#endif
	addW3DSchema(W3DLaserDraw, W3DLaserDrawModuleData);
#if defined(ZH_W3D_HEADLESS_INSTANCE) || defined(ZH_W3D_FULL_INSTANCE)
	addModule(W3DOverlordTankDraw);
#else
	addW3DSchema(W3DOverlordTankDraw, W3DOverlordTankDrawModuleData);
#endif
#if defined(ZH_W3D_HEADLESS_INSTANCE) || defined(ZH_W3D_FULL_INSTANCE)
	addModule(W3DOverlordTruckDraw);
#else
	addW3DSchema(W3DOverlordTruckDraw, W3DOverlordTruckDrawModuleData);
#endif
#if defined(ZH_W3D_HEADLESS_INSTANCE) || defined(ZH_W3D_FULL_INSTANCE)
	addModule(W3DOverlordAircraftDraw);
#else
	addW3DSchema(W3DOverlordAircraftDraw, W3DOverlordAircraftDrawModuleData);
#endif
	addW3DSchema(W3DProjectileStreamDraw, W3DProjectileStreamDrawModuleData);
	addW3DSchema(W3DPoliceCarDraw, W3DTruckDrawModuleData);
	addW3DSchema(W3DRopeDraw, ModuleData);
	addW3DSchema(W3DScienceModelDraw, W3DScienceModelDrawModuleData);
#if defined(ZH_W3D_HEADLESS_INSTANCE) || defined(ZH_W3D_FULL_INSTANCE)
	addModule(W3DSupplyDraw);
#else
	addW3DSchema(W3DSupplyDraw, W3DSupplyDrawModuleData);
#endif
#if defined(ZH_W3D_HEADLESS_INSTANCE) || defined(ZH_W3D_FULL_INSTANCE)
	addModule(W3DDependencyModelDraw);
#else
	addW3DSchema(W3DDependencyModelDraw, W3DDependencyModelDrawModuleData);
#endif
#if defined(ZH_W3D_HEADLESS_INSTANCE) || defined(ZH_W3D_FULL_INSTANCE)
	addModule(W3DTankDraw);
#else
	addW3DSchema(W3DTankDraw, W3DTankDrawModuleData);
#endif
#if defined(ZH_W3D_HEADLESS_INSTANCE) || defined(ZH_W3D_FULL_INSTANCE)
	addModule(W3DTruckDraw);
#else
	addW3DSchema(W3DTruckDraw, W3DTruckDrawModuleData);
#endif
	addW3DSchema(W3DTracerDraw, ModuleData);
#if defined(ZH_W3D_HEADLESS_INSTANCE) || defined(ZH_W3D_FULL_INSTANCE)
	addModule(W3DTankTruckDraw);
#else
	addW3DSchema(W3DTankTruckDraw, W3DTankTruckDrawModuleData);
#endif
	addW3DSchema(W3DTreeDraw, W3DTreeDrawModuleData);
	addW3DSchema(W3DPropDraw, W3DPropDrawModuleData);
#undef addW3DSchema

}  // end init
