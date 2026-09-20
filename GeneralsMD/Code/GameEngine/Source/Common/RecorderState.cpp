#include "PreRTS.h"

#include "Common/Recorder.h"

RecorderClass *TheRecorder = NULL;

RecorderModeType RecorderClass::getMode()
{
	return m_mode;
}
