// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/FileSystem.h"
#include "Common/GameMemory.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/NativeMapCompression.h"
#include "Common/NativeUserStorage.h"
#include "ZlibFixture.h"
#include <array>
#include <cstring>
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <unistd.h>
#include <vector>
namespace {
void require(bool value, const char *message) {
  if (!value)
    throw std::runtime_error(message);
}
struct Tree {
  std::filesystem::path path;
  Tree() {
    char pattern[] = "/tmp/zh-map-owner-XXXXXX";
    const auto *created = ::mkdtemp(pattern);
    if (!created)
      throw std::runtime_error("generated map fixture root");
    path = created;
  }
  ~Tree() {
    std::error_code ignored;
    std::filesystem::remove_all(path, ignored);
  }
  void write(const char *name, const std::string &bytes) {
    std::ofstream out(path / name, std::ios::binary);
    out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    if (!out)
      throw std::runtime_error("generated map fixture write");
  }
};
struct GlobalFiles {
  FileSystem *prior;
  explicit GlobalFiles(FileSystem &owner) : prior(TheFileSystem) {
    TheFileSystem = &owner;
  }
  ~GlobalFiles() { TheFileSystem = prior; }
};
unsigned descriptors() {
  DIR *dir = ::opendir("/proc/self/fd");
  if (!dir)
    throw std::runtime_error("descriptor baseline");
  unsigned count = 0;
  while (::readdir(dir))
    ++count;
  ::closedir(dir);
  return count;
}
void little(std::string &data, unsigned value) {
  for (unsigned shift : {0, 8, 16, 24})
    data += char(value >> shift);
}
void big(std::string &data, unsigned value, unsigned width) {
  while (width--)
    data += char(value >> (width * 8));
}
std::string pairMap(const std::string &nodes, const std::string &commands,
                    unsigned output, unsigned type = 0x46fb,
                    unsigned clue = 255) {
  std::string result("EAB\0", 4);
  little(result, output);
  big(result, type, 2);
  if (type == 0x47fb)
    big(result, 0, 3);
  big(result, output, 3);
  result += char(clue);
  result += char(nodes.size() / 3);
  return result + nodes + commands;
}
struct BitWriter {
  std::string bytes;
  unsigned used = 0;
  void bits(unsigned value, unsigned count) {
    while (count--) {
      if (!used)
        bytes += char(0);
      bytes.back() = char(static_cast<unsigned char>(bytes.back()) |
                          (((value >> count) & 1) << (7 - used)));
      used = (used + 1) % 8;
    }
  }
  void number(unsigned value) {
    unsigned width = 2;
    while (std::uint64_t(value) >= (std::uint64_t(1) << (width + 1)) - 4)
      ++width;
    bits(1, width - 1);
    bits(value - ((1u << width) - 4), width);
  }
};
struct HuffFixture {
  BitWriter writer;
  std::array<unsigned, 256> code{}, length{};
  HuffFixture(unsigned output, unsigned type,
              const std::vector<unsigned> &counts,
              const std::vector<unsigned> &symbols, unsigned clue = 255) {
    writer.bits(type, 16);
    const unsigned width = type & 0x8000 ? 32 : 24;
    if (type & 0x100)
      writer.bits(0, width);
    writer.bits(output, width);
    writer.bits(clue, 8);
    for (const auto count : counts)
      writer.number(count);
    std::array<bool, 256> used{};
    unsigned previous = 255;
    for (const auto symbol : symbols) {
      unsigned delta = 0;
      for (;;) {
        previous = (previous + 1) % 256;
        if (previous == symbol)
          break;
        if (!used[previous])
          ++delta;
      }
      writer.number(delta);
      used[symbol] = true;
    }
    unsigned next = 0, index = 0;
    for (unsigned n = 0; n < counts.size(); ++n) {
      next <<= 1;
      for (unsigned k = 0; k < counts[n]; ++k) {
        const auto symbol = symbols.at(index++);
        code[symbol] = next++;
        length[symbol] = n + 1;
      }
    }
  }
  void symbol(unsigned value) { writer.bits(code[value], length[value]); }
  void literal(unsigned value) {
    symbol(255);
    writer.number(0);
    writer.bits(value, 9); // Zero EOF bit followed by the explicit byte.
  }
  void run(unsigned count) {
    symbol(255);
    writer.number(count);
  }
  void end() {
    symbol(255);
    writer.number(0);
    writer.bits(1, 1);
  }
  std::string map(unsigned output) const {
    std::string result("EAH\0", 4);
    little(result, output);
    return result + writer.bytes;
  }
};
std::string refMap(const std::string &commands, unsigned output,
                   unsigned type = 0x10fb) {
  std::string result("EAR\0", 4);
  little(result, output);
  big(result, type, 2);
  const unsigned width = type & 0x8000 ? 4 : 3;
  if (type & 0x100)
    big(result, 0, width);
  big(result, output, width);
  return result + commands;
}
std::string literals(const std::string &bytes) {
  std::string commands;
  std::size_t position = 0;
  while (bytes.size() - position >= 4) {
    const auto count =
        std::min<std::size_t>(112, (bytes.size() - position) & ~std::size_t(3));
    commands += char(0xe0 + ((count - 4) >> 2));
    commands.append(bytes, position, count);
    position += count;
  }
  commands += char(0xfc + bytes.size() - position);
  commands.append(bytes, position, std::string::npos);
  return commands;
}
std::string decoded(const std::string &bytes) {
  auto input = std::make_unique<char[]>(bytes.size());
  std::memcpy(input.get(), bytes.data(), bytes.size());
  auto result =
      decodeNativeMapData({std::move(input), static_cast<Int>(bytes.size())});
  return std::string(result.bytes.get(), static_cast<std::size_t>(result.size));
}
void rejects(const std::string &bytes) {
  bool failed = false;
  try {
    (void)decoded(bytes);
  } catch (ErrorCode) {
    failed = true;
  }
  require(failed, "malformed generated compressed stream rejected");
}
void compression() {
  const std::string raw = "CkMp generated uncompressed map data";
  require(decoded(raw) == raw, "uncompressed backing retained");
  for (unsigned type : {0x10fbu, 0x11fbu, 0x90fbu, 0x91fbu})
    for (unsigned length : {1u, 3u, 4u, 111u, 112u, 113u, 1024u}) {
      const std::string output(length, 'x');
      require(decoded(refMap(literals(output), length, type)) == output,
              "all source RefPack header and literal boundaries");
    }
  // Source short/int/very-int forms, three immediate literals and overlapping
  // distance1.
  for (const auto &[token, count] :
       std::vector<std::pair<std::string, unsigned>>{
           {std::string("\x03\x00"
                        "abc"
                        "\xfc",
                        6),
            6},
           {std::string("\x80\xc0\x00"
                        "abc"
                        "\xfc",
                        7),
            7},
           {std::string("\xc3\x00\x00\x00"
                        "abc"
                        "\xfc",
                        8),
            8}}) {
    const std::string expected = "ab" + std::string(count - 2, 'c');
    require(decoded(refMap(token, count)) == expected,
            "source overlapping backreference control forms");
  }
  const std::string initial(131072, 'Q');
  for (const auto &[tail, count] :
       std::vector<std::pair<std::string, unsigned>>{
           {std::string("\x7c\xff\xfc", 3), 10},
           {std::string("\xbf\x3f\xff\xfc", 4), 67},
           {std::string("\xdc\xff\xff\xff\xfc", 5), 1028}}) {
    auto commands = literals(initial);
    commands.pop_back();
    commands += tail;
    require(decoded(refMap(commands,
                           static_cast<unsigned>(initial.size()) + count)) ==
                initial + std::string(count, 'Q'),
            "maximum source reference distance and run");
  }
  const auto valid = refMap(literals(raw), static_cast<unsigned>(raw.size()));
  for (std::size_t n = 4; n < valid.size(); ++n)
    rejects(valid.substr(0, n));
  rejects(refMap(std::string("\x00\x00\xfc", 3), 3)); // before-start reference
  rejects(refMap(std::string("\xfd"
                             "a",
                             2),
                 2)); // premature terminal
  rejects(refMap(std::string("\xff"
                             "abc",
                             4),
                 2)); // terminal output overflow
  auto huge = valid;
  for (unsigned n = 4; n < 8; ++n)
    huge[n] = char(0xff);
  rejects(huge);
  huge = refMap(literals("x"), 0x7fffffffu, 0x90fb);
  rejects(huge); // whole-stream validation before huge allocation
  for (const char *tag : {"NOX", "EAB", "EAH"}) {
    std::string unsupported(tag, 3);
    unsupported += char(0);
    little(unsupported, 1);
    unsupported += 'x';
    rejects(unsupported);
  }
  const std::string zbytes(50000, 'z');
  for (int level = 1; level <= 9; ++level) {
    const auto zip = generatedZlibMap(zbytes, level);
    require(decoded(zip) == zbytes,
            "all original zlib level tags use public decoder");
    for (std::size_t n = 4; n < zip.size(); ++n)
      rejects(zip.substr(0, n));
    auto invalid = zip;
    invalid[4] ^= 1;
    rejects(invalid);
    invalid = zip;
    invalid.back() ^= 1;
    rejects(invalid);
    require(decoded(zip + "trailing") == zbytes,
            "original decoder accepts bytes beyond compressed stream");
  }
}
void legacyCodecs() {
  for (unsigned type : {0x46fbu, 0x47fbu}) {
    const std::string nodes("\x80"
                            "ab"
                            "\x81\x80\x80",
                            6);
    const std::string commands("\x81\xff\x80\x00\xff\x00", 6);
    const std::string result("abab\x80\x00", 6);
    const auto valid = pairMap(nodes, commands, 6, type);
    require(decoded(valid) == result && decoded(valid + "tail") == result,
            "source pair graph, escape, zero literal and optional size");
    for (std::size_t n = 4; n < valid.size(); ++n)
      rejects(valid.substr(0, n));
    rejects(pairMap(nodes, commands, 5, type));
    rejects(pairMap(nodes, commands, 7, type));
  }
  for (const std::string &nodes : {std::string("\x80\x80"
                                               "a",
                                               3), // Self cycle.
                                   std::string("\x80\x81"
                                               "a"
                                               "\x81\x80"
                                               "b",
                                               6),
                                   std::string("\x80\xff"
                                               "a",
                                               3), // Special clue as a child.
                                   std::string("\xff"
                                               "ab",
                                               3), // Overwritten clue.
                                   std::string("\x80"
                                               "ab"
                                               "\x80"
                                               "cd",
                                               6)})
    rejects(pairMap(nodes, std::string("x\xff\x00", 3), 1));
  // Longest graph depth with shared children: exact expansion counts are capped
  // during validation and cannot trigger recursion or declared-size allocation.
  std::string hugeGraph;
  for (unsigned node = 1; node <= 254; ++node) {
    hugeGraph += char(node);
    hugeGraph += char(node - 1);
    hugeGraph += char(node - 1);
  }
  rejects(pairMap(hugeGraph, std::string("\xfe\xff\x00", 3), 1));
  require(decoded(pairMap(hugeGraph, std::string("\0\xff\x00", 3), 1)) ==
              std::string(1, '\0'),
          "unused large acyclic graph does not demand output backing");
  // A single right-deep graph reaches254 branches without exponential output.
  std::string deepGraph;
  for (unsigned node = 1; node <= 254; ++node) {
    deepGraph += char(node);
    deepGraph += char(0);
    deepGraph += char(node - 1);
  }
  require(decoded(pairMap(deepGraph, std::string("\xfe\xff\x00", 3), 255)) ==
              std::string(255, char(0)),
          "bounded maximum-depth pair expansion");
  for (unsigned type : {0x30fbu, 0x31fbu, 0x32fbu, 0x33fbu, 0x34fbu, 0x35fbu,
                        0xb0fbu, 0xb1fbu, 0xb2fbu, 0xb3fbu, 0xb4fbu, 0xb5fbu}) {
    HuffFixture fixture(7, type, {2}, {0, 255});
    fixture.symbol(0);
    fixture.literal(200);
    fixture.run(3);
    fixture.literal(255);
    fixture.symbol(0);
    fixture.end();
    std::string expected("\0\xc8\xc8\xc8\xc8\xff\0", 7);
    unsigned velocity = 0, position = 0;
    const auto variant = type & ~0x8100u;
    if (variant != 0x30fb)
      for (char &value : expected) {
        velocity = (velocity + static_cast<unsigned char>(value)) % 256;
        position = variant == 0x34fb ? (position + velocity) % 256 : velocity;
        value = char(position);
      }
    const auto valid = fixture.map(7);
    require(decoded(valid) == expected && decoded(valid + "tail") == expected,
            "all source Huffman variants, clue run/literal/EOF and undelta");
    for (std::size_t n = 4; n < valid.size(); ++n)
      rejects(valid.substr(0, n));
    auto mismatch = valid;
    mismatch[4] = char(8);
    rejects(mismatch);
  }
  // Canonical lengths1..16 include the old decoder's out-of-array final index.
  std::vector<unsigned> counts(16, 1), symbols;
  counts.back() = 2;
  for (unsigned n = 0; n < 16; ++n)
    symbols.push_back(n);
  symbols.push_back(255);
  HuffFixture deep(16, 0x30fb, counts, symbols);
  std::string expected;
  for (unsigned n = 0; n < 16; ++n) {
    deep.symbol(n);
    expected += char(n);
  }
  deep.end();
  require(decoded(deep.map(16)) == expected, "maximum16-bit canonical codes");
  symbols.clear();
  for (unsigned n = 0; n < 256; ++n)
    symbols.push_back(n);
  HuffFixture full(256, 0x30fb, {0, 0, 0, 0, 0, 0, 0, 256}, symbols);
  expected.clear();
  for (unsigned n = 0; n < 255; ++n) {
    full.symbol(n);
    expected += char(n);
  }
  full.literal(255);
  expected += char(255);
  full.end();
  require(decoded(full.map(256)) == expected,
          "complete256-symbol canonical table");
  HuffFixture wrapped(2, 0x30fb, {1, 2}, {200, 10, 255});
  wrapped.symbol(200);
  wrapped.symbol(10);
  wrapped.end();
  require(decoded(wrapped.map(2)) == std::string("\xc8\x0a", 2),
          "leapfrog ordering wraps among only unused symbols");
  for (unsigned count : {1u, 3u, 4u, 11u, 12u, 27u, 28u, 59u, 60u, 123u, 124u,
                         251u, 252u, 507u, 508u, 65531u, 131067u}) {
    HuffFixture run(count + 1, 0x30fb, {2}, {'x', 255});
    run.symbol('x');
    run.run(count);
    run.end();
    require(decoded(run.map(count + 1)) == std::string(count + 1, 'x'),
            "source encoded integer run boundaries");
  }
  HuffFixture beforeStart(1, 0x30fb, {2}, {'x', 255});
  beforeStart.run(1);
  beforeStart.end();
  rejects(beforeStart.map(1));
  HuffFixture overflow(1, 0x30fb, {2}, {'x', 255});
  overflow.symbol('x');
  overflow.run(1);
  overflow.end();
  rejects(overflow.map(1));
  HuffFixture early(1, 0x30fb, {2}, {'x', 255});
  early.end();
  rejects(early.map(1));
  HuffFixture missingClue(1, 0x30fb, {2}, {'x', 'y'});
  missingClue.symbol('x');
  rejects(missingClue.map(1));
  for (unsigned badCount : {3u, 257u}) {
    BitWriter writer;
    writer.bits(0x30fb, 16);
    writer.bits(1, 24);
    writer.bits(255, 8);
    writer.number(badCount);
    std::string malformed("EAH\0", 4);
    little(malformed, 1);
    rejects(malformed + writer.bytes);
  }
  for (bool badLeap : {false, true}) {
    BitWriter writer;
    writer.bits(0x30fb, 16);
    writer.bits(1, 24);
    writer.bits(255, 8);
    if (badLeap) {
      writer.number(2);
      writer.number(256); // No such ordinal among256 unused symbols.
    } else
      for (unsigned n = 0; n < 16; ++n)
        writer.number(0); // Incomplete tree after the maximum length.
    std::string malformed("EAH\0", 4);
    little(malformed, 1);
    rejects(malformed + writer.bytes);
  }
  auto huge = early.map(1);
  huge[4] = char(0xff);
  huge[5] = char(0xff);
  huge[6] = char(0x7f);
  // Outer/inner disagreement cannot authorize a declared huge allocation.
  rejects(huge);
}
void stream() {
  Tree tree;
  tree.write("raw", "accepted-map");
  tree.write("empty", "");
  tree.write("packed", refMap(literals("decoded-map"), 11));
  tree.write("bad", refMap(std::string("\x00\x00", 2), 3));
  FileSystem files;
  files.mountReadOnly({tree.path.string()});
  GlobalFiles global(files);
  CachedFileInputStream owner;
  require(owner.tell() == 0 && owner.eof(), "fresh stream cursor initialized");
  require(owner.open("raw"), "actual rooted raw map owner");
  char first{};
  require(owner.read(&first, 1) == 1 && first == 'a', "accepted stream cursor");
  for (const char *name : {"empty", "missing", "bad"}) {
    bool failed = false;
    try {
      failed = !owner.open(name);
    } catch (ErrorCode) {
      failed = true;
    }
    require(failed && owner.tell() == 1,
            "failed map reopen preserves accepted backing/cursor");
    owner.rewind();
    char bytes[12]{};
    require(owner.read(bytes, 12) == 12 &&
                std::string(bytes, 12) == "accepted-map",
            "failed reopen retains readable original backing");
    owner.absoluteSeek(1);
  }
  bool failed = false;
  try {
    owner.read(nullptr, 1);
  } catch (ErrorCode) {
    failed = true;
  }
  require(failed && owner.tell() == 1, "null read rejection preserves cursor");
  failed = false;
  try {
    owner.read(&first, -1);
  } catch (ErrorCode) {
    failed = true;
  }
  require(failed && owner.tell() == 1,
          "negative read rejection preserves cursor");
  require(owner.read(nullptr, 0) == 0, "zero-length null read admitted");
  require(owner.absoluteSeek(std::numeric_limits<UnsignedInt>::max()) &&
              owner.tell() == 12 && owner.eof(),
          "source beyond-end seek clamps without narrowing overflow");
  require(owner.open("packed") && owner.tell() == 0,
          "corrected compressed reopen publishes complete candidate");
  char result[11]{};
  require(owner.read(result, std::numeric_limits<Int>::max()) == 11 &&
              std::string(result, 11) == "decoded-map",
          "bounded large read without signed overflow");
  owner.close();
  owner.close();
  require(owner.tell() == 0 && owner.eof(), "repeated stream retirement");
}
void cachedStream() {
  Tree assets, user;
  const std::string content = "decoded original map";
  assets.write("packed", generatedZlibMap(content, 6));
  assets.write("accepted", "accepted-map");
  assets.write("bad", refMap(std::string("\0\0", 2), 3));
  FileSystem files;
  files.mountReadOnly({assets.path.string()});
  GlobalFiles global(files);
  NativeUserStorage storage(
      NativeUserPaths::resolve(user.path.string(),
                               (user.path / "data").string(),
                               (user.path / "cache").string()),
      files);
  CachedFileInputStream owner(&storage);
  const auto check = [&] {
    require(owner.open("packed") && owner.tell() == 0,
            "actual cache-backed rooted map open");
    std::array<char, 20> bytes{};
    require(owner.read(bytes.data(), bytes.size()) == Int(content.size()) &&
                std::string(bytes.data(), content.size()) == content,
            "actual cache-backed decoder bytes");
  };
  check();
  check();
  const auto directory = user.path / "cache/cnc-generals-zero-hour";
  require(std::distance(std::filesystem::directory_iterator(directory), {}) ==
              1,
          "actual map converter persists one content/version owner");
  {
    std::ofstream corrupt(
        std::filesystem::directory_iterator(directory)->path(),
        std::ios::binary | std::ios::trunc);
    corrupt << "invalid generated cache";
  }
  check();
  require(owner.open("accepted"), "accepted actual cache-backed map baseline");
  owner.absoluteSeek(1);
  bool rejected = false;
  try {
    owner.open("bad");
  } catch (ErrorCode) {
    rejected = true;
  }
  require(rejected && owner.tell() == 1,
          "malformed cached reopen preserves accepted cursor");
  owner.rewind();
  char accepted[12]{};
  require(owner.read(accepted, 12) == 12 &&
              std::string(accepted, 12) == "accepted-map",
          "malformed cached reopen preserves accepted bytes");
  check();
  user.write("blocked", "not a directory");
  NativeUserStorage unavailable({"", (user.path / "blocked").string()}, files);
  CachedFileInputStream memoryOnly(&unavailable);
  require(memoryOnly.open("packed"),
          "actual decoder survives unavailable cache");
  std::array<char, 20> fallback{};
  require(memoryOnly.read(fallback.data(), fallback.size()) ==
                  Int(content.size()) &&
              std::string(fallback.data(), content.size()) == content,
          "actual decoded memory fallback");
}
void faults(const std::string &family) {
  Tree tree, user;
  const std::string candidate(50000, 'z');
  tree.write("accepted", "accepted-map");
  tree.write(
      "candidate",
      family == "raw" ? candidate
      : family == "ref"
          ? refMap(literals(candidate), static_cast<unsigned>(candidate.size()))
      : family == "pair" ? pairMap("", candidate + std::string("\xff\0", 2),
                                   static_cast<unsigned>(candidate.size()))
      : family == "huff" ? [&] {
          HuffFixture fixture(candidate.size(), 0x30fb, {2}, {'z', 255});
          fixture.symbol('z');
          fixture.run(candidate.size() - 1);
          fixture.end();
          return fixture.map(candidate.size());
        }()
                         : generatedZlibMap(candidate, 6));
  FileSystem files;
  files.mountReadOnly({tree.path.string()});
  GlobalFiles global(files);
  const bool cached = family == "cached-hit" || family == "cached-miss";
  std::optional<NativeUserStorage> storage;
  if (cached)
    storage.emplace(NativeUserPaths::resolve(user.path.string(),
                                             (user.path / "data").string(),
                                             (user.path / "cache").string()),
                    files);
  CachedFileInputStream owner(storage ? &*storage : nullptr);
  require(owner.open("accepted"), "accepted map fault baseline");
  require(owner.open("candidate"),
          "warm complete provider public initialization");
  require(owner.open("accepted"), "restore accepted baseline");
  std::filesystem::path candidateCache;
  if (cached)
    for (const auto &entry : std::filesystem::directory_iterator(
             user.path / "cache/cnc-generals-zero-hour"))
      if (entry.file_size() == candidate.size() + 84)
        candidateCache = entry.path();
  require(!cached || !candidateCache.empty(),
          "complete actual map cache candidate");
  const AsciiString name("candidate");
  auto *pool = TheMemoryPoolFactory->findMemoryPool("NativeDataFile");
  require(pool, "actual rooted file pool owner");
  if(family=="cached-miss") {
    std::ofstream invalid(candidateCache,std::ios::binary|std::ios::trunc);
    invalid<<"generated invalid cache";
  }
  AllocationFault::arm(SIZE_MAX);
  try {require(owner.open(name),"complete map allocation census");}
  catch(...) {AllocationFault::disarm();throw;}
  const auto census=AllocationFault::attempts();AllocationFault::disarm();
  require(census>0 && census<64,"independent complete map census bound");
  require(owner.open("accepted"),"retire discovery and restore accepted backing");
  for (std::size_t ordinal = 0; ordinal < 64; ++ordinal) {
    if (family == "cached-miss") {
      std::ofstream invalid(candidateCache, std::ios::binary | std::ios::trunc);
      invalid << "generated invalid cache";
    }
    owner.absoluteSeek(1);
    const auto baseline = AllocationFault::live();
    const auto fds = descriptors();
    const auto units = pool->getUsedBlockCount();
    AllocationFault::arm(ordinal);
    bool failed = false;
    try {
      require(owner.open(name), "generated candidate map open");
    } catch (const std::bad_alloc &) {
      failed = true;
    } catch (...) {
      AllocationFault::disarm();
      throw;
    }
    AllocationFault::disarm();
    if (!failed) {
      require(owner.tell() == 0, "map complete candidate publication");
      require(descriptors() == fds && pool->getUsedBlockCount() == units,
              "terminal file owner settles");
      if (AllocationFault::triggered()) {
        require(cached,
                "only optional cache allocation can fall back successfully");
        std::array<char, 16> result{};
        require(owner.read(result.data(), 16) == 16 &&
                    std::string(result.data(), 16) == candidate.substr(0, 16),
                "successful actual map cache fallback bytes");
        require(owner.open("accepted"),
                "restore after accepted optional-cache fault");
        continue;
      }
      require(ordinal == census && AllocationFault::attempts()==census,
              "map exact terminal publication");
      std::cout << "map " << family << " fault ordinals [0," << ordinal
                << ") complete; terminal " << ordinal << '\n';
      return;
    }
    require(AllocationFault::live() == baseline && descriptors() == fds &&
                pool->getUsedBlockCount() == units && owner.tell() == 1,
            "map backing/file/decode immediate rollback");
    owner.rewind();
    std::array<char, 12> before{};
    require(owner.read(before.data(), 12) == 12 &&
                std::string(before.data(), 12) == "accepted-map",
            "map accepted bytes after fault");
    require(owner.open(name) && owner.tell() == 0,
            "same map owner corrected retry");
    std::array<char, 16> result{};
    require(owner.read(result.data(), 16) == 16 &&
                std::string(result.data(), 16) == candidate.substr(0, 16),
            "decoded retry candidate bytes");
    require(owner.open("accepted"),
            "restore same owner for next independent ordinal");
  }
  throw std::runtime_error("map fault manifest exceeded");
}
} // namespace
int main(int argc, char **argv) {
  bool initialized = false;
  try {
    require(argc == 2, "map fixture family required");
    initMemoryManager();
    initialized = true;
    const std::string family = argv[1];
    if (family == "compression")
      compression();
    else if (family == "legacy-codecs")
      legacyCodecs();
    else if (family == "stream")
      for (int repeat = 0; repeat < 3; ++repeat)
        stream();
    else if (family == "cached-stream")
      for (int repeat = 0; repeat < 3; ++repeat)
        cachedStream();
    else if (family.rfind("fault-", 0) == 0)
      for (int repeat = 0; repeat < 3; ++repeat)
        faults(family.substr(6));
    else
      throw std::runtime_error("unknown generated map fixture family");
    shutdownMemoryManager();
    initialized = false;
    std::cout << "PASS: original map supporting owner " << family
              << " (full map/startup acceptance pending)\n";
    return 0;
  } catch (ErrorCode) {
    std::cerr << "FAIL: generated map ownership/admission\n";
  } catch (const std::exception &error) {
    std::cerr << "FAIL: generated map fixture: " << error.what() << '\n';
  }
  if (initialized)
    shutdownMemoryManager();
  return 1;
}
