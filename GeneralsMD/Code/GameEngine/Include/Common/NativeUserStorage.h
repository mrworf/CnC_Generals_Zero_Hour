// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/MapReaderWriterInfo.h"
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
class FileSystem;
class File;
struct FileInfo;
struct NativeUserPaths {
  std::string data, cache;
  static NativeUserPaths resolve(std::string_view home, std::string_view data,
                                 std::string_view cache);
  static NativeUserPaths fromEnvironment();
};
class NativeStorageError : public std::runtime_error {
public:
  NativeStorageError() : std::runtime_error("User storage is unavailable") {}
};
// Game-owned public POSIX seam. Cleanup always uses direct no-throw native
// calls. A provider must not acquire a descriptor and then throw before
// returning it.
class NativeStorageIO {
public:
  virtual ~NativeStorageIO() = default;
  virtual int openFile(int directory, const char *name, int flags,
                       unsigned mode);
  virtual Int writeFile(int descriptor, const void *bytes, Int count);
  virtual std::int64_t seekFile(int descriptor, std::uint64_t offset);
  virtual int sync(int descriptor);
  virtual void
  preparePublish(); // Fallible before the irreversible native rename.
};
enum class NativeUserArea { Data, Cache };
enum class NativeCommitResult { Durable, PublishedDurabilityUnknown };
constexpr std::size_t NativeCacheHeaderBytes = 84;
class NativeCacheAdmission {
public:
  virtual ~NativeCacheAdmission() = default;
  virtual bool header(std::span<const unsigned char> bytes,
                      std::uint64_t totalBytes) = 0;
  virtual void beginPayload() = 0;
  virtual void payload(std::span<const unsigned char> bytes) = 0;
  virtual bool finishPayload() = 0;
};
class NativeAtomicOutput final : public OutputStream {
  int m_directory = -1, m_file = -1;
  std::string m_target;
  std::array<char, 80> m_temporary{};
  NativeStorageIO *m_io;
  bool m_poisoned = false, m_published = false;
  std::uint64_t m_position = 0, m_length = 0;
  friend class NativeUserStorage;
  friend class NativeScratchOutput;
  NativeAtomicOutput(int directory, std::string target, NativeStorageIO &io);

public:
  ~NativeAtomicOutput();
  NativeAtomicOutput(const NativeAtomicOutput &) = delete;
  NativeAtomicOutput &operator=(const NativeAtomicOutput &) = delete;
  Int write(const void *bytes, Int count) override;
  std::uint64_t position() const noexcept { return m_position; }
  // Backpatch only existing candidate bytes; never create sparse output.
  void seek(std::uint64_t offset);
  NativeCommitResult commit();
};
// Captures exact scratch backing before publication. Destruction uses only
// direct no-throw POSIX cleanup and does not need storage, GameState or globals.
class NativeScratchOutput final : public OutputStream {
  std::unique_ptr<NativeAtomicOutput> m_output;
  std::string m_target;
  int m_directory = -1;
  std::uint64_t m_device = 0, m_inode = 0;
  bool m_published = false;
  friend class NativeUserStorage;
  explicit NativeScratchOutput(std::unique_ptr<NativeAtomicOutput> output);
public:
  ~NativeScratchOutput();
  NativeScratchOutput(const NativeScratchOutput&)=delete;
  NativeScratchOutput& operator=(const NativeScratchOutput&)=delete;
  Int write(const void* bytes,Int count) override;
  NativeCommitResult commit();
};
struct NativeUserEntry {
  std::string relative;
  std::uint64_t bytes = 0;
  bool directory = false;
};
class NativeUserStorage {
  friend class FileSystem;
  NativeUserPaths m_paths;
  const FileSystem *m_assets;
  NativeStorageIO *m_io;
  int openDirectory(NativeUserArea area, std::string_view relative,
                    bool create) const;

public:
  NativeUserStorage(NativeUserPaths paths, const FileSystem &assets,
                    NativeStorageIO *io = nullptr);
  ~NativeUserStorage();
  NativeUserStorage(const NativeUserStorage&)=delete;
  NativeUserStorage& operator=(const NativeUserStorage&)=delete;
  const NativeUserPaths& paths() const noexcept { return m_paths; }
  // Before a parent publishes raw path metadata, prove neither configured area
  // aliases supplied content. Availability remains lazy: an unwritable cache
  // must still permit conversion without persistence. No directories are made.
  void validateRootOwnership() const;
  void ensureDirectory(NativeUserArea area,std::string_view relative) const;
  // Exact original File consumer over protected user backing. The returned
  // read-only pooled view owns its descriptor independently of this storage
  // owner and uses the ordinary source close/delete-on-close convention.
  File* openReadFile(NativeUserArea area, std::string_view relative,Int access=0) const;
  bool getFileInfo(NativeUserArea area,std::string_view relative,FileInfo& output) const;
  // Runtime admission budget for a single complete user-directory transaction.
  static constexpr std::size_t MaximumDirectoryEntries = 1048576;
  // Only explicit configured-root absolute paths may enter the native user
  // namespace. Root spelling is never case-folded or interpreted via CWD.
  std::optional<std::string> relativeDataPath(std::string_view absolute) const;
  // Separate source map-key protocol: resolve ASCII-folded identities only
  // inside this captured owner's Maps/Save subtree, preserving physical root
  // spelling. This never broadens ordinary native absolute-path admission.
  std::string mapIdentityPath(std::string_view identity) const;
  // Offside, no-follow discovery. Missing optional directories are empty;
  // bounds count every encountered entry, including directories/special files.
  std::vector<NativeUserEntry> list(NativeUserArea area,
      std::string_view relative, bool recursive, std::size_t maximumEntries) const;
  NativeCommitResult copy(NativeUserArea area, std::string_view source,
      std::string_view destination) const;
  // Missing files are an idempotent no-op. Never follow a link or remove a
  // directory; callers capture the exact scratch names they acquired.
  bool removeFile(NativeUserArea area, std::string_view relative) const;
  std::unique_ptr<NativeAtomicOutput>
  beginWrite(NativeUserArea area, std::string_view relative) const;
  std::unique_ptr<NativeScratchOutput> beginScratchWrite(std::string_view relative) const;
  // Missing user files return nullopt; malformed paths/I/O/size failures throw.
  // A complete candidate is returned without changing any caller-owned state.
  std::optional<std::vector<unsigned char>>
  readFile(NativeUserArea area, std::string_view relative,
           std::size_t maximumBytes) const;
  std::optional<std::vector<unsigned char>>
  readCache(std::string_view relative, std::size_t maximumBytes,
            NativeCacheAdmission &admission) const;
};
// Borrowed only while the native engine's storage owner is alive.
extern NativeUserStorage* TheNativeUserStorage;
struct NativeCacheResult {
  std::vector<unsigned char> bytes;
  bool hit = false, persisted = false;
};
using NativeConverter =
    std::function<std::vector<unsigned char>(std::span<const unsigned char>)>;
struct NativeConversionKey {
  std::uint32_t kind, version;
};
NativeCacheResult loadOrConvertOriginalData(
    const NativeUserStorage &storage, std::span<const unsigned char> source,
    NativeConversionKey converterKey, std::size_t maximumOutputBytes,
    const NativeConverter &converter);
