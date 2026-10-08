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

// FILE: XferCRC.cpp //////////////////////////////////////////////////////////////////////////////
// Author: Matt Campbell, February 2002
// Desc:   Xfer CRC implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"
#include "Common/XferCRC.h"
#include "Common/XferDeepCRC.h"
#include "Common/NativeTransferWire.h"
#include "Common/Snapshot.h"
#include <arpa/inet.h>
#include <cstring>
#include <limits>

XferCRC::XferCRC() { m_xferMode=XFER_CRC; m_crc=0; }
XferCRC::~XferCRC() = default;
void XferCRC::open(AsciiString identifier) {
    FailureScope failure(*this);
    XferBase::open(identifier); m_crc=0; m_failed=false;
}
void XferCRC::close() { if (m_failed) throw XFER_INVALID_PARAMETERS; }
Int XferCRC::beginBlock() { return 0; }
void XferCRC::endBlock() {}
void XferCRC::skip(Int) {}
void XferCRC::addCRC(UnsignedInt value) {
    static_assert(sizeof(UnsignedInt)==4);
    value=htonl(value);
    const UnsignedInt carry=(m_crc&0x80000000u) ? 1u : 0u;
    m_crc=(m_crc<<1)+value+carry;
}
void XferCRC::xferSnapshot(Snapshot* snapshot) {
    FailureScope failure(*this);
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    if (snapshot) snapshot->crc(this); // The CRC owner's source null snapshot is a no-op.
}
void XferCRC::xferImplementation(void* data,Int count) {
    FailureScope failure(*this);
    if (m_failed || count<0 || (count && !data)) throw XFER_INVALID_PARAMETERS;
    const auto* bytes=static_cast<const unsigned char*>(data);
    for (Int i=0;i<count/4;++i) {
        UnsignedInt value=0;
        std::memcpy(&value,bytes+std::size_t(i)*4,4);
        addCRC(value);
    }
    const Int leftover=count&3;
    if (leftover) {
        UnsignedInt value=0;
        const auto* tail=bytes+std::size_t(count/4)*4;
        for (Int i=0;i<leftover;++i) value+=UnsignedInt(tail[i])<<(i*8);
        value=htonl(value); // Preserve source double conversion in the leftover path.
        addCRC(value);
    }
}
UnsignedInt XferCRC::getCRC() {
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    return htonl(m_crc);
}

XferDeepCRC::XferDeepCRC() { m_xferMode=XFER_SAVE; }
XferDeepCRC::~XferDeepCRC() { abort(); }
void XferDeepCRC::abort() noexcept { m_output.reset(); m_identifier.clear(); }
void XferDeepCRC::open(AsciiString identifier) {
    FailureScope failure(*this);
    if (m_output) throw XFER_FILE_ALREADY_OPEN;
    if (!TheNativeUserStorage) throw XFER_FILE_NOT_OPEN;
    auto candidate=TheNativeUserStorage->beginWrite(NativeUserArea::Data,
        nativeTransferUserPath(*TheNativeUserStorage,identifier));
    XferBase::open(identifier);
    m_output=std::move(candidate); m_crc=0; m_failed=false;
}
void XferDeepCRC::close() {
    FailureScope failure(*this);
    if (!m_output) throw XFER_FILE_NOT_OPEN;
    if (m_failed) throw XFER_INVALID_PARAMETERS;
    m_commitResult=m_output->commit();
    m_output.reset(); m_identifier.clear();
}
void XferDeepCRC::xferImplementation(void* bytes,Int count) {
    FailureScope failure(*this);
    if (!m_output || m_failed) throw XFER_FILE_NOT_OPEN;
    if (count<0 || (count && !bytes)) throw XFER_INVALID_PARAMETERS;
    if (m_output->write(bytes,count)!=count) throw XFER_WRITE_ERROR;
    XferCRC::xferImplementation(bytes,count);
}
void XferDeepCRC::xferMarkerLabel(AsciiString) {}
void XferDeepCRC::xferAsciiString(AsciiString* text) {
    FailureScope failure(*this);
    if (!text || text->getLength()>16385) throw XFER_STRING_ERROR;
    UnsignedShort length=static_cast<UnsignedShort>(text->getLength());
    xferUnsignedShort(&length);
    if (length) xferUser(const_cast<Char*>(text->str()),length);
}
void XferDeepCRC::xferUnicodeString(UnicodeString* text) {
    FailureScope failure(*this);
    if (!text) throw XFER_STRING_ERROR;
    auto wire=nativeTransferUTF16(*text,255);
    UnsignedByte length=static_cast<UnsignedByte>(wire.size()/2);
    xferUnsignedByte(&length);
    if (!wire.empty()) xferUser(wire.data(),static_cast<Int>(wire.size()));
}
