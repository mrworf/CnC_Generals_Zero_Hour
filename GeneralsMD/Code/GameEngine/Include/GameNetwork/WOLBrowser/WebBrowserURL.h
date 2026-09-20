// FILE: WebBrowserURL.h /////////////////////////////////////////////////////
// Portable INI data retained when the obsolete embedded browser is excluded.

#pragma once

#ifndef __WEBBROWSERURL_H__
#define __WEBBROWSERURL_H__

#include "Common/AsciiString.h"
#include "Common/GameMemory.h"

struct FieldParse;

class WebBrowserURL : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( WebBrowserURL, "WebBrowserURL" )

public:
	WebBrowserURL();

	const FieldParse *getFieldParse( void ) const { return m_URLFieldParseTable; }

	AsciiString m_tag;
	AsciiString m_url;
	WebBrowserURL *m_next;

	static const FieldParse m_URLFieldParseTable[];
};

#endif
