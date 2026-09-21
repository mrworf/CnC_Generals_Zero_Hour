#include "ww3d.h"
#include "ffactory.h"
#include "animatedsoundmgr.h"

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
