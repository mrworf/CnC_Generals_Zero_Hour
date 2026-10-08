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

// FILE: INI.cpp //////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:   INI Reader
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////

// SPDX-License-Identifier: GPL-3.0-or-later
// Original INI token/field dispatch split from the complete gameplay block table.
#include "Common/INI.h"
#include "Common/FileSystem.h"
#include "Common/file.h"
#include "Common/FileOwner.h"
#include "Common/INIException.h"
#include "GameLogic/FPUControl.h"
#include <algorithm>
#include <charconv>
#include <cctype>
#include <cmath>
#include <cstring>
#include <strings.h>
#include <string>
#include <utility>
namespace {
const char* numericStart(const char* token) {
    if(!token)throw ERROR_BAD_INI;
    while(std::isspace(static_cast<unsigned char>(*token)))++token;
    if(!*token)throw ERROR_BAD_INI;
    return token;
}
template<std::size_t N> void copyBounded(char(&out)[N],const char* text) {
    if(!text || std::strlen(text)>=N)throw ERROR_BAD_INI;
    std::strcpy(out,text);
}
template<std::size_t N> void appendBounded(char(&out)[N],const char* text) {
    if(!text || std::strlen(text)>=N-std::strlen(out))throw ERROR_BAD_INI;
    std::strcat(out,text);
}
}
INI::INI( void )
{

	m_file							= NULL;
  m_readBufferNext=m_readBufferUsed=0;
	m_filename					= "None";
	m_loadType					= INI_LOAD_INVALID;
	m_lineNum						= 0;
	m_seps							= " \n\r\t=";			///< make sure you update m_sepsPercent/m_sepsColon as well
	m_sepsPercent				= " \n\r\t=%%";
	m_sepsColon					= " \n\r\t=:";
	m_sepsQuote					= "\"\n=";				///< stop at " = EOL
	m_blockEndToken			= "END";
	m_endOfFile					= FALSE;
	std::fill(std::begin(m_buffer),std::end(m_buffer),char(0));
    std::fill(std::begin(m_readBuffer),std::end(m_readBuffer),char(0));
#if defined(_DEBUG) || defined(_INTERNAL)
	m_curBlockStart[0]	= 0;
#endif

}

INI::~INI(){unPrepFile();}

void INI::prepFile(AsciiString filename,INILoadType type) {
    if(m_file || !TheFileSystem || type<INI_LOAD_OVERWRITE || type>INI_LOAD_MULTIFILE)throw ERROR_BAD_INI;
    File* opened=TheFileSystem->openFile(filename.str(),File::READ);
    if(!opened)throw ERROR_BAD_INI;
    File* converted=nullptr;
    try {converted=opened->convertToRAMFile();}
    catch(...){opened->close();throw;}
    FileCloseOwner candidate(converted);
    if(!candidate)throw ERROR_BAD_INI;
    candidate->deleteOnClose();
    m_filename=filename;m_loadType=type;m_lineNum=0;m_endOfFile=FALSE;
    m_readBufferNext=0;m_readBufferUsed=0;m_tokenCursor=nullptr;
    std::fill(std::begin(m_buffer),std::end(m_buffer),char(0));
    std::fill(std::begin(m_readBuffer),std::end(m_readBuffer),char(0));
    m_file=candidate.release();
}

void INI::unPrepFile() {
    File* retired=std::exchange(m_file,nullptr);
    m_readBufferUsed=0;m_readBufferNext=0;m_tokenCursor=nullptr;
    m_filename.clear();m_loadType=INI_LOAD_INVALID;m_lineNum=0;m_endOfFile=FALSE;
    m_lineTransfer=nullptr;m_transferOwner=nullptr;
    if(retired)retired->close();
}

static INIFieldParseProc findFieldParse(const FieldParse* parseTable, const char* token, int& offset, const void*& userData)
{
	if(!parseTable || !token)throw ERROR_BAD_INI;
	const FieldParse* parse=parseTable;
	for (; parse->token; ++parse)
	{
		if (strcmp( parse->token, token ) == 0)
		{
			offset = parse->offset;
			userData = parse->userData;
			return parse->parse;
		}
	}

	if (!parse->token && parse->parse)
	{
		offset = parse->offset;
		userData = token;
		return parse->parse;
	}
	else
	{
		return NULL;
	}
}

