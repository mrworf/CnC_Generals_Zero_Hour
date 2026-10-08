// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/DataChunk.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
namespace {
void require(bool value, const char *message) {
  if (!value)
    throw std::runtime_error(message);
}
struct MemoryInput : ChunkInputStream {
  const std::string &bytes;
  UnsignedInt position = 0;
  bool throwRead = false;
  bool rejectSeek = false;
  explicit MemoryInput(const std::string &source) : bytes(source) {}
  Int read(void *output, Int count) override {
    if (count < 0 || (!output && count))
      throw ERROR_BAD_ARG;
    const auto actual = std::min<std::size_t>(static_cast<std::size_t>(count),
                                              bytes.size() - position);
    if (actual)
      std::memcpy(output, bytes.data() + position, actual);
    position += static_cast<UnsignedInt>(actual);
    if (throwRead) {
      throwRead = false;
      throw std::runtime_error("generated provider read fault");
    }
    return static_cast<Int>(actual);
  }
  UnsignedInt tell() override { return position; }
  Bool absoluteSeek(UnsignedInt value) override {
    if (rejectSeek)
      return false;
    position =
        static_cast<UnsignedInt>(std::min<std::size_t>(value, bytes.size()));
    return true;
  }
  Bool eof() override { return position == bytes.size(); }
};
struct MemoryOutput : OutputStream {
  std::string bytes;
  Int write(const void *data, Int size) override {
    bytes.append(static_cast<const char *>(data),
                 static_cast<std::size_t>(size));
    return size;
  }
};
struct Namespace {
  NameKeyGenerator owner;
  NameKeyGenerator *previous;
  Namespace() : previous(TheNameKeyGenerator) {
    owner.init();
    TheNameKeyGenerator = &owner;
  }
  ~Namespace() { TheNameKeyGenerator = previous; }
};
void put(std::string &output, UnsignedInt value, unsigned width = 4) {
  for (unsigned n = 0; n < width; ++n)
    output += char(value >> (8 * n));
}
std::string
toc(const std::vector<std::pair<std::string, UnsignedInt>> &entries) {
  std::string output = "CkMp";
  put(output, static_cast<UnsignedInt>(entries.size()));
  for (const auto &[name, id] : entries) {
    put(output, static_cast<UnsignedInt>(name.size()), 1);
    output += name;
    put(output, id);
  }
  return output;
}
std::string chunk(UnsignedInt id, const std::string &payload) {
  std::string output;
  put(output, id);
  put(output, 7, 2);
  put(output, static_cast<UnsignedInt>(payload.size()));
  return output + payload;
}
void ascii(std::string &output, const std::string &text) {
  put(output, static_cast<UnsignedInt>(text.size()), 2);
  output += text;
}
void unicode(std::string &output, const std::vector<UnsignedShort> &text) {
  put(output, static_cast<UnsignedInt>(text.size()), 2);
  for (auto unit : text)
    put(output, unit, 2);
}
std::string values() {
  std::string data;
  put(data, 5, 2);
  put(data, (11u << 8) | Dict::DICT_BOOL);
  put(data, 2, 1); // Original bool treats every nonzero byte as true.
  put(data, (12u << 8) | Dict::DICT_INT);
  put(data, std::bit_cast<UnsignedInt>(Int(-17)));
  put(data, (13u << 8) | Dict::DICT_REAL);
  put(data, std::bit_cast<UnsignedInt>(Real(-1.25f)));
  put(data, (14u << 8) | Dict::DICT_ASCIISTRING);
  ascii(data, "generated text");
  put(data, (15u << 8) | Dict::DICT_UNICODESTRING);
  unicode(data, {u'\u03a9', u' ', 0xd83d, 0xde80});
  return data;
}
std::string header() {
  return toc({{"Root", 1},
              {"Child", 2},
              {"Boolean", 11},
              {"Integer", 12},
              {"Real", 13},
              {"ASCII", 14},
              {"Unicode", 15}});
}
void warmNames() {
  for (const char *name : {"Boolean", "Integer", "Real", "ASCII", "Unicode"})
    TheNameKeyGenerator->nameToKey(name);
}
void verify(const Dict &data) {
  require(data.getPairCount() == 5 && data.getBool(NAMEKEY("Boolean")) &&
              data.getInt(NAMEKEY("Integer")) == -17 &&
              data.getReal(NAMEKEY("Real")) == -1.25f &&
              data.getAsciiString(NAMEKEY("ASCII")) == "generated text" &&
              data.getUnicodeString(NAMEKEY("Unicode")) ==
                  UnicodeString(L"\u03a9 \U0001f680"),
          "original dictionary wire values by table-name mapping");
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
  require(failed, "generated malformed chunk rejected");
}
struct ParserState {
  Int outer = 0, inner = 0;
  bool fail = false, throwCallback = false;
};
Bool childParser(DataChunkInput &input, DataChunkInfo *info, void *raw) {
  auto &state = *static_cast<ParserState *>(raw);
  require(info->label == "Child" && info->parentLabel == "Root" &&
              info->version == 7,
          "actual nested registered parser scope");
  ++state.inner;
  verify(input.readDict());
  if (state.throwCallback)
    throw std::runtime_error("generated parser fault");
  return !state.fail;
}
Bool rootParser(DataChunkInput &input, DataChunkInfo *, void *raw) {
  auto &state = *static_cast<ParserState *>(raw);
  ++state.outer;
  return input.parse(raw);
}
void functional() {
  Namespace names;
  TheNameKeyGenerator->nameToKey("not-a-table-id");
  warmNames();
  const auto bytes = header() + chunk(1, chunk(2, values()));
  MemoryInput stream(bytes);
  DataChunkInput input(&stream);
  require(input.isValidFileType(), "original CkMp admission");
  input.registerParser("Root", "", rootParser);
  input.registerParser("Child", "Root", childParser);
  ParserState state;
  require(input.parse(&state) && state.outer == 1 && state.inner == 1 &&
              stream.eof(),
          "source recursive parser reaches original dictionary consumer");
  input.reset();
  require(input.parse(&state) && state.outer == 2 && state.inner == 2,
          "same parser owner reset/repeat");
  state.fail = true;
  input.reset();
  require(!input.parse(&state), "parser false is not success");
  state.fail = false;
  input.reset();
  require(input.parse(&state), "parser false corrected retry");
  state.throwCallback = true;
  input.reset();
  rejects([&] { input.parse(&state); });
  state.throwCallback = false;
  input.reset();
  require(input.parse(&state), "callback throw reset/retry");
  rejects([&] { input.registerParser("Child", "Root", nullptr); });
  input.reset();
  require(input.parse(&state),
          "bad parser registration leaves accepted callbacks");
  const auto unknown =
      header() + chunk(1, chunk(2, values())) + chunk(2, "ignored bytes");
  MemoryInput skipped(unknown);
  DataChunkInput noParser(&skipped);
  require(noParser.parse() && skipped.eof(),
          "unregistered complete chunks are skipped");
  const auto emptyChunk = header() + chunk(1, "");
  MemoryInput empty(emptyChunk);
  DataChunkInput emptyInput(&empty);
  require(emptyInput.parse(), "source zero-byte chunk is valid at EOF");
  std::string plain = "legacy-format";
  MemoryInput legacy(plain);
  DataChunkInput probe(&legacy);
  require(!probe.isValidFileType() && !probe.parse(),
          "legacy tag probe is not valid chunk success");
  // Sparse/full-width table IDs are metadata; only packed dictionary subfields
  // are 24-bit.
  const auto sparse = toc({{"Sparse", 0xffffffffu}}) + "x";
  MemoryInput sparseInput(sparse);
  DataChunkTableOfContents table;
  table.read(sparseInput);
  require(table.getID("Sparse") == 0xffffffffu &&
              table.getName(0xffffffffu) == "Sparse",
          "full-width TOC identity");
  rejects([&] { table.allocateID("would-wrap"); });
  require(table.getID("Sparse") == 0xffffffffu,
          "TOC max+1 rejection preserves accepted table");
  MemoryOutput output;
  table.write(output);
  require(output.bytes == sparse.substr(0, sparse.size() - 1),
          "TOC write keeps explicit original little-endian fields");
}
void malformed() {
  Namespace names;
  warmNames();
  const auto complete = header() + chunk(1, values());
  for (std::size_t prefix = 0; prefix < complete.size(); ++prefix) {
    const auto truncated = complete.substr(0, prefix);
    MemoryInput stream(truncated);
    if (prefix == header().size()) {
      DataChunkInput input(&stream);
      require(!input.isValidFileType(), "TOC-only input is not a complete map");
      continue;
    }
    rejects([&] {
      DataChunkInput input(&stream);
      DataChunkVersionType version = 0;
      input.openDataChunk(&version);
    });
  }
  for (const auto &bad : {toc({{"duplicate", 1}, {"duplicate", 2}}) + "x",
                          toc({{"one", 1}, {"two", 1}}) + "x",
                          toc({{"zero", 0}}) + "x", toc({{"", 1}}) + "x"}) {
    MemoryInput stream(bad);
    rejects([&] { DataChunkInput input(&stream); });
    require(stream.tell() == 0, "rejected TOC restores borrowed cursor");
  }
  auto bad = header() + chunk(1, values());
  const auto payload = header().size() + 10;
  for (unsigned type : {5u, 255u}) {
    auto invalid = bad;
    invalid[payload + 2] = char(type);
    MemoryInput stream(invalid);
    DataChunkInput input(&stream);
    DataChunkVersionType version = 0;
    input.openDataChunk(&version);
    const auto position = stream.tell(), left = input.getChunkDataSizeLeft();
    rejects([&] { input.readDict(); });
    require(stream.tell() == position && input.getChunkDataSizeLeft() == left,
            "invalid encoded type read rollback");
  }
  auto invalid = bad;
  for (unsigned n = 0; n < 4; ++n)
    invalid[payload + 2 + n] = char(0xff);
  MemoryInput invalidStream(invalid);
  DataChunkInput invalidInput(&invalidStream);
  DataChunkVersionType version = 0;
  invalidInput.openDataChunk(&version);
  rejects([&] { invalidInput.readDict(); });
  // Unicode and ASCII payload failures include prefix reads and candidate
  // allocation.
  for (const auto &value :
       {std::string("\x01\x00\x00\xdc", 4), std::string("\x01\x00\x00\xd8", 4),
        std::string("\x01\x00\x00\x00", 4)}) {
    const auto badString = header() + chunk(1, value);
    MemoryInput stream(badString);
    DataChunkInput input(&stream);
    input.openDataChunk(&version);
    const auto before = stream.tell();
    rejects([&] { input.readUnicodeString(); });
    require(stream.tell() == before && input.getChunkDataSizeLeft() == 4,
            "Unicode rejection restores full compound read");
  }
  std::string goodAscii;
  ascii(goodAscii, "valid");
  const auto data = header() + chunk(1, goodAscii);
  MemoryInput stream(data);
  DataChunkInput input(&stream);
  input.openDataChunk(&version);
  const auto before = stream.tell();
  stream.throwRead = true;
  rejects([&] { input.readAsciiString(); });
  require(stream.tell() == before && input.getChunkDataSizeLeft() == 7,
          "throwing borrowed provider restores cursor and parent ledger");
  require(input.readAsciiString() == "valid",
          "same input corrected read retry");
  const auto rawBytes = header() + chunk(1, "abcd");
  MemoryInput raw(rawBytes);
  DataChunkInput rawInput(&raw);
  rawInput.openDataChunk(&version);
  std::array<char, 4> destination{'q', 'q', 'q', 'q'};
  const auto rawBefore = raw.tell();
  raw.throwRead = true;
  rejects([&] { rawInput.readArrayOfBytes(destination.data(), 4); });
  require(raw.tell() == rawBefore && rawInput.getChunkDataSizeLeft() == 4 &&
              destination == std::array<char, 4>{'q', 'q', 'q', 'q'},
          "array read failure has no outward bytes/cursor publication");
  rawInput.readArrayOfBytes(destination.data(), 4);
  require(std::string(destination.data(), 4) == "abcd",
          "array corrected retry");
  rawInput.reset();
  rawInput.openDataChunk(&version);
  raw.throwRead = true;
  raw.rejectSeek = true;
  rejects([&] { rawInput.readArrayOfBytes(destination.data(), 4); });
  raw.rejectSeek = false;
  rejects([&] { rawInput.readInt(); });
  rawInput.reset();
  rawInput.openDataChunk(&version);
  rawInput.readArrayOfBytes(destination.data(), 4);
  require(std::string(destination.data(), 4) == "abcd",
          "failed borrowed-cursor repair poisons until verified reset");
}
void faults(bool dictionary) {
  Namespace names;
  warmNames();
  const auto bytes = header() + chunk(1, chunk(2, values()));
  if (!dictionary) {
    const auto warm = toc({{"existing", 100}}) + "x";
    {
      MemoryInput warmInput(bytes);
      DataChunkTableOfContents initialized;
      initialized.read(warmInput);
    } // Public pool initialization before baseline.
    auto *pool = TheMemoryPoolFactory->findMemoryPool("Mapping");
    require(pool, "actual Mapping pool");
    for (std::size_t ordinal = 0; ordinal < 64; ++ordinal) {
      DataChunkTableOfContents owner;
      MemoryInput old(warm);
      owner.read(old);
      MemoryInput stream(bytes);
      const auto baseline = AllocationFault::live();
      const auto units = pool->getUsedBlockCount();
      AllocationFault::arm(ordinal);
      bool failed = false;
      try {
        owner.read(stream);
      } catch (const std::bad_alloc &) {
        failed = true;
      } catch (...) {
        AllocationFault::disarm();
        throw;
      }
      AllocationFault::disarm();
      if (!failed) {
        require(!AllocationFault::triggered() && ordinal > 0 &&
                    owner.getID("existing") == 100 && owner.getID("Root") == 1,
                "TOC exact terminal append");
        std::cout << "TOC fault ordinals [0," << ordinal
                  << ") complete; terminal " << ordinal << '\n';
        return;
      }
      require(AllocationFault::live() == baseline &&
                  pool->getUsedBlockCount() == units && stream.tell() == 0 &&
                  owner.getID("existing") == 100 && owner.getID("Root") == 0,
              "TOC immediate graph/pool/cursor rollback");
      owner.read(stream);
      require(owner.getID("Root") == 1, "same TOC corrected retry");
    }
  } else {
    MemoryInput stream(bytes);
    DataChunkInput owner(&stream);
    DataChunkVersionType version = 0;
    owner.openDataChunk(&version);
    owner.openDataChunk(&version);
    const auto position = stream.tell();
    verify(owner.readDict());
    owner.reset();
    owner.openDataChunk(&version);
    owner.openDataChunk(&version);
    for (std::size_t ordinal = 0; ordinal < 128; ++ordinal) {
      Dict result;
      const auto baseline = AllocationFault::live();
      AllocationFault::arm(ordinal);
      bool failed = false;
      try {
        result = owner.readDict();
      } catch (const std::bad_alloc &) {
        failed = true;
      } catch (...) {
        AllocationFault::disarm();
        throw;
      }
      AllocationFault::disarm();
      if (!failed) {
        verify(result);
        require(!AllocationFault::triggered() && ordinal > 0 &&
                    owner.atEndOfChunk(),
                "dictionary wire exact terminal");
        owner.closeDataChunk();
        require(owner.atEndOfChunk(), "terminal outer ledger settles");
        std::cout << "chunk values fault ordinals [0," << ordinal
                  << ") complete; terminal " << ordinal << '\n';
        return;
      }
      require(AllocationFault::live() == baseline &&
                  stream.tell() == position &&
                  owner.getChunkDataSizeLeft() == values().size(),
              "wire dictionary exact candidate/ledger rollback");
      verify(owner.readDict());
      owner.closeDataChunk();
      require(owner.atEndOfChunk(),
              "rollback/retry preserves every parent ledger");
      owner.reset();
      owner.openDataChunk(&version);
      owner.openDataChunk(&version);
    }
  }
  throw std::runtime_error("chunk fault manifest exceeded");
}
void coldNames(bool allocationFaults) {
  Namespace names;
  names.owner.nameToKey("warm-pool");names.owner.reset();
  auto* pool=TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool");require(pool,"actual cold namespace pool");
  if(!allocationFaults) {
    const auto full=values();
    for(std::size_t prefix=0;prefix<full.size()+1;++prefix) {
      names.owner.reset();require(names.owner.nameToKey("accepted")==1,"cold accepted key");
      auto payload=prefix<full.size()?full.substr(0,prefix):full;
      if(prefix==full.size())payload[7]=char(0xff); // Late invalid second type, after a valid first field.
      const auto bytes=header()+chunk(1,chunk(2,payload));MemoryInput stream(bytes);DataChunkInput input(&stream);
      DataChunkVersionType version=0;input.openDataChunk(&version);input.openDataChunk(&version);
      const auto position=stream.tell(),left=input.getChunkDataSizeLeft();
      const auto live=AllocationFault::live();const auto used=pool->getUsedBlockCount();
      rejects([&]{input.readDict();});
      require(stream.tell()==position && input.getChunkDataSizeLeft()==left && AllocationFault::live()==live &&
        pool->getUsedBlockCount()==used && names.owner.keyToName(NameKeyType(2)).isEmpty(),
        "every cold malformed prefix/type restores cursor, keys and exact backing");
      require(names.owner.nameToKey("after-rejection")==2,"malformed dictionary restores ordinal headroom");
    }
    return;
  }
  const auto bytes=header()+chunk(1,chunk(2,values()));MemoryInput stream(bytes);DataChunkInput input(&stream);
  DataChunkVersionType version=0;
  auto prepare=[&]{input.reset();input.openDataChunk(&version);input.openDataChunk(&version);};
  auto action=[&]{NameKeyTransaction outer(names.owner);auto data=input.readDict();};
  prepare();action();prepare();
  AllocationFault::arm(SIZE_MAX);action();const auto census=AllocationFault::attempts();AllocationFault::disarm();prepare();
  require(census>0 && census<128,"bounded complete cold dictionary census");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal) {
    const auto position=stream.tell(),left=input.getChunkDataSizeLeft();
    const auto live=AllocationFault::live();const auto used=pool->getUsedBlockCount();
    AllocationFault::arm(ordinal);bool failed=false;
    try {action();}catch(const std::bad_alloc&){failed=true;}
    catch(...){AllocationFault::disarm();throw;}
    AllocationFault::disarm();
    require(failed==(ordinal<census) && AllocationFault::triggered()==failed && AllocationFault::live()==live &&
      pool->getUsedBlockCount()==used && names.owner.keyToName(NameKeyType(1)).isEmpty(),
      "cold dictionary complete all-ordinal rollback and terminal under outer owner");
    if(failed)require(stream.tell()==position && input.getChunkDataSizeLeft()==left,"cold failure restores complete read ledger");
    prepare();verify(input.readDict()); // Corrected actual same-reader/same-namespace retry.
    names.owner.reset();prepare();
  }
  require(names.owner.nameToKey("next accepted")==1,"cold retries preserve next ordinal");
  std::cout<<"cold dictionary ordinals [0,"<<census<<"); terminal "<<census<<'\n';
}
} // namespace
int main(int argc, char **argv) {
  bool initialized = false;
  try {
    require(argc == 2, "chunk family required");
    initMemoryManager();
    initialized = true;
    const std::string family = argv[1];
    if (family == "functional")
      for (int repeat = 0; repeat < 3; ++repeat)
        functional();
    else if (family == "malformed")
      malformed();
    else if (family == "fault-toc" || family == "fault-values")
      for (int repeat = 0; repeat < 3; ++repeat)
        faults(family == "fault-values");
    else if(family=="cold-names" || family=="fault-cold-names")
      for(int repeat=0;repeat<3;++repeat)coldNames(family=="fault-cold-names");
    else
      throw std::runtime_error("unknown generated chunk family");
    shutdownMemoryManager();
    initialized = false;
    std::cout << "PASS: original chunk reader " << family
              << " (writer/full startup acceptance pending)\n";
    return 0;
  } catch (ErrorCode) {
    std::cerr << "FAIL: generated original chunk admission/ownership\n";
  } catch (const std::exception &error) {
    std::cerr << "FAIL: generated chunk fixture: " << error.what() << '\n';
  }
  if (initialized)
    shutdownMemoryManager();
  return 1;
}
