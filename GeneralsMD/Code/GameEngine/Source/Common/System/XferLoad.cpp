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

// FILE: XferLoad.cpp /////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   Xfer implemenation for loading from disk
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"
#include "Common/XferLoad.h"
#include "Common/NativeTransferWire.h"
#include "Common/NativeTransferServices.h"
#include "Common/Snapshot.h"
#include <array>
#include <cstring>
#include <limits>

XferLoad::XferLoad() { m_xferMode=XFER_LOAD; }
XferLoad::~XferLoad() { abort(); }
void XferLoad::abort() noexcept {
    std::vector<unsigned char>{}.swap(m_bytes);
    m_position=0; m_open=false; m_identifier.clear();
}
void XferLoad::open(AsciiString identifier) {
    FailureScope failure(*this);
    if (m_open) throw XFER_FILE_ALREADY_OPEN;
    if (!TheNativeUserStorage) throw XFER_FILE_NOT_OPEN;
    auto candidate=TheNativeUserStorage->readFile(NativeUserArea::Data,
        nativeTransferUserPath(*TheNativeUserStorage,identifier),
        static_cast<std::size_t>(std::numeric_limits<Int>::max()));
    if (!candidate) throw XFER_FILE_NOT_FOUND;
    XferBase::open(identifier);
    m_bytes=std::move(*candidate); m_position=0; m_open=true; m_failed=false;
}
void XferLoad::close() {
    if (!m_open) throw XFER_FILE_NOT_OPEN;
    abort();
}
Int XferLoad::beginBlock() {
    FailureScope failure(*this);
    XferBlockSize size=0;
    xferImplementation(&size,sizeof size);
    if (size<0 || static_cast<std::size_t>(size)>m_bytes.size()-m_position)
        throw XFER_READ_ERROR;
    return size;
}
void XferLoad::endBlock() {
    FailureScope failure(*this);
    if (!m_open || m_failed) throw XFER_FILE_NOT_OPEN;
}
void XferLoad::skip(Int count) {
    FailureScope failure(*this);
    if (!m_open || m_failed) throw XFER_FILE_NOT_OPEN;
    if (count<0 || static_cast<std::size_t>(count)>m_bytes.size()-m_position)
        throw XFER_SKIP_ERROR;
    m_position+=static_cast<std::size_t>(count);
}
void XferLoad::xferSnapshot(Snapshot* snapshot) {
    FailureScope failure(*this);
    if (!m_open || m_failed) throw XFER_FILE_NOT_OPEN;
    const auto services=nativeTransferServices();
    if (!snapshot || (!BitTest(getOptions(),XO_NO_POST_PROCESSING) && !services.owner))
        throw XFER_INVALID_PARAMETERS;
    snapshot->xfer(this);
    if (!BitTest(getOptions(),XO_NO_POST_PROCESSING))
        services.postProcess(services.owner,snapshot);
}
void XferLoad::xferAsciiString(AsciiString* text) {
    FailureScope failure(*this);
    if (!text) throw XFER_INVALID_PARAMETERS;
    UnsignedByte length=0; xferUnsignedByte(&length);
    std::array<Char,256> bytes{};
    if (length) xferUser(bytes.data(),length);
    if (std::memchr(bytes.data(),0,length)) throw XFER_STRING_ERROR;
    AsciiString candidate(bytes.data());
    *text=candidate;
}
void XferLoad::xferUnicodeString(UnicodeString* text) {
    FailureScope failure(*this);
    if (!text) throw XFER_INVALID_PARAMETERS;
    UnsignedByte length=0; xferUnsignedByte(&length);
    std::array<unsigned char,510> bytes{};
    if (length) xferUser(bytes.data(),static_cast<Int>(length)*2);
    auto candidate=nativeTransferDecodeUTF16(std::span<const unsigned char>(bytes).first(std::size_t(length)*2));
    *text=candidate;
}
void XferLoad::xferImplementation(void* bytes,Int count) {
    FailureScope failure(*this);
    if (!m_open || m_failed) throw XFER_FILE_NOT_OPEN;
    if (count<0 || (count && !bytes)) throw XFER_INVALID_PARAMETERS;
    if (static_cast<std::size_t>(count)>m_bytes.size()-m_position) throw XFER_READ_ERROR;
    if (count) std::memcpy(bytes,m_bytes.data()+m_position,static_cast<std::size_t>(count));
    m_position+=static_cast<std::size_t>(count);
}