void INI::readLine() {
    if(!m_file)throw ERROR_BAD_INI;
    m_tokenCursor=nullptr;std::size_t length=0;bool comment=false;
    m_buffer[0]=0;
    if(m_endOfFile)return;
    while(true) {
        if(m_readBufferNext==m_readBufferUsed) {
            const Int count=m_file->read(m_readBuffer,INI_READ_BUFFER);
            if(count<0)throw ERROR_BAD_INI;
            m_readBufferNext=0;m_readBufferUsed=UnsignedInt(count);
            if(count==0){m_endOfFile=TRUE;break;}
        }
        unsigned char value=static_cast<unsigned char>(m_readBuffer[m_readBufferNext++]);
        if(value=='\n')break;
        if(length==INI_MAX_CHARS_PER_LINE)throw ERROR_BAD_INI;
        if(value==0)throw ERROR_BAD_INI;
        if(value==';')comment=true;
        m_buffer[length++]=comment?char(0):(value<32?' ':char(value));
    }
    m_buffer[length]=0;
    if(m_lineNum==UINT32_MAX)throw ERROR_BAD_INI;
    ++m_lineNum;
    if(m_lineTransfer)m_lineTransfer(m_transferOwner,m_buffer,Int(std::strlen(m_buffer)));
}

void INI::parseUnsignedByte( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();
	Int value = scanInt(token);
	if (value < 0 || value > 255)
	{
		DEBUG_CRASH(("Bad value INI::parseUnsignedByte"));
		throw ERROR_BUG;
	}
	*(Byte *)store = (Byte)value;
}

void INI::parseShort( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();
	Int value = scanInt(token);
	if (value < -32768 || value > 32767)
	{
		DEBUG_CRASH(("Bad value INI::parseShort"));
		throw ERROR_BUG;
	}
	*(Short *)store = (Short)value;
}

void INI::parseUnsignedShort( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();
	Int value = scanInt(token);
	if (value < 0 || value > 65535)
	{
		DEBUG_CRASH(("Bad value INI::parseUnsignedShort"));
		throw ERROR_BUG;
	}
	*(UnsignedShort *)store = (UnsignedShort)value;
}

void INI::parseInt( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();
	*(Int *)store = scanInt(token);

}

void INI::parseUnsignedInt( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();
	*(UnsignedInt *)store = scanUnsignedInt(token);

}

void INI::parseReal( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();
	*(Real *)store = scanReal(token);

}

void INI::parsePositiveNonZeroReal( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();
	const Real candidate=scanReal(token);
	if (candidate <= 0.0f)
	{
		DEBUG_CRASH(("invalid Real value %f -- expected > 0\n",*(Real*)store));
		throw INI_INVALID_DATA;
	}
	*(Real*)store=candidate;

}

void INI::parseBool( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	*(Bool*)store = INI::scanBool(ini->getNextToken());
}

void INI::parseBitInInt32( INI *ini, void *instance, void *store, const void* userData )
{
	UnsignedInt* s = (UnsignedInt*)store;
	(void)instance;
    const auto raw=reinterpret_cast<std::uintptr_t>(userData);
    if(raw>UINT32_MAX)throw ERROR_BAD_INI;
    UnsignedInt mask=UnsignedInt(raw);

	if (INI::scanBool(ini->getNextToken()))
		*s |= mask;
	else
		*s &= ~mask;
}

/*static*/ Bool INI::scanBool(const char* token)
{
	if(!token)throw ERROR_BAD_INI;
	// translate string yes/no into TRUE/FALSE
	if( strcasecmp( token, "yes" ) == 0 )
		return TRUE;
	else if( strcasecmp( token, "no" ) == 0 )
		return FALSE;
	else
	{
		DEBUG_CRASH(("invalid boolean token %s -- expected Yes or No\n",token));
		throw INI_INVALID_DATA;
		return false;	// keep compiler happy
	}

}

void INI::parseAsciiString( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	AsciiString* asciiString = (AsciiString *)store;
	*asciiString = ini->getNextAsciiString();
}

void INI::parseQuotedAsciiString( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	AsciiString* asciiString = (AsciiString *)store;
	*asciiString = ini->getNextQuotedAsciiString();
}

