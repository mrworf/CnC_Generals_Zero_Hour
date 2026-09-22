#pragma once

#include "Common/GameLOD.h"
#include "GameClient/CommandXlat.h"

// The legacy header relies on MSVC's incomplete-enum extension. Linux includes
// the source definitions before the canonical declaration instead.
enum GraphicsVenderID
{
	DC_NVIDIA_VENDOR_ID = 0x10DE,
	DC_3DFX_VENDOR_ID = 0x121A,
	DC_ATI_VENDOR_ID = 0x1002
};
