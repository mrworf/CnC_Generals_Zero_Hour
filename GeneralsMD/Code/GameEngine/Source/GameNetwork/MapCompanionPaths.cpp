// SPDX-License-Identifier: GPL-3.0-or-later
// Actual original FileTransfer path helpers, shared by setup and LAN owners.
#include "GameNetwork/FileTransfer.h"
#include <cstring>
#include <string_view>

AsciiString GetBasePathFromPath( AsciiString path )
{
	const auto separator=std::string_view(path.str()).find_last_of("/\\");
	const char *s = separator==std::string_view::npos ? nullptr : path.str()+separator;
	if (s)
	{
		Int len = s - path.str();

		AsciiString base;
		char *buf = base.getBufferForRead(len + 1);
		memcpy(buf, path.str(), len);
		buf[len] = 0;
		return buf;
	}
	return AsciiString::TheEmptyString;
}

AsciiString GetFileFromPath( AsciiString path )
{
	const auto separator=std::string_view(path.str()).find_last_of("/\\");
	const char *s = separator==std::string_view::npos ? nullptr : path.str()+separator;
	if (s)
		return s+1;
	return path;
}

AsciiString GetExtensionFromFile( AsciiString fname )
{
	const char *s = fname.reverseFind('.');
	if (s)
		return s+1;
	return fname;
}

AsciiString GetBaseFileFromFile( AsciiString fname )
{
	const char *s = fname.reverseFind('.');
	if (s)
	{
		Int len = s - fname.str();

		AsciiString base;
		char *buf = base.getBufferForRead(len + 1);
		memcpy(buf, fname.str(), len);
		buf[len] = 0;
		return buf;
	}
	return AsciiString::TheEmptyString;
}

AsciiString GetPreviewFromMap( AsciiString path )
{
	AsciiString fname = GetBaseFileFromFile(GetFileFromPath(path));
	AsciiString base = GetBasePathFromPath(path);

	AsciiString out;
	out.format("%s%s%s.tga", base.str(), base.isEmpty()?"":"\\", fname.str());
	return out;
}

AsciiString GetINIFromMap( AsciiString path )
{
	AsciiString base = GetBasePathFromPath(path);

	AsciiString out;
	out.format("%s%smap.ini", base.str(), base.isEmpty()?"":"\\");
	return out;
}

AsciiString GetStrFileFromMap( AsciiString path )
{
	AsciiString base = GetBasePathFromPath(path);

	AsciiString out;
	out.format("%s%smap.str", base.str(), base.isEmpty()?"":"\\");
	return out;
}

AsciiString GetSoloINIFromMap( AsciiString path )
{
	AsciiString base = GetBasePathFromPath(path);

	AsciiString out;
	out.format("%s%ssolo.ini", base.str(), base.isEmpty()?"":"\\");
	return out;
}

AsciiString GetAssetUsageFromMap( AsciiString path )
{
	AsciiString base = GetBasePathFromPath(path);

	AsciiString out;
	out.format("%s%sassetusage.txt", base.str(), base.isEmpty()?"":"\\");
	return out;
}

AsciiString GetReadmeFromMap( AsciiString path )
{
	AsciiString base = GetBasePathFromPath(path);

	AsciiString out;
	out.format("%s%sreadme.txt", base.str(), base.isEmpty()?"":"\\");
	return out;
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
