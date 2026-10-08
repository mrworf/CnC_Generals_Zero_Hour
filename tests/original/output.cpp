// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/DataChunk.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
namespace {
void require(bool value, const char *message) {
  if (!value)
    throw std::runtime_error(message);
}
template <class F> void rejects(F action) {
  bool failed = false;
  try {
    action();
  } catch (ErrorCode) {
    failed = true;
  } catch (const std::runtime_error &) {
    failed = true;
  }
  require(failed, "generated output admission rejected");
}
struct Sink : OutputStream {
  std::string bytes = "accepted candidate output sentinel";
  unsigned calls = 0;
  bool fail = false, shortWrite = false;
  Int write(const void *data, Int count) override {
    ++calls;
    if (fail)
      throw std::runtime_error("generated output callback fault");
    if (shortWrite)
      return count - 1;
    std::string candidate(static_cast<const char *>(data),
                          static_cast<std::size_t>(count));
    bytes.swap(candidate);
    return count;
  }
};
struct Input : ChunkInputStream {
  const std::string &bytes;
  UnsignedInt position = 0;
  explicit Input(const std::string &source) : bytes(source) {}
  Int read(void *output, Int count) override {
    if (count < 0 || (!output && count))
      throw ERROR_BAD_ARG;
    const auto actual = std::min<std::size_t>(static_cast<std::size_t>(count),
                                              bytes.size() - position);
    if (actual)
      std::memcpy(output, bytes.data() + position, actual);
    position += static_cast<UnsignedInt>(actual);
    return static_cast<Int>(actual);
  }
  UnsignedInt tell() override { return position; }
  Bool absoluteSeek(UnsignedInt value) override {
    position =
        static_cast<UnsignedInt>(std::min<std::size_t>(value, bytes.size()));
    return true;
  }
  Bool eof() override { return position == bytes.size(); }
};
struct Namespace {
  NameKeyGenerator names;
  NameKeyGenerator *previous;
  Namespace() : previous(TheNameKeyGenerator) {
    names.init();
    TheNameKeyGenerator = &names;
    for (const char *name : {"Boolean", "Integer", "Real", "ASCII", "Unicode"})
      names.nameToKey(name);
  }
  ~Namespace() { TheNameKeyGenerator = previous; }
};
Dict data() {
  Dict result;
  result.setBool(NAMEKEY("Boolean"), true);
  result.setInt(NAMEKEY("Integer"), -17);
  result.setReal(NAMEKEY("Real"), -.5f);
  result.setAsciiString(NAMEKEY("ASCII"), AsciiString("A"));
  result.setUnicodeString(NAMEKEY("Unicode"),
                          UnicodeString(L"\u03a9\U0001f680"));
  return result;
}
void put(std::string &output, UnsignedInt value, unsigned width = 4) {
  for (unsigned n = 0; n < width; ++n)
    output += char(value >> (8 * n));
}
std::string
toc(const std::vector<std::pair<std::string, UnsignedInt>> &entries) {
  std::string result = "CkMp";
  put(result, static_cast<UnsignedInt>(entries.size()));
  for (const auto &[name, id] : entries) {
    put(result, static_cast<UnsignedInt>(name.size()), 1);
    result += name;
    put(result, id);
  }
  return result;
}
std::string chunk(UnsignedInt id, const std::string &payload) {
  std::string result;
  put(result, id);
  put(result, 7, 2);
  put(result, static_cast<UnsignedInt>(payload.size()));
  return result + payload;
}
void author(DataChunkOutput &writer, const Dict &dictionary) {
  writer.openDataChunk("Root", 7);
  writer.writeInt(-17);
  writer.writeReal(-.5f);
  writer.writeByte(static_cast<Byte>(0xfe));
  writer.writeAsciiString("A");
  writer.writeUnicodeString(UnicodeString(L"\u03a9\U0001f680"));
  writer.writeNameKey(NAMEKEY("Unicode"));
  writer.writeDict(dictionary);
  writer.openDataChunk("Child", 7);
  writer.writeByte(0x55);
  writer.closeDataChunk();
  writer.closeDataChunk();
}
std::string oracle() {
  std::string body;
  put(body, 0xffffffefu);
  put(body, 0xbf000000u);
  put(body, 0xfe, 1);
  put(body, 1, 2);
  body += 'A';
  put(body, 3, 2);
  put(body, 0x03a9, 2);
  put(body, 0xd83d, 2);
  put(body, 0xde80, 2);
  put(body, (2u << 8) | 3);
  put(body, 5, 2);
  put(body, (3u << 8) | 0);
  put(body, 1, 1);
  put(body, (4u << 8) | 1);
  put(body, 0xffffffefu);
  put(body, (5u << 8) | 2);
  put(body, 0xbf000000u);
  put(body, (6u << 8) | 3);
  put(body, 1, 2);
  body += 'A';
  put(body, (2u << 8) | 4);
  put(body, 3, 2);
  put(body, 0x03a9, 2);
  put(body, 0xd83d, 2);
  put(body, 0xde80, 2);
  body += chunk(7, std::string(1, 0x55));
  return toc({{"Child", 7},
              {"ASCII", 6},
              {"Real", 5},
              {"Integer", 4},
              {"Boolean", 3},
              {"Unicode", 2},
              {"Root", 1}}) +
         chunk(1, body);
}
void verify(const std::string &bytes) {
  require(bytes == oracle(), "fixed independent original wire oracle");
  Input stream(bytes);
  DataChunkInput input(&stream);
  DataChunkVersionType version = 0;
  require(input.openDataChunk(&version) == "Root" && version == 7,
          "original reader reaches output root");
  require(input.readInt() == -17 && input.readReal() == -.5f &&
              static_cast<unsigned char>(input.readByte()) == 0xfe,
          "fixed integer/float/byte wire roundtrip");
  require(input.readAsciiString() == "A" &&
              input.readUnicodeString() == UnicodeString(L"\u03a9\U0001f680") &&
              input.readNameKey() == NAMEKEY("Unicode"),
          "explicit ASCII/UTF16/table-name roundtrip");
  const auto dictionary = input.readDict();
  require(dictionary.getPairCount() == 5 &&
              dictionary.getBool(NAMEKEY("Boolean")) &&
              dictionary.getInt(NAMEKEY("Integer")) == -17 &&
              dictionary.getReal(NAMEKEY("Real")) == -.5f &&
              dictionary.getAsciiString(NAMEKEY("ASCII")) == "A" &&
              dictionary.getUnicodeString(NAMEKEY("Unicode")) ==
                  UnicodeString(L"\u03a9\U0001f680"),
          "every original dictionary writer/reader kind");
  require(input.openDataChunk(&version) == "Child" &&
              input.readByte() == 0x55 && input.atEndOfChunk(),
          "nested size patching");
  input.closeDataChunk();
  require(input.atEndOfChunk(),
          "parent size includes complete child header/payload");
  input.closeDataChunk();
  require(stream.eof(), "exact published extent");
}
void functional() {
  Namespace names;
  const auto dictionary = data();
  Sink sink;
  {
    DataChunkOutput writer(&sink);
    author(writer, dictionary);
    require(sink.calls == 0, "pre-finish candidate has no sink callbacks");
    writer.finish();
    writer.finish();
    require(sink.calls == 1, "single explicit idempotent publication");
    rejects([&] { writer.writeInt(1); });
  }
  require(sink.calls == 1, "destructor never calls accepted sink");
  verify(sink.bytes);
  Sink abandoned;
  {
    DataChunkOutput writer(&abandoned);
    writer.openDataChunk("Root", 7);
    writer.writeInt(1);
  }
  require(abandoned.calls == 0,
          "abandoned/unbalanced candidate destruction does not write");
  Sink deep;
  {
    DataChunkOutput writer(&deep);
    for (int n = 0; n < 2000; ++n)
      writer.openDataChunk("Root", 7);
    for (int n = 0; n < 2000; ++n)
      writer.closeDataChunk();
    writer.finish();
  }
  Input stream(deep.bytes);
  DataChunkInput input(&stream);
  DataChunkVersionType version = 0;
  for (int n = 0; n < 2000; ++n)
    require(input.openDataChunk(&version) == "Root",
            "deep bounded output chunk hierarchy");
  for (int n = 0; n < 2000; ++n)
    input.closeDataChunk();
  require(stream.eof(), "deep frame closes exact parent extents");
}
void rejection() {
  Namespace names;
  const auto dictionary = data();
  for (int fault = 0; fault < 9; ++fault) {
    Sink sink;
    DataChunkOutput writer(&sink);
    author(writer, dictionary);
    const auto before = sink.bytes;
    switch (fault) {
    case 0:
      rejects(
          [&] { writer.writeReal(std::numeric_limits<Real>::quiet_NaN()); });
      break;
    case 1:
      rejects([&] { writer.openDataChunk(nullptr, 7); });
      break;
    case 2:
      rejects([&] { writer.openDataChunk("", 7); });
      break;
    case 3: {
      const std::string name(256, 'n');
      rejects([&] { writer.openDataChunk(name.c_str(), 7); });
      break;
    }
    case 4:
      writer.openDataChunk("Root", 7);
      rejects([&] { writer.writeArrayOfBytes(nullptr, 1); });
      break;
    case 5:
      writer.openDataChunk("Root", 7);
      rejects([&] { writer.writeArrayOfBytes("x", -1); });
      break;
    case 6:
      writer.openDataChunk("Root", 7);
      rejects([&] {
        writer.writeArrayOfBytes("x", std::numeric_limits<Int>::max());
      });
      break;
    case 7:
      writer.openDataChunk("Root", 7);
      rejects([&] { writer.writeNameKey(NAMEKEY_INVALID); });
      break;
    case 8:
      writer.openDataChunk("Root", 7);
      rejects([&] { writer.finish(); });
      break;
    }
    rejects([&] { writer.finish(); });
    require(sink.calls == 0 && sink.bytes == before,
            "late rejection poisons otherwise commit-ready owner before sink");
  }
  for (bool shortWrite : {false, true}) {
    Sink sink;
    const auto before = sink.bytes;
    {
      DataChunkOutput writer(&sink);
      author(writer, dictionary);
      sink.fail = !shortWrite;
      sink.shortWrite = shortWrite;
      rejects([&] { writer.finish(); });
      sink.fail = sink.shortWrite = false;
      rejects([&] { writer.finish(); });
      require(
          sink.calls == 1 && sink.bytes == before,
          "callback failure poisons candidate and cannot duplicate publish");
    }
    require(sink.calls == 1, "failed-writer destructor has no callbacks");
    {
      DataChunkOutput retry(&sink);
      author(retry, dictionary);
      retry.finish();
    }
    verify(sink.bytes);
  }
}
struct SeedWriter : DataChunkOutput {
  using DataChunkOutput::DataChunkOutput;
  void seed(const std::string &bytes) {
    Input stream(bytes);
    m_contents.read(stream);
  }
};
void boundaries() {
  Namespace names;
  const std::string longASCII(AsciiString::MAX_LEN - 1, 'a');
  const AsciiString ascii(longASCII.c_str());
  const std::wstring maxUnits(UnicodeString::MAX_LEN - 1,
                              static_cast<WideChar>(0x1f680));
  const UnicodeString wide(maxUnits.c_str());
  const std::string longName(255, 'n');
  Sink sink;
  {
    DataChunkOutput writer(&sink);
    writer.openDataChunk(longName.c_str(), 7);
    writer.writeAsciiString(ascii);
    writer.writeUnicodeString(wide);
    writer.closeDataChunk();
    writer.finish();
  }
  Input stream(sink.bytes);
  DataChunkInput input(&stream);
  DataChunkVersionType version = 0;
  require(input.openDataChunk(&version) == longName.c_str() &&
              input.readAsciiString() == ascii &&
              input.readUnicodeString() == wide,
          "exact name and reachable native string maxima with explicit "
          "UTF16-unit counts");
  // A UInt16 wire count is not proof that the native string API can provide
  // 65535 scalars/bytes. Exercise its real owner boundary without enlarging it.
  rejects([&] {
    AsciiString tooLong(std::string(AsciiString::MAX_LEN, 'x').c_str());
  });
  rejects([&] {
    UnicodeString tooWide(
        std::wstring(UnicodeString::MAX_LEN, static_cast<WideChar>(0x1f680))
            .c_str());
  });
  const WideChar invalid[]{static_cast<WideChar>(0xd800), 0};
  const UnicodeString invalidScalar(invalid);
  {
    Sink rejected;
    DataChunkOutput writer(&rejected);
    writer.openDataChunk("Root", 7);
    writer.writeInt(1);
    writer.closeDataChunk();
    writer.openDataChunk("Root", 7);
    rejects([&] { writer.writeUnicodeString(invalidScalar); });
    rejects([&] { writer.finish(); });
    require(rejected.calls == 0,
            "string unit max+1/invalid scalar no publication");
  }
  Sink packed;
  {
    SeedWriter writer(&packed);
    writer.seed(toc({{"Root", 1}, {"Unicode", 0xffffffu}}) + "x");
    writer.openDataChunk("Root", 7);
    writer.writeNameKey(NAMEKEY("Unicode"));
    writer.closeDataChunk();
    writer.finish();
  }
  Input packedStream(packed.bytes);
  DataChunkInput packedInput(&packedStream);
  packedInput.openDataChunk(&version);
  require(packedInput.readNameKey() == NAMEKEY("Unicode"),
          "exact packed table ID maximum survives signed transport");
  for (bool existing : {false, true}) {
    Sink rejected;
    SeedWriter writer(&rejected);
    writer.seed(toc({{"Root", 1}, {"Unicode", 0x1000000u}}) + "x");
    writer.openDataChunk("Root", 7);
    auto *pool = TheMemoryPoolFactory->findMemoryPool("Mapping");
    const auto units = pool->getUsedBlockCount();
    rejects([&] {
      writer.writeNameKey(NAMEKEY(existing ? "Unicode" : "Boolean"));
    });
    require(pool->getUsedBlockCount() == units,
            "packed existing/new-ID headroom rejected before reservation");
    rejects([&] { writer.finish(); });
    require(rejected.calls == 0, "unrepresentable packed table ID cannot wrap");
  }
}
void faults() {
  Namespace names;
  const auto dictionary = data();
  {
    Sink warm;
    DataChunkOutput writer(&warm);
    author(writer, dictionary);
    writer.finish();
  }
  auto *mappings = TheMemoryPoolFactory->findMemoryPool("Mapping");
  require(mappings, "source mapping pool initialized");
  for (std::size_t ordinal = 0; ordinal < 128; ++ordinal) {
    Sink sink;
    const auto accepted = sink.bytes;
    const auto baseline = AllocationFault::live();
    const auto units = mappings->getUsedBlockCount();
    bool failed = false;
    AllocationFault::arm(ordinal);
    try {
      DataChunkOutput writer(&sink);
      author(writer, dictionary);
      writer.finish();
    } catch (const std::bad_alloc &) {
      failed = true;
    } catch (...) {
      AllocationFault::disarm();
      throw;
    }
    AllocationFault::disarm();
    if (!failed) {
      require(!AllocationFault::triggered() && ordinal > 0 &&
                  mappings->getUsedBlockCount() == units,
              "writer exact terminal ownership");
      verify(sink.bytes);
      std::cout << "chunk output fault ordinals [0," << ordinal
                << ") complete; terminal " << ordinal << '\n';
      return;
    }
    require(AllocationFault::live() == baseline &&
                mappings->getUsedBlockCount() == units &&
                sink.bytes == accepted,
            "writer candidate/backing/symbol/output rollback after failure");
    {
      DataChunkOutput retry(&sink);
      author(retry, dictionary);
      retry.finish();
    }
    verify(sink.bytes);
  }
  throw std::runtime_error("writer fault manifest exceeded");
}
} // namespace
int main(int argc, char **argv) {
  bool initialized = false;
  try {
    require(argc == 2, "output family required");
    initMemoryManager();
    initialized = true;
    const std::string family = argv[1];
    if (family == "functional")
      for (int repeat = 0; repeat < 3; ++repeat)
        functional();
    else if (family == "rejection")
      rejection();
    else if (family == "boundaries")
      boundaries();
    else if (family == "faults")
      for (int repeat = 0; repeat < 3; ++repeat)
        faults();
    else
      throw std::runtime_error("unknown generated writer family");
    shutdownMemoryManager();
    initialized = false;
    std::cout << "PASS: original native chunk output " << family
              << " (file publication/full startup pending)\n";
    return 0;
  } catch (ErrorCode) {
    std::cerr << "FAIL: generated output ownership/admission\n";
  } catch (const std::exception &error) {
    std::cerr << "FAIL: generated output fixture: " << error.what() << '\n';
  }
  if (initialized)
    shutdownMemoryManager();
  return 1;
}
