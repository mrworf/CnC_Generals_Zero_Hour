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

// FILE: XferSave.h ///////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   Xfer hard disk write implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __XFER_SAVE_H_
#define __XFER_SAVE_H_

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "Common/Xfer.h"
#include "Common/NativeUserStorage.h"
#include <vector>

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class XferBlockData;
class Snapshot;

///////////////////////////////////////////////////////////////////////////////////////////////////
typedef long XferFilePos;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class XferSave : public XferBase
{

public:

	XferSave( void );
	virtual ~XferSave( void );

	// Xfer methods
	virtual void open( AsciiString identifier );		///< open file for writing
	virtual void close( void );											///< close file
	void abort() noexcept;
	NativeCommitResult getCommitResult() const noexcept { return m_commitResult; }
	virtual Int beginBlock( void );									///< write placeholder block size
	virtual void endBlock( void );									///< backup to last begin block and write size
	virtual void skip( Int dataSize );							///< skipping during a write is a no-op

	virtual void xferSnapshot( Snapshot *snapshot );		///< entry point for xfering a snapshot

	// xfer methods
	virtual void xferAsciiString( AsciiString *asciiStringData );  ///< xfer ascii string (need our own)
	virtual void xferUnicodeString( UnicodeString *unicodeStringData );	///< xfer unicode string (need our own);

protected:

	virtual void xferImplementation( void *data, Int dataSize );		///< the xfer implementation

	std::unique_ptr<NativeAtomicOutput> m_output;
	std::vector<std::uint64_t> m_blocks;
	NativeCommitResult m_commitResult=NativeCommitResult::Durable;

};

#endif // __XFER_SAVE_H_
