// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/NativeUserStorage.h"
#include "Common/FileSystem.h"
#include "Common/NativeDataFile.h"
#include "Common/FileOwner.h"
#include "Common/NativeFileMetadata.h"
#include <algorithm>
#include <atomic>
#include <bit>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <limits>
#include <map>
#include <sys/stat.h>
#include <unistd.h>
NativeUserStorage* TheNativeUserStorage = nullptr;
namespace {
struct Descriptor {
  int value = -1;
  explicit Descriptor(int fd = -1) : value(fd) {}
  ~Descriptor() {
    if (value >= 0)
      ::close(value);
  }
  Descriptor(const Descriptor &) = delete;
  int release() noexcept {
    const int result = value;
    value = -1;
    return result;
  }
  void replace(int fd) noexcept {
    if (value >= 0)
      ::close(value);
    value = fd;
  }
};
NativeStorageIO defaultIO;
struct DirectoryCloser {
  void operator()(DIR* directory) const noexcept { if(directory) ::closedir(directory); }
};
using Directory=std::unique_ptr<DIR,DirectoryCloser>;
Directory ownedDirectory(int descriptor) {
  Descriptor guard(descriptor);
  if(descriptor<0) throw NativeStorageError();
  DIR* directory=::fdopendir(descriptor);
  if(!directory) throw NativeStorageError();
  guard.release();
  return Directory(directory);
}
std::string folded(std::string value) {
  for(char& byte:value) if(byte>='A' && byte<='Z') byte=char(byte-'A'+'a');
  return value;
}
void validateAbsolute(std::string_view path) {
  if (path.empty() || path.front() != '/' || path.find('\0') != path.npos)
    throw NativeStorageError();
}
std::vector<std::string> components(std::string_view relative) {
  std::vector<std::string> result;
  while (!relative.empty()) {
    const auto end = relative.find('/');
    const auto part = relative.substr(0, end);
    if (part.empty() || part == "." || part == "..")
      throw NativeStorageError();
    for (unsigned char value : part)
      if (value < 32 || value == 127 || value == ':' || value == '\\')
        throw NativeStorageError();
    result.emplace_back(part);
    if (end == relative.npos)
      break;
    relative.remove_prefix(end + 1);
    if (relative.empty())
      throw NativeStorageError();
  }
  return result;
}
// Resolve aliases in the existing ancestor without creating anything. Missing
// suffixes are admitted only as simple components, then walked with no-follow.
std::string prospectivePath(std::string path) {
  validateAbsolute(path);
  while (path.size() > 1 && path.back() == '/')
    path.pop_back();
  std::vector<std::string> missing;
  for (;;) {
    std::unique_ptr<char, decltype(&std::free)> resolved(
        ::realpath(path.c_str(), nullptr), &std::free);
    if (resolved) {
      std::string result(resolved.get());
      for (auto part = missing.rbegin(); part != missing.rend(); ++part) {
        if (result.back() != '/')
          result += '/';
        result += *part;
      }
      return result;
    }
    if (errno != ENOENT)
      throw NativeStorageError();
    const auto slash = path.find_last_of('/');
    auto part = path.substr(slash + 1);
    (void)components(part);
    missing.push_back(std::move(part));
    path.resize(slash == 0 ? 1 : slash);
  }
}
} // namespace
NativeUserPaths NativeUserPaths::resolve(std::string_view home,
                                         std::string_view data,
                                         std::string_view cache) {
  auto base = [&](std::string_view selected, const char *fallback) {
    std::string result;
    if (!selected.empty() && selected.front() == '/')
      result = selected;
    else {
      validateAbsolute(home);
      result = home;
      result += fallback;
    }
    validateAbsolute(result);
    while (result.size() > 1 && result.back() == '/')
      result.pop_back();
    if (result.back() != '/')
      result += '/';
    return result + "cnc-generals-zero-hour";
  };
  return {base(data, "/.local/share"), base(cache, "/.cache")};
}
NativeUserPaths NativeUserPaths::fromEnvironment() {
  const auto value = [](const char *key) -> std::string_view {
    const char *input = std::getenv(key);
    return input ? input : "";
  };
  return resolve(value("HOME"), value("XDG_DATA_HOME"),
                 value("XDG_CACHE_HOME"));
}
int NativeStorageIO::openFile(int directory, const char *name, int flags,
                              unsigned mode) {
  return ::openat(directory, name, flags, mode_t(mode));
}
Int NativeStorageIO::writeFile(int descriptor, const void *bytes, Int count) {
  const auto result =
      ::write(descriptor, bytes, static_cast<std::size_t>(count));
  return result < 0 ? -1 : static_cast<Int>(result);
}
std::int64_t NativeStorageIO::seekFile(int descriptor, std::uint64_t offset) {
  if(offset>std::uint64_t(std::numeric_limits<off_t>::max())) return -1;
  return ::lseek(descriptor,static_cast<off_t>(offset),SEEK_SET);
}
int NativeStorageIO::sync(int descriptor) { return ::fsync(descriptor); }
void NativeStorageIO::preparePublish() {}
NativeUserStorage::NativeUserStorage(NativeUserPaths paths,
                                     const FileSystem &assets,
                                     NativeStorageIO *io)
    : m_paths(std::move(paths)), m_assets(&assets), m_io(io ? io : &defaultIO) {
}
NativeUserStorage::~NativeUserStorage() {m_assets->withdrawUserStorage(this);}
void NativeUserStorage::validateRootOwnership() const {
  const auto data=prospectivePath(m_paths.data);
  const auto cache=prospectivePath(m_paths.cache);
  if(!m_assets->admitsUserStorage(data) || !m_assets->admitsUserStorage(cache))
    throw NativeStorageError();
}
std::optional<std::string> NativeUserStorage::relativeDataPath(std::string_view absolute) const {
  validateAbsolute(m_paths.data);
  std::string_view root=m_paths.data;
  while(root.size()>1 && root.back()=='/') root.remove_suffix(1);
  if(absolute==root) return std::string{};
  if(!absolute.starts_with(root) || absolute.size()<=root.size() || absolute[root.size()]!='/')
    return std::nullopt;
  std::string result(absolute.substr(root.size()+1));
  for(char& value:result) if(value=='\\') value='/';
  if(components(result).empty()) throw NativeStorageError();
  return result;
}
std::string NativeUserStorage::mapIdentityPath(std::string_view identity) const {
  validateAbsolute(m_paths.data);
  std::string root=m_paths.data;
  while(root.size()>1 && root.back()=='/') root.pop_back();
  std::string source(identity);
  for(char& byte:source) if(byte=='\\') byte='/';
  if(source.size()<=root.size() || source[root.size()]!='/' ||
      folded(source.substr(0,root.size()))!=folded(root)) throw NativeStorageError();
  const auto parts=components(std::string_view(source).substr(root.size()+1));
  if(parts.empty() || (folded(parts.front())!="maps" && folded(parts.front())!="save"))
    throw NativeStorageError();
  std::string relative;
  std::size_t remaining=MaximumDirectoryEntries;
  bool missing=false;
  for(std::size_t ordinal=0;ordinal<parts.size();++ordinal) {
    std::string selected=parts[ordinal];
    if(!missing) {
      const auto entries=list(NativeUserArea::Data,relative,false,remaining);
      remaining-=entries.size();
      missing=true;
      const auto key=folded(selected);
      for(const auto& entry:entries) {
        const auto slash=entry.relative.find_last_of('/');
        const auto leaf=entry.relative.substr(slash==std::string::npos?0:slash+1);
        if(folded(leaf)==key) {
          if(ordinal+1<parts.size() && !entry.directory) throw NativeStorageError();
          selected=leaf;missing=false;break;
        }
      }
    }
    if(!relative.empty()) relative+='/';
    relative+=selected;
  }
  return root+"/"+relative;
}
std::vector<NativeUserEntry> NativeUserStorage::list(NativeUserArea area,
    std::string_view relative, bool recursive, std::size_t maximumEntries) const {
  // Validate before acquiring descriptors or publishing any result.
  (void)components(relative);
  const int descriptor=openDirectory(area,relative,false);
  if(descriptor<0) return {};
  struct Pending { std::string prefix; Directory directory; };
  auto initial=ownedDirectory(descriptor);
  std::vector<Pending> pending;
  std::vector<NativeUserEntry> result;
  pending.push_back({relative.empty()?std::string{}:std::string(relative)+"/",std::move(initial)});
  std::size_t encountered=0;
  while(!pending.empty()) {
    // Complete each directory's case admission before descending into it.
    auto current=std::move(pending.back());pending.pop_back();
    std::map<std::string,NativeUserEntry> entries;
    for(;;) {
      errno=0;const auto* entry=::readdir(current.directory.get());
      if(!entry) {if(errno) throw NativeStorageError();break;}
      const std::string name(entry->d_name);
      if(name=="." || name=="..") continue;
      if(encountered==maximumEntries) throw NativeStorageError();
      ++encountered;
      (void)components(name);
      struct stat info{};
      if(::fstatat(::dirfd(current.directory.get()),name.c_str(),&info,AT_SYMLINK_NOFOLLOW)!=0)
        throw NativeStorageError();
      if(!S_ISREG(info.st_mode) && !S_ISDIR(info.st_mode)) throw NativeStorageError();
      if(info.st_size<0 || (S_ISREG(info.st_mode) && std::uint64_t(info.st_size)>INT32_MAX))
        throw NativeStorageError();
      NativeUserEntry candidate{current.prefix+name,
          S_ISREG(info.st_mode)?std::uint64_t(info.st_size):0,S_ISDIR(info.st_mode)};
      if(!entries.emplace(folded(name),std::move(candidate)).second) throw NativeStorageError();
    }
    for(auto& [key,entry]:entries) {
      (void)key;
      if(recursive && entry.directory) {
        const auto leaf=entry.relative.substr(current.prefix.size());
        auto child=ownedDirectory(::openat(::dirfd(current.directory.get()),leaf.c_str(),
            O_RDONLY|O_DIRECTORY|O_CLOEXEC|O_NOFOLLOW));
        pending.push_back({entry.relative+"/",std::move(child)});
      }
      result.push_back(std::move(entry));
    }
  }
  std::sort(result.begin(),result.end(),[](const auto& left,const auto& right){
    return left.relative<right.relative;
  });
  return result;
}
NativeCommitResult NativeUserStorage::copy(NativeUserArea area,std::string_view source,
    std::string_view destination) const {
  // Validate both names before acquiring or creating any candidate output.
  if(components(source).empty() || components(destination).empty()) throw NativeStorageError();
  FileCloseOwner input(openReadFile(area,source));
  if(!input) throw NativeStorageError();
  auto output=beginWrite(area,destination);
  std::array<char,16384> bytes{};
  Int remaining=input->size();
  while(remaining) {
    const Int count=std::min<Int>(remaining,bytes.size());
    if(input->read(bytes.data(),count)!=count) throw NativeStorageError();
    output->write(bytes.data(),count);
    remaining-=count;
  }
  return output->commit();
}
bool NativeUserStorage::removeFile(NativeUserArea area,std::string_view relative) const {
  const auto parts=components(relative);
  if(parts.empty()) throw NativeStorageError();
  const auto slash=relative.find_last_of('/');
  const auto parents=slash==relative.npos?std::string_view{}:relative.substr(0,slash);
  Descriptor directory(openDirectory(area,parents,false));
  if(directory.value<0) return false;
  struct stat info{};
  if(::fstatat(directory.value,parts.back().c_str(),&info,AT_SYMLINK_NOFOLLOW)!=0) {
    if(errno==ENOENT) return false;
    throw NativeStorageError();
  }
  if(!S_ISREG(info.st_mode)) throw NativeStorageError();
  if(::unlinkat(directory.value,parts.back().c_str(),0)!=0) {
    if(errno==ENOENT) return false;
    throw NativeStorageError();
  }
  return true;
}
int NativeUserStorage::openDirectory(NativeUserArea area,
                                     std::string_view relative,
                                     bool create) const {
  if (area != NativeUserArea::Data && area != NativeUserArea::Cache)
    throw NativeStorageError();
  auto path = prospectivePath(area == NativeUserArea::Data ? m_paths.data
                                                           : m_paths.cache);
  const auto children = components(relative);
  for (const auto &part : children) {
    if (path.back() != '/')
      path += '/';
    path += part;
  }
  if (!m_assets->admitsUserStorage(path))
    throw NativeStorageError();
  const auto parts = components(std::string_view(path).substr(1));
  Descriptor directory(::open("/", O_RDONLY | O_DIRECTORY | O_CLOEXEC));
  if (directory.value < 0)
    throw NativeStorageError();
  for (const auto &part : parts) {
    int child = ::openat(directory.value, part.c_str(),
                         O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (child < 0 && errno == ENOENT && create) {
      if (::mkdirat(directory.value, part.c_str(), 0700) != 0 &&
          errno != EEXIST)
        throw NativeStorageError();
      child = ::openat(directory.value, part.c_str(),
                       O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    }
    if (child < 0 && errno == ENOENT && !create)
      return -1;
    if (child < 0)
      throw NativeStorageError();
    directory.replace(child);
  }
  return directory.release();
}
std::unique_ptr<NativeAtomicOutput>
NativeUserStorage::beginWrite(NativeUserArea area,
                              std::string_view relative) const {
  auto parts = components(relative);
  if (parts.empty())
    throw NativeStorageError();
  auto leaf = std::move(parts.back());
  parts.pop_back();
  std::string parents;
  for (const auto &part : parts) {
    if (!parents.empty())
      parents += '/';
    parents += part;
  }
  Descriptor directory(openDirectory(area, parents, true));
  // Allocate before handing the directory into a fallible constructor.
  auto owner = std::unique_ptr<NativeAtomicOutput>(
      new NativeAtomicOutput(directory.value, std::move(leaf), *m_io));
  directory.release();
  return owner;
}
void NativeUserStorage::ensureDirectory(NativeUserArea area,std::string_view relative) const {
  Descriptor directory(openDirectory(area,relative,true));
}
std::optional<std::vector<unsigned char>>
NativeUserStorage::readFile(NativeUserArea area, std::string_view relative,
                            std::size_t maximumBytes) const {
  auto parts = components(relative);
  if (parts.empty()) throw NativeStorageError();
  const auto slash = relative.find_last_of('/');
  const auto parents = slash == relative.npos ? std::string_view{} : relative.substr(0, slash);
  Descriptor directory(openDirectory(area, parents, false));
  if (directory.value < 0) return std::nullopt;
  Descriptor file(m_io->openFile(directory.value, parts.back().c_str(),
                                 O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK, 0));
  if (file.value < 0) {
    if (errno == ENOENT) return std::nullopt;
    throw NativeStorageError();
  }
  struct stat info{};
  if (::fstat(file.value, &info) != 0 || !S_ISREG(info.st_mode) ||
      info.st_size < 0 || std::uint64_t(info.st_size) > maximumBytes)
    throw NativeStorageError();
  std::vector<unsigned char> bytes(static_cast<std::size_t>(info.st_size));
  std::size_t done = 0;
  while (done < bytes.size()) {
    const auto count = ::read(file.value, bytes.data() + done,
                             std::min<std::size_t>(bytes.size() - done, INT32_MAX));
    if (count < 0 && errno == EINTR) continue;
    if (count <= 0) throw NativeStorageError();
    done += static_cast<std::size_t>(count);
  }
  unsigned char extra;
  ssize_t tail;
  do { tail = ::read(file.value, &extra, 1); } while (tail < 0 && errno == EINTR);
  if (tail != 0) throw NativeStorageError();
  return bytes;
}
File* NativeUserStorage::openReadFile(NativeUserArea area, std::string_view relative,Int access) const {
  constexpr Int allowed=File::READ|File::TEXT|File::BINARY|File::STREAMING;
  if((access&~allowed) || ((access&(File::TEXT|File::BINARY))==(File::TEXT|File::BINARY)))
    throw NativeStorageError();
  const auto parts=components(relative);
  if(parts.empty()) throw NativeStorageError();
  const auto slash=relative.find_last_of('/');
  const auto parents=slash==relative.npos ? std::string_view{} : relative.substr(0,slash);
  Descriptor directory(openDirectory(area,parents,false));
  if(directory.value<0) return nullptr;
  Descriptor descriptor(m_io->openFile(directory.value,parts.back().c_str(),
      O_RDONLY|O_CLOEXEC|O_NOFOLLOW|O_NONBLOCK,0));
  if(descriptor.value<0) {
    if(errno==ENOENT) return nullptr;
    throw NativeStorageError();
  }
  auto backing=std::make_shared<NativeDataBacking>(descriptor.value,
      NativeDataBacking::AdmittedDescriptor{});
  descriptor.release(); // Guard owns it until full backing construction succeeds.
  const std::string name(relative);
  auto* file=new(NativeDataFile::NativeDataFile_GLUE_NOT_IMPLEMENTED)
      NativeDataFile(backing,0,static_cast<Int>(backing->length),name.c_str(),access);
  file->deleteOnClose();return file;
}
bool NativeUserStorage::getFileInfo(NativeUserArea area,std::string_view relative,FileInfo& output) const {
  const auto parts=components(relative);
  if(parts.empty()) throw NativeStorageError();
  const auto slash=relative.find_last_of('/');
  const auto parents=slash==relative.npos?std::string_view{}:relative.substr(0,slash);
  Descriptor directory(openDirectory(area,parents,false));
  if(directory.value<0) return false;
  Descriptor descriptor(m_io->openFile(directory.value,parts.back().c_str(),
      O_RDONLY|O_CLOEXEC|O_NOFOLLOW|O_NONBLOCK,0));
  if(descriptor.value<0) {
    if(errno==ENOENT) return false;
    throw NativeStorageError();
  }
  struct stat info{};std::uint64_t timestamp=0;
  if(::fstat(descriptor.value,&info)!=0 || !S_ISREG(info.st_mode) || info.st_size<0 ||
      std::uint64_t(info.st_size)>INT32_MAX || !nativeFileTimestamp(info,timestamp))
    throw NativeStorageError();
  const FileInfo candidate{0,static_cast<Int>(info.st_size),
      std::bit_cast<Int>(UnsignedInt(timestamp>>32)),std::bit_cast<Int>(UnsignedInt(timestamp))};
  output=candidate;
  return true;
}
NativeAtomicOutput::NativeAtomicOutput(int directory, std::string target,
                                       NativeStorageIO &io)
    : m_directory(directory), m_target(std::move(target)), m_io(&io) {
  static std::atomic<std::uint64_t> sequence{0};
  for (unsigned attempt = 0; attempt < 64; ++attempt) {
    const auto number = sequence.fetch_add(1, std::memory_order_relaxed);
    if (number == UINT64_MAX)
      throw NativeStorageError();
    std::snprintf(m_temporary.data(), m_temporary.size(), ".zh-write-%ld-%llu",
                  long(::getpid()), static_cast<unsigned long long>(number));
    m_file = m_io->openFile(
        m_directory, m_temporary.data(),
        O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (m_file >= 0)
      return;
    if (errno != EEXIST)
      throw NativeStorageError();
  }
  throw NativeStorageError();
}
NativeAtomicOutput::~NativeAtomicOutput() {
  if (m_file >= 0)
    ::close(m_file);
  if (!m_published && m_temporary[0])
    ::unlinkat(m_directory, m_temporary.data(), 0);
  if (m_directory >= 0)
    ::close(m_directory);
}
std::unique_ptr<NativeScratchOutput> NativeUserStorage::beginScratchWrite(std::string_view relative) const {
  auto output=beginWrite(NativeUserArea::Data,relative);
  return std::unique_ptr<NativeScratchOutput>(new NativeScratchOutput(std::move(output)));
}
NativeScratchOutput::NativeScratchOutput(std::unique_ptr<NativeAtomicOutput> output)
    :m_output(std::move(output)),m_target(m_output->m_target) {
  Descriptor directory(::fcntl(m_output->m_directory,F_DUPFD_CLOEXEC,0));
  struct stat info{};
  if(directory.value<0 || ::fstat(m_output->m_file,&info)!=0 || !S_ISREG(info.st_mode))
    throw NativeStorageError();
  m_device=static_cast<std::uint64_t>(info.st_dev);
  m_inode=static_cast<std::uint64_t>(info.st_ino);
  m_directory=directory.release(); // No fallible work after adopting this fd.
}
NativeScratchOutput::~NativeScratchOutput() {
  if(m_directory<0) return;
  if(m_published) {
    struct stat info{};
    if(::fstatat(m_directory,m_target.c_str(),&info,AT_SYMLINK_NOFOLLOW)==0 &&
        S_ISREG(info.st_mode) && static_cast<std::uint64_t>(info.st_dev)==m_device &&
        static_cast<std::uint64_t>(info.st_ino)==m_inode)
      (void)::unlinkat(m_directory,m_target.c_str(),0);
  }
  ::close(m_directory);
}
Int NativeScratchOutput::write(const void* bytes,Int count) {return m_output->write(bytes,count);}
NativeCommitResult NativeScratchOutput::commit() {
  const auto result=m_output->commit();
  m_published=true;
  return result;
}
Int NativeAtomicOutput::write(const void *bytes, Int count) {
  if (m_poisoned || m_published)
    throw NativeStorageError();
  try {
    if (count < 0 || (count && !bytes))
      throw NativeStorageError();
    if(std::uint64_t(count)>std::uint64_t(std::numeric_limits<off_t>::max())-m_position)
      throw NativeStorageError();
    Int done = 0;
    while (done < count) {
      const Int written = m_io->writeFile(
          m_file, static_cast<const char *>(bytes) + done, count - done);
      if (written < 0 && errno == EINTR)
        continue;
      if (written <= 0 || written > count - done)
        throw NativeStorageError();
      done += written;
      m_position+=std::uint64_t(written);
      m_length=std::max(m_length,m_position);
    }
    return done;
  } catch (...) {
    m_poisoned = true;
    throw;
  }
}
void NativeAtomicOutput::seek(std::uint64_t offset) {
  if(m_poisoned || m_published) throw NativeStorageError();
  try {
    if(offset>m_length || m_io->seekFile(m_file,offset)!=static_cast<std::int64_t>(offset))
      throw NativeStorageError();
    m_position=offset;
  } catch(...) { m_poisoned=true;throw; }
}
NativeCommitResult NativeAtomicOutput::commit() {
  if (m_poisoned || m_published)
    throw NativeStorageError();
  try {
    if (m_io->sync(m_file) != 0)
      throw NativeStorageError();
    const int file = m_file;
    m_file = -1;
    if (::close(file) != 0)
      throw NativeStorageError(); // Linux retires the fd even on error.
    m_io->preparePublish();
    if (::renameat(m_directory, m_temporary.data(), m_directory,
                   m_target.c_str()) != 0)
      throw NativeStorageError();
    m_published = true;
  } catch (...) {
    m_poisoned = true;
    throw;
  }
  // Publication is irreversible. Never report unchanged storage after this
  // point.
  try {
    if (m_io->sync(m_directory) == 0)
      return NativeCommitResult::Durable;
  } catch (...) {
  }
  return NativeCommitResult::PublishedDurabilityUnknown;
}
std::optional<std::vector<unsigned char>>
NativeUserStorage::readCache(std::string_view relative,
                             std::size_t maximumBytes,
                             NativeCacheAdmission &admission) const {
  auto parts = components(relative);
  if (parts.size() != 1)
    throw NativeStorageError();
  Descriptor directory(openDirectory(NativeUserArea::Cache, {}, false));
  if (directory.value < 0) return std::nullopt;
  Descriptor file(m_io->openFile(directory.value, parts[0].c_str(),
                                 O_RDONLY | O_CLOEXEC | O_NOFOLLOW, 0));
  if (file.value < 0)
    return std::nullopt;
  struct stat info{};
  if (::fstat(file.value, &info) != 0 || !S_ISREG(info.st_mode) ||
      info.st_size < 0 || std::uint64_t(info.st_size) > maximumBytes)
    return std::nullopt;
  if (std::uint64_t(info.st_size) < NativeCacheHeaderBytes)
    return std::nullopt;
  const auto exactRead = [&](void *data, std::size_t size) {
    std::size_t done = 0;
    while (done < size) {
      const auto count =
          ::read(file.value, static_cast<unsigned char *>(data) + done,
                 std::min<std::size_t>(size - done, INT32_MAX));
      if (count < 0 && errno == EINTR)
        continue;
      if (count <= 0)
        return false;
      done += static_cast<std::size_t>(count);
    }
    return true;
  };
  std::array<unsigned char, NativeCacheHeaderBytes> header{};
  if (!exactRead(header.data(), header.size()) ||
      !admission.header(header, std::uint64_t(info.st_size)))
    return std::nullopt;
  admission.beginPayload();
  std::array<unsigned char, 16384> scratch{};
  std::size_t remaining =
      static_cast<std::size_t>(info.st_size) - header.size();
  while (remaining) {
    const auto count = std::min(remaining, scratch.size());
    if (!exactRead(scratch.data(), count))
      return std::nullopt;
    admission.payload(std::span<const unsigned char>(scratch).first(count));
    remaining -= count;
  }
  if (!admission.finishPayload())
    return std::nullopt;
  if (::lseek(file.value, off_t(header.size()), SEEK_SET) !=
      off_t(header.size()))
    return std::nullopt;
  std::vector<unsigned char> bytes(static_cast<std::size_t>(info.st_size));
  std::copy(header.begin(), header.end(), bytes.begin());
  std::size_t done = header.size();
  while (done < bytes.size()) {
    const auto count =
        ::read(file.value, bytes.data() + done,
               std::min<std::size_t>(bytes.size() - done, INT32_MAX));
    if (count < 0 && errno == EINTR)
      continue;
    if (count <= 0)
      return std::nullopt;
    done += static_cast<std::size_t>(count);
  }
  unsigned char extra;
  if (::read(file.value, &extra, 1) != 0)
    return std::nullopt;
  return bytes;
}
