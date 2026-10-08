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

//----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright(C) 2001 - All Rights Reserved                  
//                                                                          
//----------------------------------------------------------------------------
//
// Project:   WSYS Library
//
// Module:    IO
//
// File name: WSYS_RAMFile.cpp
//
// Created:   11/08/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes                                                      
//----------------------------------------------------------------------------

#include "Common/RAMFile.h"
#include "Common/FileSystem.h"
#include <algorithm>
#include <cstring>
#include <memory>

// Original RAM file consumer; all backing is constructed before publication.
RAMFile::RAMFile():m_data(nullptr),m_pos(0),m_size(0) {}
RAMFile::~RAMFile(){m_deleteOnClose=FALSE;close();}
Bool RAMFile::open(const Char* filename,Int access) {
    if(!TheFileSystem || m_open)return FALSE;
    File* file=TheFileSystem->openFile(filename,access);
    if(!file)return FALSE;
    try {const Bool result=open(file);file->close();return result;}
    catch(...){file->close();throw;}
}
Bool RAMFile::open(File* file) {
    if(!file || m_open)return FALSE;
    const Int length=file->size(),prior=file->position();
    if(length<0 || prior<0)return FALSE;
    auto candidate=std::make_unique<char[]>(std::size_t(length)+1);
    try {
        if(file->seek(0,START)!=0){file->seek(prior,START);return FALSE;}
        if(file->read(candidate.get(),length)!=length){file->seek(prior,START);return FALSE;}
        candidate[std::size_t(length)]=0;
        if(!File::open(file->getName(),file->getAccess())){file->seek(prior,START);return FALSE;}
    }
    catch(...){file->seek(prior,START);throw;}
    m_data=candidate.release();m_size=length;m_pos=0;return TRUE;
}
Bool RAMFile::openFromArchive(File* archive,const AsciiString& filename,Int offset,Int length) {
    if(!archive || m_open || offset<0 || length<0)return FALSE;
    const Int total=archive->size(),prior=archive->position();
    if(total<0 || offset>total || length>total-offset || prior<0)return FALSE;
    auto candidate=std::make_unique<char[]>(std::size_t(length)+1);
    try {
        if(archive->seek(offset,START)!=offset){archive->seek(prior,START);return FALSE;}
        if(archive->read(candidate.get(),length)!=length){archive->seek(prior,START);return FALSE;}
        candidate[std::size_t(length)]=0;
        if(!File::open(filename.str(),READ|BINARY)){archive->seek(prior,START);return FALSE;}
    }
    catch(...){archive->seek(prior,START);throw;}
    m_data=candidate.release();m_size=length;m_pos=0;return TRUE;
}
void RAMFile::close() {
    delete[] m_data;m_data=nullptr;m_size=0;m_pos=0;File::close();
}
Int RAMFile::read(void* buffer,Int length) {
    if(!m_open || !m_data || length<0)return -1;
    length=std::min(length,m_size-m_pos);
    if(buffer && length)std::memcpy(buffer,m_data+m_pos,std::size_t(length));
    m_pos+=length;return length;
}
Int RAMFile::write(const void*,Int){return -1;}
Int RAMFile::seek(Int offset,seekMode mode) {
    if(!m_open)return -1;
    Int64 position=offset;
    switch(mode){case START:break;case CURRENT:position+=m_pos;break;case END:position+=m_size;break;default:return -1;}
    m_pos=Int(std::clamp<Int64>(position,0,m_size));return m_pos;
}
Bool RAMFile::scanInt(Int& output){return File::scanInt(output);}
Bool RAMFile::scanReal(Real& output){return File::scanReal(output);}
Bool RAMFile::scanString(AsciiString& output){return File::scanString(output);}
void RAMFile::nextLine(Char* buffer,Int capacity){File::nextLine(buffer,capacity);}
Bool RAMFile::copyDataToFile(File* file){return file && m_open && file->write(m_data,m_size)==m_size;}
File* RAMFile::convertToRAMFile(){return this;}
char* RAMFile::readEntireAndClose() {
    if(!m_open || !m_data)throw ERROR_BAD_ARG;
    char* result=m_data;m_data=nullptr;close();return result;
}
