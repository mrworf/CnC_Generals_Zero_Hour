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

// FILE: AsciiString.cpp 
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information					         
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  AsciiString.cpp
//
// Created:    Steven Johnson, October 2001
//
// Desc:       General-purpose string classes
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/UnicodeString.h"
#include <string>


// -----------------------------------------------------

/*static*/ AsciiString AsciiString::TheEmptyString;

//-----------------------------------------------------------------------------
inline char* skipSeps(char* p, const char* seps)
{
	while (*p && strchr(seps, *p) != NULL)
		++p;
	return p;
}

//-----------------------------------------------------------------------------
inline char* skipNonSeps(char* p, const char* seps)
{
	while (*p && strchr(seps, *p) == NULL)
		++p;
	return p;
}

//-----------------------------------------------------------------------------
inline char* skipWhitespace(char* p)
{
	while (*p && isspace(static_cast<unsigned char>(*p)))
		++p;
	return p;
}

//-----------------------------------------------------------------------------
inline char* skipNonWhitespace(char* p)
{
	while (*p && !isspace(static_cast<unsigned char>(*p)))
		++p;
	return p;
}

void AsciiString::freeBytes(void)
{
	m_data->~AsciiStringData();
	::operator delete(m_data);
}

// -----------------------------------------------------
#ifdef _DEBUG
void AsciiString::validate() const
{
	if (!m_data) return;
	DEBUG_ASSERTCRASH(m_data->m_refCount > 0, ("m_refCount is zero"));
	DEBUG_ASSERTCRASH(m_data->m_refCount < 32000, ("m_refCount is suspiciously large"));
	DEBUG_ASSERTCRASH(m_data->m_numCharsAllocated > 0, ("m_numCharsAllocated is zero"));
//	DEBUG_ASSERTCRASH(m_data->m_numCharsAllocated < 1024, ("m_numCharsAllocated suspiciously large"));
	DEBUG_ASSERTCRASH(strlen(m_data->peek())+1 <= m_data->m_numCharsAllocated,("str is too long (%d) for storage",strlen(m_data->peek())+1));
}
#endif

// -----------------------------------------------------
void AsciiString::debugIgnoreLeaks() {}

// -----------------------------------------------------
void AsciiString::ensureUniqueBufferOfSize(int needed, Bool preserve, const char* copy, const char* append)
{
    if(needed<=0 || needed>MAX_LEN)throw ERROR_OUT_OF_MEMORY;
    // Snapshot borrowed aliases before touching their backing owner.
    std::string value=copy?copy:(preserve && m_data?m_data->peek():"");
    if(append)value+=append;
    if(value.size()>=std::size_t(needed))throw ERROR_BAD_ARG;
    const auto bytes=sizeof(AsciiStringData)+std::size_t(needed);
    auto* fresh=new (::operator new(bytes)) AsciiStringData{};
    fresh->m_refCount.store(1);fresh->m_numCharsAllocated=static_cast<unsigned short>(needed);
#if defined(_DEBUG) || defined(_INTERNAL)
    fresh->m_debugptr=fresh->peek();
#endif
    std::memset(fresh->peek(),0,std::size_t(needed));
    std::memcpy(fresh->peek(),value.data(),value.size());
    releaseBuffer();m_data=fresh;
}

// -----------------------------------------------------
char*  AsciiString::getBufferForRead(Int len)
{
	validate();
	DEBUG_ASSERTCRASH(len>0, ("No need to allocate 0 len strings."));
	if(len<0 || len>=MAX_LEN)throw ERROR_BAD_ARG;
	ensureUniqueBufferOfSize(len + 1, false, NULL, NULL);
	validate();
	return peek();
}

// -----------------------------------------------------
void AsciiString::translate(const UnicodeString& stringSrc)
{
	validate();
	/// @todo srj put in a real translation here; this will only work for 7-bit ascii
	clear();
	Int len = stringSrc.getLength();
	for (Int i = 0; i < len; i++)
		concat((char)stringSrc.getCharAt(i));
	validate();
}

// -----------------------------------------------------
void AsciiString::trim()
{
	validate();

	if (m_data)
	{
		char *c = peek();

		//	Strip leading white space from the string.
		c = skipWhitespace(c);
		if (c != peek())
		{
			set(c);
		}

		if (m_data) // another check, because the previous set() could erase m_data
		{
			//	Clip trailing white space from the string.
			int len = strlen(peek());
			for (int index = len-1; index >= 0; index--)
			{
				if (isspace(getCharAt(index)))
				{
					removeLastChar();
				}
				else
				{
					break;
				}
			}
		}
	}
	validate();
}

