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

// FILE: XferSave.cpp /////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   Xfer disk write implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"
#include "Common/XferSave.h"
#include "Common/NativeTransferWire.h"
#include "Common/Snapshot.h"
#include <limits>

XferSave::XferSave() { m_xferMode=XFER_SAVE; }
XferSave::~XferSave() { abort(); }
void XferSave::abort() noexcept {
    m_output.reset();
    std::vector<std::uint64_t>{}.swap(m_blocks);
    m_identifier.clear();
}
void XferSave::open(AsciiString identifier) {
    FailureScope failure(*this);
    if (m_output) throw XFER_FILE_ALREADY_OPEN;
    if (!TheNativeUserStorage) throw XFER_FILE_NOT_OPEN;
    auto candidate=TheNativeUserStorage->beginWrite(NativeUserArea::Data,
        nativeTransferUserPath(*TheNativeUserStorage,identifier));
    XferBase::open(identifier);
    m_output=std::move(candidate); m_blocks.clear(); m_failed=false;
    m_commitResult=NativeCommitResult::Durable;
}
void XferSave::close() {
    FailureScope failure(*this);
    if (!m_output) throw XFER_FILE_NOT_OPEN;
    if (m_failed || !m_blocks.empty()) throw XFER_BEGIN_END_MISMATCH;
    m_commitResult=m_output->commit();
    m_output.reset(); m_identifier.clear();
}
Int XferSave::beginBlock() {
    FailureScope failure(*this);
    if (!m_output || m_failed) throw XFER_FILE_NOT_OPEN;
    const auto position=m_output->position();
    m_blocks.push_back(position); // Acquire journal backing before placeholder mutation.
    XferBlockSize placeholder=0;
    xferImplementation(&placeholder,sizeof placeholder);
    return XFER_OK;
}
void XferSave::endBlock() {
    FailureScope failure(*this);
    if (!m_output || m_failed) throw XFER_FILE_NOT_OPEN;
    if (m_blocks.empty()) throw XFER_BEGIN_END_MISMATCH;
    const auto end=m_output->position(),start=m_blocks.back();
    if (end<start || end-start<sizeof(XferBlockSize)
        || end-start-sizeof(XferBlockSize)>std::uint64_t(std::numeric_limits<XferBlockSize>::max()))
        throw XFER_INVALID_PARAMETERS;
    XferBlockSize size=static_cast<XferBlockSize>(end-start-sizeof(XferBlockSize));
    m_output->seek(start);
    xferImplementation(&size,sizeof size);
    m_output->seek(end);
    m_blocks.pop_back(); // Retire only after both backpatch and position restoration succeed.
}
void XferSave::skip(Int count) {
    FailureScope failure(*this);
    if (!m_output || m_failed) throw XFER_FILE_NOT_OPEN;
    if (count<0 || std::uint64_t(count)>std::numeric_limits<std::uint64_t>::max()-m_output->position())
        throw XFER_SKIP_ERROR;
    m_output->seek(m_output->position()+static_cast<std::uint64_t>(count));
}
void XferSave::xferSnapshot(Snapshot* snapshot) {
    FailureScope failure(*this);
    if (!m_output || m_failed) throw XFER_FILE_NOT_OPEN;
    if (!snapshot) throw XFER_INVALID_PARAMETERS;
    snapshot->xfer(this);
}
void XferSave::xferAsciiString(AsciiString* text) {
    FailureScope failure(*this);
    if (!text || text->getLength()>255) throw XFER_STRING_ERROR;
    UnsignedByte length=static_cast<UnsignedByte>(text->getLength());
    xferUnsignedByte(&length);
    if (length) xferUser(const_cast<Char*>(text->str()),length);
}
void XferSave::xferUnicodeString(UnicodeString* text) {
    FailureScope failure(*this);
    if (!text) throw XFER_STRING_ERROR;
    auto wire=nativeTransferUTF16(*text,255);
    UnsignedByte length=static_cast<UnsignedByte>(wire.size()/2);
    xferUnsignedByte(&length);
    if (!wire.empty()) xferUser(wire.data(),static_cast<Int>(wire.size()));
}
void XferSave::xferImplementation(void* bytes,Int count) {
    FailureScope failure(*this);
    if (!m_output || m_failed) throw XFER_FILE_NOT_OPEN;
    if (count<0 || (count && !bytes)) throw XFER_INVALID_PARAMETERS;
    if (m_output->write(bytes,count)!=count) throw XFER_WRITE_ERROR;
}
