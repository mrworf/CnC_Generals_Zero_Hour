/* Command & Conquer Generals Zero Hour(tm), Copyright 2025 Electronic Arts Inc.
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Actual DataChunk.cpp TOC/input owners, portable wire and guarded lifetimes.
 */
#include "Common/DataChunk.h"
#include <array>
#include <bit>
#include <cmath>
#include <cstring>
#include <exception>
#include <limits>
#include <memory>
#include <vector>
namespace {
[[noreturn]] void corrupt() { throw ERROR_CORRUPT_FILE_FORMAT; }
void exact(ChunkInputStream &source, void *bytes, Int count) {
  if (source.read(bytes, count) != count)
    corrupt();
}
UnsignedInt word(ChunkInputStream &source, unsigned width) {
  std::array<unsigned char, 4> bytes{};
  exact(source, bytes.data(), static_cast<Int>(width));
  UnsignedInt result = 0;
  for (unsigned n = 0; n < width; ++n)
    result |= UnsignedInt(bytes[n]) << (8 * n);
  return result;
}
struct ListOwner {
  Mapping *head = nullptr;
  ~ListOwner() {
    while (head) {
      auto *next = head->next;
      head->deleteInstance();
      head = next;
    }
  }
};
struct CursorGuard {
  ChunkInputStream &source;
  UnsignedInt start;
  int exceptions = std::uncaught_exceptions();
  Bool *poisoned;
  explicit CursorGuard(ChunkInputStream &stream, Bool *failure = nullptr)
      : source(stream), start(stream.tell()), poisoned(failure) {}
  ~CursorGuard() {
    if (std::uncaught_exceptions() > exceptions)
      try {
        if (!source.absoluteSeek(start) || source.tell() != start) {
          if (poisoned)
            *poisoned = true;
        }
      } catch (...) {
        if (poisoned)
          *poisoned = true;
      }
  }
};
void writeExact(OutputStream &output, const void *bytes, Int count) {
  if (output.write(bytes, count) != count)
    throw ERROR_CORRUPT_FILE_FORMAT;
}
void writeWord(OutputStream &output, UnsignedInt value, unsigned width) {
  std::array<unsigned char, 4> bytes{};
  for (unsigned n = 0; n < width; ++n)
    bytes[n] = static_cast<unsigned char>(value >> (8 * n));
  writeExact(output, bytes.data(), static_cast<Int>(width));
}
} // namespace
DataChunkTableOfContents::DataChunkTableOfContents()
    : m_list(nullptr), m_listLength(0), m_nextID(1), m_headerOpened(false) {}