AsciiString INI::getNextQuotedAsciiString()
{
	AsciiString result;
	char buff[INI_MAX_CHARS_PER_LINE]{};

	const char *token = getNextTokenOrNull();	// if null, just leave an empty string
	if (token != NULL)
	{
		if (token[0] != '\"')
		{
			// if token is simply "
			result.set( token );	// Start following the "
		}
		else
		{	int strLen=0;
			Bool done=FALSE;
			if ((strLen=strlen(token)) > 1)
			{
				copyBounded(buff, &token[1]);	//skip the starting quote
				//Check for end of quoted string.  Checking here fixes cases where quoted string on same line with other data.
				if (buff[strLen-2]=='"')	//skip ending quote if present
				{	buff[strLen-2]='\0';
					done=TRUE;
				}
			}

			if (!done)
			{
				token = getNextToken(getSepsQuote());

				if (strlen(token) > 1 && token[1] != '\t')
				{
					appendBounded(buff, " ");
					appendBounded(buff, token);
				}
				else
				{	Int buflen=strlen(buff);
					if (buflen>0 && buff[buflen-1]=='\"')
						buff[buflen-1]='\0';
				}
			}
			result.set(buff);
		}
	}
	return result;
}

AsciiString INI::getNextAsciiString()
{
	AsciiString result;

	const char *token = getNextTokenOrNull();	// if null, just leave an empty string
	if (token != NULL)
	{
		if (token[0] != '\"')
		{
			// if token is simply "
			result.set( token );	// Start following the "
		}
		else
		{
			char buff[INI_MAX_CHARS_PER_LINE]{};
			buff[0] = 0;
			if (strlen(token) > 1)
			{
				copyBounded(buff, &token[1]);
			}

			token = getNextTokenOrNull(getSepsQuote());
			if (token) {
				if (strlen(token) > 1 && token[1] != '\t')
				{
					appendBounded(buff, " ");
				}
				appendBounded(buff, token);
				result.set(buff);
			} else {
				Int len = strlen(buff);
				if (len && buff[len-1] == '"') { // strip off trailing quote jba. [2/12/2003]
					buff[len-1] = 0;
				}
				result.set(buff);
			}
		}
	}
	return result;
}

void MultiIniFieldParse::add(const FieldParse* f, UnsignedInt e)
{
	if (m_count < MAX_MULTI_FIELDS)
	{
		m_fieldParse[m_count] = f;
		m_extraOffset[m_count] = e;
		++m_count;
	}
	else
	{
		DEBUG_CRASH(("too many multi-fields in INI::initFromINIMultiProc"));
		throw ERROR_BUG;
	}
}

void INI::initFromINI( void *what, const FieldParse* parseTable )
{
	MultiIniFieldParse p;
	p.add(parseTable);
	initFromINIMulti(what, p);
}

void INI::initFromINIMultiProc( void *what, BuildMultiIniFieldProc proc )
{
	MultiIniFieldParse p;
	(*proc)(p);
	initFromINIMulti(what, p);
}

void INI::initFromINIMulti( void *what, const MultiIniFieldParse& parseTableList )
{
	Bool done = FALSE;

	if( what == NULL )
	{
		DEBUG_ASSERTCRASH( 0, ("INI::initFromINI - Invalid parameters supplied!\n") );
		throw INI_INVALID_PARAMS;
	}

	// read each of the data fields
	while( !done )
	{

		// read next line
		readLine();

		// check for end token
		const char* field = ::strtok_r(m_buffer,getSeps(),&m_tokenCursor);
		if( field )
		{

			if( strcasecmp( field, m_blockEndToken ) == 0 )
			{
				done = TRUE;
			}
			else
			{
				Bool found = false;
				for (int ptIdx = 0; ptIdx < parseTableList.getCount(); ++ptIdx)
				{
					int offset = 0;
					const void* userData = nullptr;
					INIFieldParseProc parse = findFieldParse(parseTableList.getNthFieldParse(ptIdx), field, offset, userData);
					if (parse)
					{
						// parse this block and check for parse errors
						try {

						(*parse)( this, what, (char *)what + offset + parseTableList.getNthExtraOffset(ptIdx), userData );

						} catch (const std::bad_alloc&) {
                            throw; // preserve OOM without a second diagnostic allocation
						} catch (...) {
							DEBUG_CRASH( ("[LINE: %d - FILE: '%s'] Error reading field '%s' of block '%s'\n",
																 INI::getLineNum(), INI::getFilename().str(), field, m_curBlockStart) );


							throw INIException("Original INI field rejected");
						}

						found = true;
						break;

					}
				}

				if (!found)
				{
					DEBUG_ASSERTCRASH( 0, ("[LINE: %d - FILE: '%s'] Unknown field '%s' in block '%s'\n",
														 INI::getLineNum(), INI::getFilename().str(), field, m_curBlockStart) );
					throw INI_UNKNOWN_TOKEN;
				}

			}  // end else

		}  // end if

		// sanity check for reaching end of file with no closing end token
		if( done == FALSE && INI::isEOF() == TRUE )
		{

			done = TRUE;
			DEBUG_ASSERTCRASH( 0, ("Error parsing block '%s', in INI file '%s'.  Missing '%s' token\n",
												 m_curBlockStart, getFilename().str(), m_blockEndToken) );
			throw INI_MISSING_END_TOKEN;

		}  // end if

	}  // end while

}

