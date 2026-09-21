#include "ww3d.h"
#include "ffactory.h"
#include "animatedsoundmgr.h"

// CPU-side definitions copied from ww3d.cpp. The physical device state is
// supplied by the M22 translator rather than linking the retired D3D runtime.
unsigned int WW3D::SyncTime = 0;
unsigned int WW3D::PreviousSyncTime = 0;
bool WW3D::IsSortingEnabled = true;
float WW3D::PixelCenterX = 0.0f;
float WW3D::PixelCenterY = 0.0f;
bool WW3D::IsInitted = false;
bool WW3D::IsRendering = false;
bool WW3D::IsCapturing = false;
bool WW3D::IsScreenUVBiased = false;
bool WW3D::AreDecalsEnabled = true;
float WW3D::DecalRejectionDistance = 1000000.0f;
bool WW3D::AreStaticSortListsEnabled = false;
bool WW3D::MungeSortOnLoad = false;
bool WW3D::OverbrightModifyOnLoad = false;
WW3D::PrelitModeEnum WW3D::PrelitMode = WW3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS;
float WW3D::DefaultNativeScreenSize = 1.0f;
WW3D::NPatchesGapFillingModeEnum WW3D::NPatchesGapFillingMode = WW3D::NPATCHES_GAP_FILLING_ENABLED;
unsigned WW3D::NPatchesLevel = 1;

FileFactoryClass *_TheFileFactory = nullptr;
RawFileFactoryClass *_TheWritingFileFactory = nullptr;
SimpleFileFactoryClass *_TheSimpleFileFactory = nullptr;

// The original manager returns these exact values before its optional sound
// library/configuration is installed. M22 does not initialize that audio edge.
const char *AnimatedSoundMgrClass::Get_Embedded_Sound_Name(HAnimClass *)
{
	return nullptr;
}

float AnimatedSoundMgrClass::Trigger_Sound(HAnimClass *, float old_frame, float, const Matrix3D &)
{
	return old_frame;
}
