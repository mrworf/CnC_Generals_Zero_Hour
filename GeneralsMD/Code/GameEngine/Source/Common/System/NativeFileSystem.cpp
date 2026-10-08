// SPDX-License-Identifier: GPL-3.0-or-later
// Native implementation of the original FileSystem consumer boundary.
#include "Common/FileSystem.h"
#include "Common/NativeDataFile.h"
#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits>
#include <map>

namespace {
struct DirectoryCloser {void operator()(DIR* directory)const noexcept {if(directory)::closedir(directory);}};
using Directory=std::unique_ptr<DIR,DirectoryCloser>;
Directory ownedDirectory(int descriptor) {
    if(descriptor<0)throw ERROR_BAD_ARG;
    DIR* directory=::fdopendir(descriptor);
    if(!directory){::close(descriptor);throw ERROR_BAD_ARG;}
    return Directory(directory);
}
std::string canonicalRoot(const std::string& supplied) {
    std::unique_ptr<char,decltype(&std::free)> resolved(::realpath(supplied.c_str(),nullptr),&std::free);
    if(!resolved)throw ERROR_BAD_ARG;
    return resolved.get();
}
std::string logicalPath(std::string path) {
    if(path.empty() || path.front()=='/' || path.front()=='\\')throw ERROR_BAD_ARG;
    for(char& value:path) {
        const unsigned char byte=static_cast<unsigned char>(value);
        if(byte<32 || value==':' || byte==127)throw ERROR_BAD_ARG;
        if(value=='\\')value='/';
        else if(value>='A' && value<='Z')value=char(value-'A'+'a');
    }
    std::size_t start=0;
    for(;;) {
        const auto end=path.find('/',start);const auto part=path.substr(start,end-start);
        if(part.empty() || part=="." || part=="..")throw ERROR_BAD_ARG;
        if(end==std::string::npos)break;
        start=end+1;
    }
    return path;
}
struct Range {
    std::shared_ptr<NativeDataBacking> backing;
    std::string loosePath;
    std::uint64_t offset=0;
    Int size=0;
};
std::map<std::string,std::pair<std::string,Int>> physicalFiles(const std::string& root) {
    struct Pending {std::string relative;Directory directory;};
    std::vector<Pending> pending;
    auto directory=ownedDirectory(::open(root.c_str(),O_RDONLY|O_DIRECTORY|O_CLOEXEC|O_NOFOLLOW));
    pending.push_back({"",std::move(directory)});
    std::map<std::string,std::pair<std::string,Int>> paths;
    while(!pending.empty()) {
        auto& current=pending.back();errno=0;
        dirent* entry=::readdir(current.directory.get());
        if(!entry){if(errno)throw ERROR_BAD_ARG;pending.pop_back();continue;}
        const std::string name=entry->d_name;
        if(name=="."||name=="..")continue;
        const auto relative=current.relative+name;
        struct stat info{};
        if(::fstatat(::dirfd(current.directory.get()),name.c_str(),&info,AT_SYMLINK_NOFOLLOW)!=0)throw ERROR_BAD_ARG;
        if(S_ISDIR(info.st_mode)) {
            auto child=ownedDirectory(::openat(::dirfd(current.directory.get()),name.c_str(),
                O_RDONLY|O_DIRECTORY|O_CLOEXEC|O_NOFOLLOW));
            pending.push_back({relative+"/",std::move(child)});
        }else if(S_ISREG(info.st_mode)) {
            if(info.st_size<0 || std::uint64_t(info.st_size)>std::uint64_t(INT32_MAX))throw ERROR_BAD_ARG;
            if(!paths.emplace(logicalPath(relative),std::pair{root+"/"+relative,Int(info.st_size)}).second)throw ERROR_BAD_ARG;
        }
    }
    return paths;
}
UnsignedInt big32(const std::array<unsigned char,4>& bytes) {
    return (UnsignedInt(bytes[0])<<24)|(UnsignedInt(bytes[1])<<16)|(UnsignedInt(bytes[2])<<8)|UnsignedInt(bytes[3]);
}
std::map<std::string,Range> readBig(const std::string& path) {
    auto backing=std::make_shared<NativeDataBacking>(path);
    if(backing->length<16 || backing->length>std::uint64_t(INT32_MAX))throw ERROR_BAD_ARG;
    std::array<char,4> magic{};
    if(backing->readAt(magic.data(),4,0)!=4 || std::string(magic.data(),4)!="BIGF")throw ERROR_BAD_ARG;
    std::array<unsigned char,4> word{};
    if(backing->readAt(word.data(),4,8)!=4)throw ERROR_BAD_ARG;
    const auto count=big32(word);
    if(count>(backing->length-16)/9)throw ERROR_BAD_ARG;
    std::uint64_t position=16,firstPayload=backing->length;
    std::map<std::string,Range> candidate;
    for(UnsignedInt ordinal=0;ordinal<count;++ordinal) {
        if(position>firstPayload || firstPayload-position<9)throw ERROR_BAD_ARG;
        if(backing->readAt(word.data(),4,position)!=4)throw ERROR_BAD_ARG;
        const auto offset=big32(word);position+=4;
        if(backing->readAt(word.data(),4,position)!=4)throw ERROR_BAD_ARG;
        const auto size=big32(word);position+=4;
        if(offset>backing->length || size>backing->length-offset || size>UnsignedInt(INT32_MAX))throw ERROR_BAD_ARG;
        // Empty entries may use offset zero: they have no payload ownership.
        // Only nonempty ranges can establish the index/payload boundary.
        if(size)firstPayload=std::min(firstPayload,std::uint64_t(offset));
        std::string name;char value=0;
        for(;;) {
            if(position>=firstPayload || backing->readAt(&value,1,position)!=1)throw ERROR_BAD_ARG;
            ++position;if(!value)break;
            if(name.size()>=1023)throw ERROR_BAD_ARG;
            name+=value;
        }
        if(!candidate.emplace(logicalPath(name),Range{backing,"",offset,Int(size)}).second)throw ERROR_BAD_ARG;
    }
    if(position>firstPayload)throw ERROR_BAD_ARG;
    return candidate;
}
bool matches(const std::string& filename,const std::string& pattern) {
    // Original filename consumers use '*' and '?' DOS-style wildcard masks.
    std::size_t name=0,mask=0,star=std::string::npos,retry=0;
    while(name<filename.size()) {
        if(mask<pattern.size() && (pattern[mask]=='?' || pattern[mask]==filename[name])){++mask;++name;}
        else if(mask<pattern.size() && pattern[mask]=='*'){star=mask++;retry=name;}
        else if(star!=std::string::npos){mask=star+1;name=++retry;}
        else return false;
    }
    while(mask<pattern.size() && pattern[mask]=='*')++mask;
    return mask==pattern.size();
}
}
struct FileSystem::NativeMounts {
    std::map<std::string,Range> loose,archived;
};
FileSystem* TheFileSystem=nullptr;
FileSystem::FileSystem()=default;
FileSystem::~FileSystem()=default;
void FileSystem::init() {}
void FileSystem::reset(){m_fileExist.clear();}
void FileSystem::update() {}
void FileSystem::mountReadOnly(const std::vector<std::string>& roots) {
    if(roots.empty())throw ERROR_BAD_ARG;
    auto candidate=std::make_unique<NativeMounts>();
    for(const auto& supplied:roots) {
        if(supplied.empty())throw ERROR_BAD_ARG;
        const auto root=canonicalRoot(supplied);
        const auto paths=physicalFiles(root);
        for(const auto& [name,physical]:paths) {
            candidate->loose.try_emplace(name,Range{nullptr,physical.first,0,physical.second});
        }
        // First sorted archive wins; first configured root wins across archives.
        for(const auto& [name,physical]:paths)if(name.size()>=4 && name.substr(name.size()-4)==".big") {
            // GameEngine's original patch startup deleted this obsolete duplicate.
            // Preserve that selection without deleting user-supplied bytes.
            if(name=="data/ini/inizh.big")continue;
            auto archive=readBig(physical.first);
            for(auto& [logical,range]:archive)candidate->archived.try_emplace(logical,std::move(range));
        }
    }
    m_nativeMounts.swap(candidate);m_fileExist.clear();
}
File* FileSystem::openFile(const Char* filename,Int access) {
    if(!filename || !m_nativeMounts)return nullptr;
    constexpr Int allowed=File::READ|File::TEXT|File::BINARY|File::STREAMING;
    if(access&~allowed)throw ERROR_BAD_ARG;
    const auto name=logicalPath(filename);
    const Range* range=nullptr;
    auto loose=m_nativeMounts->loose.find(name);
    if(loose!=m_nativeMounts->loose.end())range=&loose->second;
    else {auto archived=m_nativeMounts->archived.find(name);if(archived!=m_nativeMounts->archived.end())range=&archived->second;}
    if(!range)return nullptr;
    auto backing=range->backing?range->backing:std::make_shared<NativeDataBacking>(range->loosePath);
    auto* file=new(NativeDataFile::NativeDataFile_GLUE_NOT_IMPLEMENTED)
        NativeDataFile(std::move(backing),range->offset,range->size,filename,access);
    file->deleteOnClose();return file;
}
Bool FileSystem::doesFileExist(const Char* filename) const {
    if(!filename || !m_nativeMounts)return FALSE;
    const auto name=logicalPath(filename);
    return m_nativeMounts->loose.contains(name) || m_nativeMounts->archived.contains(name);
}
Bool FileSystem::getFileInfo(const AsciiString& filename,FileInfo* output) const {
    if(!output || !m_nativeMounts)return FALSE;
    const auto name=logicalPath(filename.str());const Range* range=nullptr;
    auto loose=m_nativeMounts->loose.find(name);
    if(loose!=m_nativeMounts->loose.end())range=&loose->second;
    else {auto archived=m_nativeMounts->archived.find(name);if(archived!=m_nativeMounts->archived.end())range=&archived->second;}
    if(!range)return FALSE;
    *output=FileInfo{0,range->size,0,0};return TRUE;
}
void FileSystem::getFileListInDirectory(const AsciiString& directory,const AsciiString& mask,
 FilenameList& output,Bool recursive) const {
    if(!m_nativeMounts)return;
    std::string prefix=directory.str();
    while(!prefix.empty() && (prefix.back()=='/' || prefix.back()=='\\'))prefix.pop_back();
    if(!prefix.empty())prefix=logicalPath(prefix)+"/";
    const auto pattern=logicalPath(mask.str());FilenameList candidate(output);
    const auto append=[&](const auto& table){for(const auto& [name,range]:table) {
        (void)range;if(!name.starts_with(prefix))continue;
        const auto tail=name.substr(prefix.size());
        if(!recursive && tail.find('/')!=std::string::npos)continue;
        const auto base=tail.substr(tail.find_last_of('/')==std::string::npos?0:tail.find_last_of('/')+1);
        if(matches(base,pattern))candidate.insert(AsciiString(name.c_str()));
    }};
    append(m_nativeMounts->loose);append(m_nativeMounts->archived);output.swap(candidate);
}
Bool FileSystem::createDirectory(AsciiString){return FALSE;} // asset mounts never writable
Bool FileSystem::areMusicFilesOnCD(){return FALSE;} // explicit roots, no CD discovery
void FileSystem::loadMusicFilesFromCD() {}
void FileSystem::unloadMusicFilesFromCD() {}