/*static*/ const char* INI::getNextToken(const char* seps)
{
	if (!seps) seps = getSeps();
	const char *token = ::strtok_r(nullptr,seps,&m_tokenCursor);
	if (!token)
		throw INI_INVALID_DATA;
	return token;
}

/*static*/ const char* INI::getNextTokenOrNull(const char* seps)
{
	if (!seps) seps = getSeps();
	const char *token = ::strtok_r(nullptr,seps,&m_tokenCursor);
	return token;
}

Int INI::scanInt(const char* input) {
    const char* token=numericStart(input);if(*token=='+')++token;
    Int value=0;const auto result=std::from_chars(token,token+std::strlen(token),value);
    if(result.ec!=std::errc{} || result.ptr==token)throw ERROR_BAD_INI;
    return value; // Original sscanf admits a numeric prefix, not just whole tokens.
}

UnsignedInt INI::scanUnsignedInt(const char* input) {
    const char* token=numericStart(input);bool negative=*token=='-';
    if(negative || *token=='+')++token;
    UnsignedInt value=0;const auto result=std::from_chars(token,token+std::strlen(token),value);
    if(result.ec!=std::errc{} || result.ptr==token)throw ERROR_BAD_INI;
    return negative?UnsignedInt(0)-value:value;
}

Real INI::scanReal(const char* input) {
    const char* token=numericStart(input);if(*token=='+')++token;
    Real value=0;const auto result=std::from_chars(token,token+std::strlen(token),value);
    if(result.ec!=std::errc{} || result.ptr==token || !std::isfinite(value))throw ERROR_BAD_INI;
    return value;
}

Real INI::scanPercentToReal(const char* token){return scanReal(token)/100.f;}

/*static*/ Int INI::scanIndexList(const char* token, ConstCharPtrArray nameList)
{
	if(!token)throw ERROR_BAD_INI;
	if( nameList == NULL || nameList[ 0 ] == NULL )
	{

		DEBUG_ASSERTCRASH( 0, ("INTERNAL ERROR! scanIndexList, invalid name list\n") );
		throw INI_INVALID_NAME_LIST;

	}

	// search for matching name
	Int count = 0;
	for(ConstCharPtrArray name = nameList; *name; name++, count++ )
	{
		if( strcasecmp( *name, token ) == 0 )
		{
			return count;
		}
	}

	DEBUG_CRASH(("token %s is not a valid member of the index list\n",token));
	throw INI_INVALID_DATA;
	return 0;	// never executed, but keeps compiler happy

}

/*static*/ Int INI::scanLookupList(const char* token, ConstLookupListRecArray lookupList)
{
	if(!token)throw ERROR_BAD_INI;
	if( lookupList == NULL || lookupList[ 0 ].name == NULL )
	{
		DEBUG_ASSERTCRASH( 0, ("INTERNAL ERROR! scanLookupList, invalid name list\n") );
		throw INI_INVALID_NAME_LIST;
	}

	// search for matching name
	for( const LookupListRec* lookup = &lookupList[0]; lookup->name; lookup++ )
	{
		if( strcasecmp( lookup->name, token ) == 0 )
		{
			return lookup->value;
		}
	}

	DEBUG_CRASH(("token %s is not a valid member of the lookup list\n",token));
	throw INI_INVALID_DATA;
	return 0;	// never executed, but keeps compiler happy

}