// -----------------------------------------------------
void AsciiString::toLower()
{
	validate();
	if (m_data)
	{
		char buf[MAX_FORMAT_BUF_LEN]{};
		strcpy(buf, peek());

		char *c = buf;
		while (c && *c)
		{
			*c = tolower(static_cast<unsigned char>(*c));
			c++;
		}
		set(buf);
	}
	validate();
}

// -----------------------------------------------------
void AsciiString::removeLastChar()
{
	validate();
	if (m_data)
	{
		int len = strlen(peek());
		if (len > 0)
		{
			ensureUniqueBufferOfSize(len+1, true, NULL, NULL);
			peek()[len - 1] = 0;
		}
	}
	validate();
}

// -----------------------------------------------------
// -----------------------------------------------------
void AsciiString::format(const char* format, ...)
{
	validate();
	va_list args;
  va_start(args, format);
	try {format_va(format, args);} catch(...) {va_end(args);throw;}
  va_end(args);
	validate();
}

// -----------------------------------------------------
void AsciiString::format_va(const AsciiString& format, va_list args)
{
	validate();
	char buf[MAX_FORMAT_BUF_LEN]{};
  const int count=vsnprintf(buf, sizeof(buf), format.str(), args);
  if (count<0 || count>=int(sizeof(buf)))
			throw ERROR_OUT_OF_MEMORY;
	set(buf);
	validate();
}

// -----------------------------------------------------
void AsciiString::format_va(const char* format, va_list args)
{
	validate();
	char buf[MAX_FORMAT_BUF_LEN]{};
  if(!format)throw ERROR_BAD_ARG;
  const int count=vsnprintf(buf, sizeof(buf), format, args);
  if (count<0 || count>=int(sizeof(buf)))
			throw ERROR_OUT_OF_MEMORY;
	set(buf);
	validate();
}

// -----------------------------------------------------
Bool AsciiString::startsWith(const char* p) const
{
	if (*p == 0)
		return true;	// everything starts with the empty string

	int lenThis = getLength();
	int lenThat = strlen(p);
	if (lenThis < lenThat)
		return false;	// that must be smaller than this

	return strncmp(peek(), p, lenThat) == 0;
}

// -----------------------------------------------------
Bool AsciiString::startsWithNoCase(const char* p) const
{
	if (*p == 0)
		return true;	// everything starts with the empty string

	int lenThis = getLength();
	int lenThat = strlen(p);
	if (lenThis < lenThat)
		return false;	// that must be smaller than this

	return strncasecmp(peek(), p, lenThat) == 0;
}

// -----------------------------------------------------
Bool AsciiString::endsWith(const char* p) const
{
	if (*p == 0)
		return true;	// everything ends with the empty string

	int lenThis = getLength();
	int lenThat = strlen(p);
	if (lenThis < lenThat)
		return false;	// that must be smaller than this

	return strncmp(peek() + lenThis - lenThat, p, lenThat) == 0;
}

// -----------------------------------------------------
Bool AsciiString::endsWithNoCase(const char* p) const
{
	if (*p == 0)
		return true;	// everything ends with the empty string

	int lenThis = getLength();
	int lenThat = strlen(p);
	if (lenThis < lenThat)
		return false;	// that must be smaller than this

	return strncasecmp(peek() + lenThis - lenThat, p, lenThat) == 0;
}

//-----------------------------------------------------------------------------
Bool AsciiString::isNone() const
{
	return m_data && strcasecmp(peek(), "None") == 0;
}

//-----------------------------------------------------------------------------
Bool AsciiString::nextToken(AsciiString* tok, const char* seps)
{
    if(!tok)throw ERROR_BAD_ARG;
	if (this->isEmpty() || tok == this)
		return false;

	if (seps == NULL)
		seps = " \n\r\t";

	char* start = skipSeps(peek(), seps);
	char* end = skipNonSeps(start, seps);

	if (end > start)
	{
		const std::string tokenValue(start,end);
		AsciiString candidateToken(tokenValue.c_str()),candidateRest(end);
		*tok=candidateToken;
		*this=candidateRest;

		return true;
	}
	else
	{
		this->clear();
		tok->clear();
		return false;
	}
}
