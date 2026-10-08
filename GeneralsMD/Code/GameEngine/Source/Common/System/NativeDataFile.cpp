// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/NativeDataFile.h"
#include "Common/RAMFile.h"
#include <cerrno>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

NativeDataBacking::NativeDataBacking(const std::string& path) {
    const int candidate=::open(path.c_str(),O_RDONLY|O_CLOEXEC|O_NOFOLLOW);
    if(candidate<0)throw ERROR_BAD_ARG;
    struct stat info{};
    if(::fstat(candidate,&info)!=0 || !S_ISREG(info.st_mode) || info.st_size<0) {
        ::close(candidate);throw ERROR_BAD_ARG;
    }
    descriptor=candidate;length=std::uint64_t(info.st_size);
}
NativeDataBacking::~NativeDataBacking(){if(descriptor>=0)::close(descriptor);}
NativeDataBacking::NativeDataBacking(int candidate, AdmittedDescriptor) {
    struct stat info{};
    const int flags=candidate>=0 ? ::fcntl(candidate,F_GETFL) : -1;
    if(flags<0 || (flags&O_ACCMODE)!=O_RDONLY || ::fstat(candidate,&info)!=0 ||
       !S_ISREG(info.st_mode) || info.st_size<0 || std::uint64_t(info.st_size)>INT32_MAX)
        throw ERROR_BAD_ARG;
    descriptor=candidate;length=std::uint64_t(info.st_size);
}
Int NativeDataBacking::readAt(void* data,Int bytes,std::uint64_t offset) const {
    if(bytes<0 || (bytes && !data) || offset>length || std::uint64_t(bytes)>length-offset)return -1;
    Int done=0;
    while(done<bytes) {
        const auto result=::pread(descriptor,static_cast<char*>(data)+done,
                                  std::size_t(bytes-done),off_t(offset+std::uint64_t(done)));
        if(result<0){if(errno==EINTR)continue;return -1;}
        if(result==0)break;
        done+=Int(result);
    }
    return done;
}
NativeDataFile::NativeDataFile(std::shared_ptr<NativeDataBacking> backing,
 std::uint64_t offset,Int length,const char* name,Int access):m_backing(std::move(backing)),m_offset(offset),m_length(length) {
    constexpr Int allowed=READ|TEXT|BINARY|STREAMING;
    if(!m_backing || length<0 || offset>m_backing->length || std::uint64_t(length)>m_backing->length-offset ||
       (access&~allowed) || !File::open(name,access))throw ERROR_BAD_ARG;
}
NativeDataFile::~NativeDataFile(){m_deleteOnClose=FALSE;close();}
void NativeDataFile::close(){m_backing.reset();m_length=0;m_cursor=0;File::close();}
Int NativeDataFile::read(void* buffer,Int bytes) {
    if(!m_open || bytes<0)return -1;
    bytes=std::min(bytes,m_length-m_cursor);
    if(buffer && m_backing->readAt(buffer,bytes,m_offset+std::uint64_t(m_cursor))!=bytes)return -1;
    m_cursor+=bytes;return bytes;
}
Int NativeDataFile::seek(Int offset,seekMode mode) {
    if(!m_open)return -1;
    Int64 position=offset;
    switch(mode){case START:break;case CURRENT:position+=m_cursor;break;case END:position+=m_length;break;default:return -1;}
    m_cursor=Int(std::clamp<Int64>(position,0,m_length));return m_cursor;
}
void NativeDataFile::nextLine(Char* buffer,Int capacity){File::nextLine(buffer,capacity);}
Bool NativeDataFile::scanInt(Int& output){return File::scanInt(output);}
Bool NativeDataFile::scanReal(Real& output){return File::scanReal(output);}
Bool NativeDataFile::scanString(AsciiString& output){return File::scanString(output);}
char* NativeDataFile::readEntireAndClose() {
    if(!m_open)throw ERROR_BAD_ARG;
    auto candidate=std::make_unique<char[]>(std::size_t(m_length));
    if(m_backing->readAt(candidate.get(),m_length,m_offset)!=m_length)throw ERROR_BAD_ARG;
    close();return candidate.release();
}
File* NativeDataFile::convertToRAMFile() {
    auto* candidate=newInstance(RAMFile);MemoryPoolObjectHolder guard(candidate);
    const Int prior=m_cursor;
    try {seek(0,START);if(!candidate->open(this)){m_cursor=prior;return this;}}
    catch(...){m_cursor=prior;throw;}
    const Bool selfDeleting=m_deleteOnClose;
    if(selfDeleting)candidate->deleteOnClose();
    guard.release();close();
    if(!selfDeleting)deleteInstance();
    return candidate;
}