DataChunkTableOfContents::~DataChunkTableOfContents() {
  ListOwner owner;
  owner.head = m_list;
}
Mapping *DataChunkTableOfContents::findMapping(const AsciiString &name) {
  for (auto *entry = m_list; entry; entry = entry->next)
    if (entry->name == name)
      return entry;
  return nullptr;
}
UnsignedInt DataChunkTableOfContents::getID(const AsciiString &name) {
  auto *entry = findMapping(name);
  return entry ? entry->id : 0;
}
AsciiString DataChunkTableOfContents::getName(UnsignedInt id) {
  for (auto *entry = m_list; entry; entry = entry->next)
    if (entry->id == id)
      return entry->name;
  return AsciiString::TheEmptyString;
}
UnsignedInt DataChunkTableOfContents::allocateID(const AsciiString &name,
                                                 UnsignedInt maximum) {
  if (name.isEmpty() || name.getLength() > 255)
    throw ERROR_BAD_ARG;
  if (auto *entry = findMapping(name)) {
    if (entry->id > maximum)
      throw ERROR_OUT_OF_MEMORY;
    return entry->id;
  }
  if (m_nextID > maximum || m_listLength == std::numeric_limits<Int>::max())
    throw ERROR_OUT_OF_MEMORY;
  auto *entry = newInstance(Mapping);
  MemoryPoolObjectHolder holder(entry);
  entry->next = nullptr;
  entry->id = static_cast<UnsignedInt>(m_nextID);
  entry->name = name;
  entry->next = m_list;
  m_list = entry;
  ++m_listLength;
  ++m_nextID;
  holder.release();
  return entry->id;
}
void DataChunkTableOfContents::write(OutputStream &output) {
  // Wire is independent of native Int/wchar/class layouts.
  writeExact(output, "CkMp", 4);
  writeWord(output, static_cast<UnsignedInt>(m_listLength), 4);
  for (auto *entry = m_list; entry; entry = entry->next) {
    writeWord(output, static_cast<UnsignedInt>(entry->name.getLength()), 1);
    writeExact(output, entry->name.str(), entry->name.getLength());
    writeWord(output, entry->id, 4);
  }
}
void DataChunkTableOfContents::read(ChunkInputStream &source) {
  CursorGuard cursor(source);
  std::array<char, 4> tag{};
  exact(source, tag.data(), 4);
  if (std::memcmp(tag.data(), "CkMp", 4) != 0)
    return; // Source legacy format probe, not a valid chunk file.
  const UnsignedInt count = word(source, 4);
  if (count >
      static_cast<UnsignedInt>(std::numeric_limits<Int>::max() - m_listLength))
    corrupt();
  ListOwner candidate;
  UnsignedInt64 next = m_nextID;
  for (UnsignedInt n = 0; n < count; ++n) {
    auto *entry = newInstance(Mapping);
    MemoryPoolObjectHolder holder(entry);
    entry->next = nullptr;
    entry->id = 0;
    const unsigned length = word(source, 1);
    if (length == 0)
      corrupt();
    std::array<char, 256> name{};
    exact(source, name.data(), static_cast<Int>(length));
    if (std::memchr(name.data(), 0, length))
      corrupt();
    entry->name = name.data();
    entry->id = word(source, 4);
    if (entry->id == 0)
      corrupt();
    if (findMapping(entry->name))
      corrupt();
    for (auto *existing = m_list; existing; existing = existing->next)
      if (existing->id == entry->id)
        corrupt();
    for (auto *existing = candidate.head; existing; existing = existing->next)
      if (existing->id == entry->id || existing->name == entry->name)
        corrupt();
    next = std::max(next, UnsignedInt64(entry->id) + 1);
    entry->next = candidate.head;
    candidate.head = entry;
    holder.release();
  }
  const Bool opened = count > 0 && !source.eof();
  if (candidate.head) {
    auto *tail = candidate.head;
    while (tail->next)
      tail = tail->next;
    tail->next = m_list;
    m_list = candidate.head;
    candidate.head = nullptr;
  }
  m_listLength += static_cast<Int>(count);
  m_nextID = next;
  m_headerOpened = opened;
}
class DataChunkReadGuard {
  DataChunkInput &owner;
  InputChunk *top;
  UnsignedInt position;
  UnsignedInt64 consumed;
  int exceptions;
  DataChunkVersionType *version;
  DataChunkVersionType previousVersion;

public:
  explicit DataChunkReadGuard(DataChunkInput &source,
                              DataChunkVersionType *output = nullptr)
      : owner(source), top(source.m_chunkStack),
        position(source.readPosition()), consumed(source.m_consumed),
        exceptions(std::uncaught_exceptions()), version(output),
        previousVersion(output ? *output : 0) {}
  ~DataChunkReadGuard() {
    if (std::uncaught_exceptions() <= exceptions)
      return;
    while (owner.m_chunkStack != top) {
      auto *retired = owner.m_chunkStack;
      owner.m_chunkStack = retired->next;
      retired->deleteInstance();
    }
    const auto delta = owner.m_consumed - consumed;
    for (auto *parent = top; parent; parent = parent->next)
      parent->dataLeft += static_cast<Int>(delta);
    owner.m_consumed = consumed;
    if (version)
      *version = previousVersion;
    try {
      if (!owner.m_file->absoluteSeek(position) ||
          owner.m_file->tell() != position)
        owner.m_readPoisoned = true;
    } catch (...) {
      owner.m_readPoisoned = true;
    }
  }
};
DataChunkInput::DataChunkInput(ChunkInputStream *stream)
    : m_file(stream), m_fileposOfFirstChunk(0), m_parserList(nullptr),
      m_chunkStack(nullptr), m_currentObject(nullptr), m_userData(nullptr) {
  if (!stream)
    throw ERROR_BAD_ARG;
  CursorGuard cursor(*stream);
  m_contents.read(*stream);
  if (stream->tell() >
      static_cast<UnsignedInt>(std::numeric_limits<Int>::max()))
    corrupt();
  m_fileposOfFirstChunk = static_cast<Int>(stream->tell());
}
DataChunkInput::~DataChunkInput() {
  clearChunkStack();
  while (m_parserList) {
    auto *next = m_parserList->next;
    m_parserList->deleteInstance();
    m_parserList = next;
  }
}
void DataChunkInput::registerParser(const AsciiString &label,
                                    const AsciiString &parent,
                                    DataChunkParserPtr parser, void *data) {
  if (label.isEmpty() || !parser)
    throw ERROR_BAD_ARG;
  auto *entry = newInstance(UserParser);
  MemoryPoolObjectHolder holder(entry);
  entry->next = nullptr;
  entry->parser = parser;
  entry->userData = data;
  entry->label = label;
  entry->parentLabel = parent;
  entry->next = m_parserList;
  m_parserList = entry;
  holder.release();
}
void DataChunkInput::clearChunkStack() {
  while (m_chunkStack) {
    auto *next = m_chunkStack->next;
    m_chunkStack->deleteInstance();
    m_chunkStack = next;
  }
}
void DataChunkInput::reset() {
  clearChunkStack();
  if (!m_file->absoluteSeek(static_cast<UnsignedInt>(m_fileposOfFirstChunk)) ||
      m_file->tell() != static_cast<UnsignedInt>(m_fileposOfFirstChunk))
    corrupt();
  m_consumed = 0;
  m_readPoisoned = false;
  m_currentObject = nullptr;
}
Bool DataChunkInput::isValidFileType() { return m_contents.isOpenedForRead(); }
UnsignedInt DataChunkInput::readPosition() {
  if (m_readPoisoned)
    corrupt();
  return m_file->tell();
}
void DataChunkInput::decrementDataLeft(Int count) {
  for (auto *entry = m_chunkStack; entry; entry = entry->next) {
    if (count < 0 || count > entry->dataLeft)
      corrupt();
  }
  for (auto *entry = m_chunkStack; entry; entry = entry->next)
    entry->dataLeft -= count;
  m_consumed += static_cast<UnsignedInt>(count);
}
void DataChunkInput::readRaw(void *bytes, Int count) {
  (void)readPosition();
  if (count < 0 || (!bytes && count))
    throw ERROR_BAD_ARG;
  for (auto *entry = m_chunkStack; entry; entry = entry->next)
    if (count > entry->dataLeft)
      corrupt();
  CursorGuard cursor(*m_file, &m_readPoisoned);
  exact(*m_file, bytes, count);
  decrementDataLeft(count);
}
AsciiString DataChunkInput::openDataChunk(DataChunkVersionType *version) {
  if (!version)
    throw ERROR_BAD_ARG;
  DataChunkReadGuard transaction(*this, version);
  auto *entry = newInstance(InputChunk);
  MemoryPoolObjectHolder holder(entry);
  entry->next = nullptr;
  entry->id = 0;
  entry->version = 0;
  entry->chunkStart = entry->dataSize = entry->dataLeft = 0;
  std::array<unsigned char, 10> header{};
  readRaw(header.data(), 10);
  auto little = [&](unsigned offset, unsigned width) {
    UnsignedInt value = 0;
    for (unsigned n = 0; n < width; ++n)
      value |= UnsignedInt(header[offset + n]) << (8 * n);
    return value;
  };
  entry->id = little(0, 4);
  entry->version = static_cast<DataChunkVersionType>(little(4, 2));
  const auto size = little(6, 4);
  const auto start = m_file->tell();
  if (size > static_cast<UnsignedInt>(std::numeric_limits<Int>::max()) ||
      start > static_cast<UnsignedInt>(std::numeric_limits<Int>::max()) - size)
    corrupt();
  for (auto *parent = m_chunkStack; parent; parent = parent->next)
    if (size > static_cast<UnsignedInt>(parent->dataLeft))
      corrupt();
  // Probe the complete extent, including providers that clamp seeks.
  if (!m_file->absoluteSeek(start + size) || m_file->tell() != start + size)
    corrupt();
  if (!m_file->absoluteSeek(start) || m_file->tell() != start)
    corrupt();
  AsciiString label = m_contents.getName(entry->id);
  if (label.isEmpty())
    corrupt();
  entry->chunkStart = static_cast<Int>(start);
  entry->dataSize = entry->dataLeft = static_cast<Int>(size);
  entry->next = m_chunkStack;
  m_chunkStack = entry;
  holder.release();
  *version = entry->version;
  return label;
}
void DataChunkInput::closeDataChunk() {
  (void)readPosition();
  if (!m_chunkStack)
    return;
  const Int remaining = m_chunkStack->dataLeft;
  const UnsignedInt start = m_file->tell();
  if (start > std::numeric_limits<UnsignedInt>::max() -
                  static_cast<UnsignedInt>(remaining))
    corrupt();
  CursorGuard cursor(*m_file, &m_readPoisoned);
  if (!m_file->absoluteSeek(start + static_cast<UnsignedInt>(remaining)) ||
      m_file->tell() != start + static_cast<UnsignedInt>(remaining))
    corrupt();
  decrementDataLeft(remaining);
  auto *retired = m_chunkStack;
  m_chunkStack = retired->next;
  retired->deleteInstance();
}
Bool DataChunkInput::parse(void *data) {
  (void)readPosition();
  if (!isValidFileType())
    return false;
  const AsciiString parent = m_chunkStack ? m_contents.getName(m_chunkStack->id)
                                          : AsciiString::TheEmptyString;
  while (!atEndOfFile() && (!m_chunkStack || m_chunkStack->dataLeft)) {
    if (m_chunkStack && m_chunkStack->dataLeft < CHUNK_HEADER_BYTES)
      corrupt();
    DataChunkVersionType version = 0;
    const auto label = openDataChunk(&version);
    for (auto *entry = m_parserList; entry; entry = entry->next)
      if (entry->label == label && entry->parentLabel == parent) {
        DataChunkInfo info{label, parent, version,
                           static_cast<Int>(getChunkDataSize())};
        // Preserve original parse-call userData; the registered field was not
        // dispatched.
        if (!entry->parser(*this, &info, data))
          return false;
        break;
      }
    closeDataChunk();
  }
  if (m_chunkStack && m_chunkStack->dataLeft)
    corrupt();
  return true;
}
AsciiString DataChunkInput::getChunkLabel() {
  return m_chunkStack ? m_contents.getName(m_chunkStack->id)
                      : AsciiString::TheEmptyString;
}
DataChunkVersionType DataChunkInput::getChunkVersion() {
  return m_chunkStack ? m_chunkStack->version : 0;
}
UnsignedInt DataChunkInput::getChunkDataSize() {
  return m_chunkStack ? static_cast<UnsignedInt>(m_chunkStack->dataSize) : 0;
}
UnsignedInt DataChunkInput::getChunkDataSizeLeft() {
  return m_chunkStack ? static_cast<UnsignedInt>(m_chunkStack->dataLeft) : 0;
}
Bool DataChunkInput::atEndOfChunk() {
  return !m_chunkStack || m_chunkStack->dataLeft == 0;
}
Int DataChunkInput::readInt() {
  std::array<unsigned char, 4> bytes{};
  readRaw(bytes.data(), 4);
  UnsignedInt value = 0;
  for (unsigned n = 0; n < 4; ++n)
    value |= UnsignedInt(bytes[n]) << (8 * n);
  return std::bit_cast<Int>(value);
}
Real DataChunkInput::readReal() {
  DataChunkReadGuard transaction(*this);
  const auto value = std::bit_cast<Real>(readInt());
  if (!std::isfinite(value))
    corrupt();
  return value;
}
Byte DataChunkInput::readByte() {
  Byte value = 0;
  readRaw(&value, 1);
  return value;
}
void DataChunkInput::readArrayOfBytes(char *destination, Int count) {
  if (count < 0 || (!destination && count))
    throw ERROR_BAD_ARG;
  if (!count)
    return;
  for (auto *entry = m_chunkStack; entry; entry = entry->next)
    if (count > entry->dataLeft)
      corrupt();
  auto candidate = std::make_unique<char[]>(static_cast<std::size_t>(count));
  readRaw(candidate.get(), count);
  std::memcpy(destination, candidate.get(), static_cast<std::size_t>(count));
}
AsciiString DataChunkInput::readAsciiString() {
  DataChunkReadGuard transaction(*this);
  std::array<unsigned char, 2> prefix{};
  readRaw(prefix.data(), 2);
  const unsigned length = unsigned(prefix[0]) | (unsigned(prefix[1]) << 8);
  std::vector<char> bytes(length + 1, 0);
  readRaw(bytes.data(), static_cast<Int>(length));
  if (std::memchr(bytes.data(), 0, length))
    corrupt();
  return AsciiString(bytes.data());
}
UnicodeString DataChunkInput::readUnicodeString() {
  DataChunkReadGuard transaction(*this);
  std::array<unsigned char, 2> prefix{};
  readRaw(prefix.data(), 2);
  const unsigned length = unsigned(prefix[0]) | (unsigned(prefix[1]) << 8);
  std::vector<unsigned char> bytes(length * 2);
  readRaw(bytes.data(), static_cast<Int>(bytes.size()));
  std::vector<WideChar> text;
  text.reserve(length + 1);
  for (unsigned n = 0; n < length; ++n) {
    unsigned scalar =
        unsigned(bytes[n * 2]) | (unsigned(bytes[n * 2 + 1]) << 8);
    if (scalar == 0)
      corrupt();
    if (scalar >= 0xd800 && scalar <= 0xdbff) {
      if (++n == length)
        corrupt();
      const unsigned low =
          unsigned(bytes[n * 2]) | (unsigned(bytes[n * 2 + 1]) << 8);
      if (low < 0xdc00 || low > 0xdfff)
        corrupt();
      scalar = 0x10000 + ((scalar - 0xd800) << 10) + (low - 0xdc00);
    } else if (scalar >= 0xdc00 && scalar <= 0xdfff)
      corrupt();
    text.push_back(static_cast<WideChar>(scalar));
  }
  text.push_back(0);
  return UnicodeString(text.data());
}
NameKeyType DataChunkInput::readNameKey() {
  DataChunkReadGuard transaction(*this);
  const auto packed = std::bit_cast<UnsignedInt>(readInt());
  if ((packed & 255) != Dict::DICT_ASCIISTRING || !TheNameKeyGenerator)
    corrupt();
  const auto name = m_contents.getName(packed >> 8);
  if (name.isEmpty())
    corrupt();
  return TheNameKeyGenerator->nameToKey(name);
}
Dict DataChunkInput::readDict() {
  DataChunkReadGuard transaction(*this);
  NameKeyGenerator* names=TheNameKeyGenerator;
  if(!names) corrupt();
  NameKeyTransaction keys(*names);
  std::array<unsigned char, 2> prefix{};
  readRaw(prefix.data(), 2);
  const unsigned count = unsigned(prefix[0]) | (unsigned(prefix[1]) << 8);
  if (count > Dict::MAX_LEN)
    corrupt();
  Dict candidate(static_cast<Int>(count));
  for (unsigned n = 0; n < count; ++n) {
    const auto packed = std::bit_cast<UnsignedInt>(readInt());
    const unsigned type = packed & 255;
    if (type > Dict::DICT_UNICODESTRING)
      corrupt();
    const auto name = m_contents.getName(packed >> 8);
    if (name.isEmpty())
      corrupt();
    const auto key = names->nameToKey(name);
    switch (type) {
    case Dict::DICT_BOOL:
      candidate.setBool(key, readByte() != 0);
      break;
    case Dict::DICT_INT:
      candidate.setInt(key, readInt());
      break;
    case Dict::DICT_REAL:
      candidate.setReal(key, readReal());
      break;
    case Dict::DICT_ASCIISTRING:
      candidate.setAsciiString(key, readAsciiString());
      break;
    case Dict::DICT_UNICODESTRING:
      candidate.setUnicodeString(key, readUnicodeString());
      break;
    default:
      corrupt();
    }
  }
  keys.commit();
  return candidate; // Dict's shared_ptr copy is noexcept; all fields admitted.
}
