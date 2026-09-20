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

// FILE: ThreadUtils.cpp //////////////////////////////////////////////////////
// GameSpy thread utils
// Author: Matthew D. Campbell, July 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#ifndef _WIN32
#include <codecvt>
#include <locale>
#endif

//-------------------------------------------------------------------------

std::wstring MultiByteToWideCharSingleLine( const char *orig )
{
#ifndef _WIN32
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::wstring ret = converter.from_bytes(orig ? orig : "");
	for (wchar_t &value : ret)
		if (value == L'\n' || value == L'\r') value = L' ';
	return ret;
#else
	Int len = strlen(orig);
	WideChar *dest = NEW WideChar[len+1];

	MultiByteToWideChar(CP_UTF8, 0, orig, -1, dest, len);
	WideChar *c = NULL;
	do
	{
		c = wcschr(dest, L'\n');
		if (c)
		{
			*c = L' ';
		}
	}
	while ( c != NULL );
	do
	{
		c = wcschr(dest, L'\r');
		if (c)
		{
			*c = L' ';
		}
	}
	while ( c != NULL );

	dest[len] = 0;
	std::wstring ret = dest;
	delete dest;
	return ret;
#endif
}

std::string WideCharStringToMultiByte( const WideChar *orig )
{
#ifndef _WIN32
	if (!orig) return std::string();
	std::u16string value(orig);
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	return converter.to_bytes(value);
#else
	std::string ret;
	Int len = WideCharToMultiByte( CP_UTF8, 0, orig, wcslen(orig), NULL, 0, NULL, NULL ) + 1;
	if (len > 0)
	{
		char *dest = NEW char[len];
		WideCharToMultiByte( CP_UTF8, 0, orig, -1, dest, len, NULL, NULL );
		dest[len-1] = 0;
		ret = dest;
		delete dest;
	}
	return ret;
#endif
}

//-------------------------------------------------------------------------
