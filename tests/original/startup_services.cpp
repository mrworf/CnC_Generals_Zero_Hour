// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/FileOwner.h"
#include "Common/FileSystem.h"
#include "Common/GameMemory.h"
#include "Common/NativeMemoryProfiles.h"
#include "Common/file.h"
#include "GameClient/LanguageFilter.h"
#include <array>
#include <bit>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
namespace {
void require(bool value, const char *message) {
  if (!value)
    throw std::runtime_error(message);
}
template <class Action> void rejects(Action action) {
  bool rejected = false;
  try {
    action();
  } catch (ErrorCode) {
    rejected = true;
  }
  require(rejected, "generated service input rejected");
}
struct Tree {
  std::filesystem::path path;
  Tree() {
    char pattern[] = "/tmp/zh-startup-services-XXXXXX";
    const auto *created = ::mkdtemp(pattern);
    if (!created)
      throw std::runtime_error("generated service root");
    path = created;
  }
  ~Tree() {
    std::error_code ignored;
    std::filesystem::remove_all(path, ignored);
  }
  void write(const char *name, const std::string &bytes) {
    const auto target = path / name;
    std::filesystem::create_directories(target.parent_path());
    std::ofstream output(target, std::ios::binary);
    output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    require(bool(output), "generated service write");
  }
};
unsigned descriptors() {
  DIR *directory = ::opendir("/proc/self/fd");
  require(directory, "service descriptor baseline");
  unsigned count = 0;
  while (::readdir(directory))
    ++count;
  ::closedir(directory);
  return count;
}
void poolCounts(Int initial, Int overflow) {
  Int actualInitial = 0, actualOverflow = 0;
  userMemoryAdjustPoolSize("GameMessage", actualInitial, actualOverflow);
  require(actualInitial == initial && actualOverflow == overflow,
          "accepted source memory profile counts");
}
void profiles() {
  Tree tree;
  tree.write("Data/INI/MemoryPools.ini",
             "; source profile\nUnknown 999999999999 1\n"
             "gamemessage 5 9\nGameMessage +13 +17x\nMusicTrack 0 "
             "-1\nNameKeyBucketPool 12x 99\n");
  FileSystem files;
  files.mountReadOnly({tree.path.string()});
  loadOriginalMemoryProfile(files);
  poolCounts(16, 20);
  Int initial = 0, overflow = 0;
  userMemoryAdjustPoolSize("MusicTrack", initial, overflow);
  require(initial == 4 && overflow == 4, "source profile minimum rounding");
  initial = 0;
  overflow = 0;
  userMemoryAdjustPoolSize("NameKeyBucketPool", initial, overflow);
  require(initial == 9000 && overflow == 1024,
          "malformed row does not override");
  auto *live = TheMemoryPoolFactory->createMemoryPool("GameMessage", 16, 0, 0);
  require(live->getTotalBlockCount() == 16,
          "profile reaches original pool factory");
  void *unit = live->allocateBlock("fixture");
  tree.write("Data/INI/MemoryPools.ini", "GameMessage 7 11\n");
  loadOriginalMemoryProfile(files);
  poolCounts(8, 12);
  require(
      TheMemoryPoolFactory->createMemoryPool("GameMessage", 16, 0, 0) == live &&
          live->getTotalBlockCount() == 16 && live->getUsedBlockCount() == 1,
      "profile does not resize or withdraw accepted live pools");
  live->freeBlock(unit);
  initial = 2;
  overflow = 3;
  userMemoryAdjustPoolSize("GameMessage", initial, overflow);
  require(initial == 2 && overflow == 3,
          "explicit positive class sizes retained");
  for (const char *invalid :
       {"GameMessage 2147483647 4\n", "GameMessage 999999999999999 4\n",
        "MusicTrack 4 4\nGameMessage 4 2147483647\n"}) {
    tree.write("Data/INI/MemoryPools.ini", invalid);
    rejects([&] { loadOriginalMemoryProfile(files); });
    poolCounts(8, 12);
    initial = 0;
    overflow = 0;
    userMemoryAdjustPoolSize("MusicTrack", initial, overflow);
    require(initial == 32 && overflow == 32,
            "late-invalid entire profile rolls back");
  }
  Tree missing;
  FileSystem empty;
  empty.mountReadOnly({missing.path.string()});
  loadOriginalMemoryProfile(empty);
  poolCounts(2048, 32);
  tree.write("Data/INI/MemoryPools.ini", "GameMessage 5 9\n");
  loadOriginalMemoryProfile(files);
  poolCounts(8, 12);
}
void profileFaults() {
  Tree tree;
  tree.write("Data/INI/MemoryPools.ini", "GameMessage 5 9\n");
  FileSystem files;
  files.mountReadOnly({tree.path.string()});
  loadOriginalMemoryProfile(
      files); // Initialize actual file pool before baseline.
  userMemoryLoadPoolProfile("GameMessage 32 32\n");
  auto *pool = TheMemoryPoolFactory->findMemoryPool("NativeDataFile");
  require(pool, "rooted profile file pool");
  for (std::size_t ordinal = 0; ordinal < 64; ++ordinal) {
    const auto baseline = AllocationFault::live();
    const auto fds = descriptors();
    const auto units = pool->getUsedBlockCount();
    bool failed = false;
    AllocationFault::arm(ordinal);
    try {
      loadOriginalMemoryProfile(files);
    } catch (const std::bad_alloc &) {
      failed = true;
    } catch (...) {
      AllocationFault::disarm();
      throw;
    }
    AllocationFault::disarm();
    require(descriptors() == fds && pool->getUsedBlockCount() == units,
            "profile acquisition units settle");
    if (!failed) {
      require(!AllocationFault::triggered() && ordinal == 7, "profile exact terminal");
      poolCounts(8, 12);
      std::cout << "profile fault ordinals [0," << ordinal << "); terminal "
                << ordinal << '\n';
      return;
    }
    require(AllocationFault::live() == baseline,
            "profile immediate backing rollback");
    poolCounts(32, 32);
    loadOriginalMemoryProfile(files);
    poolCounts(8, 12);
    userMemoryLoadPoolProfile("GameMessage 32 32\n");
  }
  throw std::runtime_error("profile fault manifest exceeded");
}
std::string words() {
  std::string output;
  for (const auto &word : {std::u16string(u"bad"), std::u16string(u"😀bad")}) {
    for (char16_t unit : word) {
      const auto encoded = unsigned(unit) ^ 0x5555;
      output += char(encoded);
      output += char(encoded >> 8);
    }
    output += char(0x20);
    output += char(0);
  }
  return output;
}
struct GlobalFiles {
  FileSystem *prior;
  GlobalFiles(FileSystem &owner) : prior(TheFileSystem) {
    TheFileSystem = &owner;
  }
  ~GlobalFiles() { TheFileSystem = prior; }
};
void filtering(bool faults) {
  Tree tree;
  tree.write("langdata.dat", words());
  FileSystem files;
  files.mountReadOnly({tree.path.string()});
  GlobalFiles global(files);
  LanguageFilter filter;
  filter.init();
  const UnicodeString original(L"😀bad!bad;😀bad 😀good,b4d 😀bad");
  const wchar_t *expected = L"*****!***;***** 😀good,*** *****";
  UnicodeString line(original);
  if (!faults) {
    filter.filterLine(line);
    require(line.compare(expected) == 0,
            "source UTF16 replacement widths and delimiters");
    line = L"b-ad\t😀good\n😀bad";
    filter.filterLine(line);
    require(line.compare(L"****\t😀good\n*****") == 0,
            "mask original token width before normalization");
    const wchar_t invalid[] = {wchar_t(0xd800), L'x', 0};
    line = invalid;
    rejects([&] { filter.filterLine(line); });
    require(line.compare(invalid) == 0,
            "invalid scalar preserves accepted line");
    return;
  }
  filter.filterLine(line);
  line = original; // Warm all public services.
  for (std::size_t ordinal = 0; ordinal < 256; ++ordinal) {
    const auto baseline = AllocationFault::live();
    bool failed = false;
    AllocationFault::arm(ordinal);
    try {
      filter.filterLine(line);
    } catch (const std::bad_alloc &) {
      failed = true;
    } catch (...) {
      AllocationFault::disarm();
      throw;
    }
    AllocationFault::disarm();
    if (!failed) {
      require(!AllocationFault::triggered() && ordinal == 71 && line.compare(expected) == 0,
              "filter exact terminal publication");
      std::cout << "filter fault ordinals [0," << ordinal << "); terminal "
                << ordinal << '\n';
      return;
    }
    require(AllocationFault::live() == baseline && line == original,
            "filter immediate line/backing preservation");
    filter.filterLine(line);
    require(line.compare(expected) == 0, "same line corrected retry");
    line = original;
  }
  throw std::runtime_error("filter fault manifest exceeded");
}
void word(std::string &output, unsigned value) {
  for (unsigned shift : {24, 16, 8, 0})
    output += char(value >> shift);
}
void timestamps() {
  Tree tree;
  tree.write("loose", "abc");
  const std::string name = "archived";
  const auto offset = 16u + 9u + unsigned(name.size());
  std::string archive = "BIGF";
  word(archive, 0);
  word(archive, 1);
  word(archive, offset);
  word(archive, offset);
  word(archive, 5);
  archive += name;
  archive += char(0);
  archive += "value";
  tree.write("generated.big", archive);
  const auto stamp = [&](const char *name, time_t seconds, long nanos) {
    const auto file = tree.path / name;
    const timespec times[2] = {{seconds, nanos}, {seconds, nanos}};
    require(::utimensat(AT_FDCWD, file.c_str(), times, 0) == 0,
            "generated exact timestamp");
  };
  stamp("loose", 0, 123456789);
  stamp("generated.big", 1, 999999999);
  FileSystem files;
  files.mountReadOnly({tree.path.string()});
  const auto expect = [&](const char *name, Int size, std::uint64_t time) {
    FileInfo info{};
    require(files.getFileInfo(name, &info), "actual metadata owner");
    const auto actual =
        (std::uint64_t(std::bit_cast<UnsignedInt>(info.timestampHigh)) << 32) |
        std::bit_cast<UnsignedInt>(info.timestampLow);
    require(info.sizeHigh == 0 && info.sizeLow == size && actual == time,
            "exact source FILETIME100ns timestamp and logical size");
  };
  constexpr std::uint64_t epoch = 116444736000000000ull;
  expect("loose", 3, epoch + 1234567);
  expect("archived", 5, epoch + 19999999);
  // Update without remount: original loose files expose current physical size.
  tree.write("loose", "longer");
  stamp("loose", -1, 999999999);
  expect("loose", 6, epoch - 1);
  FileCloseOwner file(files.openFile("loose"));
  require(file && file->size() == 6, "loose open agrees with current metadata");
  FileInfo accepted{7, 8, 9, 10}, original = accepted;
  require(!files.getFileInfo("missing", &accepted) &&
              std::memcmp(&accepted, &original, sizeof(accepted)) == 0,
          "missing metadata preserves caller state");
  std::filesystem::remove(tree.path / "loose");
  require(!files.getFileInfo("loose", &accepted) &&
              std::memcmp(&accepted, &original, sizeof(accepted)) == 0,
          "retired physical metadata preserves caller state");
  require(!files.getFileInfo("archived", nullptr),
          "null metadata output rejected");
}
} // namespace
int main(int argc, char **argv) {
  try {
    require(argc == 2, "startup service family required");
    const std::string family = argv[1];
    for (int repeat = 0; repeat < 3; ++repeat) {
      initMemoryManager();
      try {
        poolCounts(2048,
                   32); // Previous process overrides must not survive restart.
        if (family == "profiles")
          profiles();
        else if (family == "profile-faults")
          profileFaults();
        else if (family == "filter")
          filtering(false);
        else if (family == "filter-faults")
          filtering(true);
        else if (family == "timestamps")
          timestamps();
        else
          throw std::runtime_error("unknown startup service family");
      } catch (...) {
        shutdownMemoryManager();
        throw;
      }
      shutdownMemoryManager();
    }
    std::cout << "PASS: original startup service " << family
              << " (GameLogic pending)\n";
    return 0;
  } catch (ErrorCode) {
    std::cerr << "FAIL: generated startup service admission\n";
  } catch (const std::exception &error) {
    std::cerr << "FAIL: " << error.what() << '\n';
  }
  return 1;
}
