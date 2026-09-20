#pragma once

#include "Common/LocalFile.h"

class PosixLocalFile : public LocalFile
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(PosixLocalFile, "PosixLocalFile")

public:
	PosixLocalFile();
};