const char* INI::getNextSubToken(const char* expected)
{
	if(!expected)throw ERROR_BAD_INI;
	const char* token = getNextToken(getSepsColon());
	if (strcasecmp(token, expected) != 0)
		throw INI_INVALID_DATA;
	return getNextToken(getSepsColon());
}

void INI::loadFields(AsciiString filename,INILoadType type,void* candidate,const FieldParse* fields) {
    if(!candidate || !fields)throw ERROR_BAD_INI;
    prepFile(filename,type);
    try {initFromINI(candidate,fields);}catch(...){unPrepFile();throw;}
    unPrepFile();
}

namespace {
void validateSelectedBlocks(std::span<const INIBlockDefinition> blocks, INILineTransfer transfer) {
    if (blocks.empty() || bool(transfer.owner)!=bool(transfer.line)) throw ERROR_BAD_INI;
    for (std::size_t i=0; i<blocks.size(); ++i) {
        if (!blocks[i].token || !*blocks[i].token || !blocks[i].parse) throw ERROR_BAD_INI;
        for (std::size_t j=0; j<i; ++j)
            if (std::strcmp(blocks[j].token, blocks[i].token)==0) throw ERROR_BAD_INI;
    }
}
}
void INI::loadBlocks(AsciiString filename, INILoadType type,
                     std::span<const INIBlockDefinition> blocks, INILineTransfer transfer) {
    validateSelectedBlocks(blocks, transfer);
    setFPMode();
    prepFile(filename,type);
    m_transferOwner=transfer.owner;
    m_lineTransfer=transfer.line;
    try {
        while (!m_endOfFile) {
            readLine();
            const char* token=::strtok_r(m_buffer,m_seps,&m_tokenCursor);
            if (!token) continue;
            INIBlockParse parser=nullptr;
            for (const auto& block: blocks)
                if (std::strcmp(block.token,token)==0) {parser=block.parse;break;}
            if (!parser) throw INI_UNKNOWN_TOKEN;
#if defined(_DEBUG) || defined(_INTERNAL)
            copyBounded(m_curBlockStart,token);
#endif
            parser(this);
#if defined(_DEBUG) || defined(_INTERNAL)
            copyBounded(m_curBlockStart,"NO_BLOCK");
#endif
        }
    } catch (...) { unPrepFile(); throw; }
    unPrepFile();
}

void INI::loadDirectoryBlocks(AsciiString directory, Bool subdirectories, INILoadType type,
                              std::span<const INIBlockDefinition> blocks, INILineTransfer transfer) {
    validateSelectedBlocks(blocks, transfer);
    if (!TheFileSystem || directory.isEmpty()) throw INI_INVALID_DIRECTORY;
    if (type<INI_LOAD_OVERWRITE || type>INI_LOAD_MULTIFILE) throw ERROR_BAD_INI;
    // The rooted filesystem returns normalized identities, not the caller's
    // spelling. Normalize the directory before deriving any relative offsets.
    std::string prefix;
    const bool absolute=directory.str()[0]=='/';
    for (unsigned char ch: std::string_view(directory.str())) {
        if (ch=='\\') ch='/';
        if (ch=='/' && !prefix.empty() && prefix.back()=='/') continue;
        if (!absolute && ch>='A' && ch<='Z') ch+=('a'-'A');
        prefix.push_back(static_cast<char>(ch));
    }
    if (prefix.back()!='/') prefix.push_back('/');
    directory=prefix.c_str();
    FilenameList files;
    TheFileSystem->getFileListInDirectory(directory, "*.ini", files, subdirectories);
    // Validate every returned identity before publishing the first definition.
    for (const auto& filename: files)
        if (std::string_view(filename.str()).size()<=prefix.size() ||
            !std::string_view(filename.str()).starts_with(prefix)) throw INI_INVALID_DIRECTORY;
    // Preserve the original root-files-before-subdirectory-files ordering,
    // including the original sorted filename selection within each pass.
    for (int nested=0; nested<2; ++nested)
        for (const auto& filename: files) {
            const std::string_view relative(filename.str()+prefix.size());
            const bool inSubdirectory=relative.find('/')!=std::string_view::npos;
            if (inSubdirectory==bool(nested)) loadBlocks(filename, type, blocks, transfer);
        }
}
