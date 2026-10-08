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
//  (c) 2001-2003 Electronic Arts Inc.
//  //
//																																						//
////////////////////////////////////////////////////////////////////////////////

// Native original chunk output: fixed wire encoding, no destructor-time I/O.
#include "Common/DataChunk.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstring>
#include <exception>
#include <limits>

namespace {
static_assert(sizeof(Int) == 4 && sizeof(UnsignedInt) == 4 &&
              sizeof(Real) == 4);
static_assert(sizeof(DataChunkVersionType) == 2 && sizeof(Byte) == 1);
void requireBytes(const std::vector<unsigned char> &bytes, std::size_t count) {
  if (count >
      static_cast<std::size_t>(std::numeric_limits<Int>::max()) - bytes.size())
    throw ERROR_OUT_OF_MEMORY;
}
void reserveBytes(std::vector<unsigned char> &bytes, std::size_t count) {
  requireBytes(bytes, count);
  if (bytes.size() + count > bytes.capacity())
    bytes.reserve(std::min<std::size_t>(
        std::numeric_limits<Int>::max(),
        std::max(bytes.size() + count,
                 std::max<std::size_t>(32, bytes.capacity() * 2))));
}
void appendWord(std::vector<unsigned char> &bytes, UnsignedInt value,
                unsigned width) {
  reserveBytes(bytes, width);
  for (unsigned n = 0; n < width; ++n)
    bytes.push_back(static_cast<unsigned char>(value >> (8 * n)));
}
void appendBytes(std::vector<unsigned char> &bytes, const void *source,
                 std::size_t count) {
  if (!source && count)
    throw ERROR_BAD_ARG;
  requireBytes(bytes, count);
  if (count) {
    const auto *begin = static_cast<const unsigned char *>(source);
    bytes.insert(bytes.end(), begin, begin + count);
  }
}
struct EncodedStream : OutputStream {
  std::vector<unsigned char> bytes;
  Int write(const void *data, Int count) override {
    if (count < 0)
      throw ERROR_BAD_ARG;
    appendBytes(bytes, data, static_cast<std::size_t>(count));
    return count;
  }
};
UnsignedInt packedName(DataChunkTableOfContents &contents, NameKeyType key,
                       Dict::DataType type) {
  if (!TheNameKeyGenerator || key <= NAMEKEY_INVALID || key > NAMEKEY_MAX)
    throw ERROR_BAD_ARG;
  const auto name = TheNameKeyGenerator->keyToName(key);
  if (name.isEmpty())
    throw ERROR_BAD_ARG;
  const auto id = contents.allocateID(name, 0xffffffu);
  return (id << 8) | static_cast<UnsignedInt>(type);
}
} // namespace
class DataChunkWriteGuard {
  DataChunkOutput &owner;
  int exceptions = std::uncaught_exceptions();

public:
  explicit DataChunkWriteGuard(DataChunkOutput &output) : owner(output) {
    owner.ensureWritable();
  }
  ~DataChunkWriteGuard() {
    if (std::uncaught_exceptions() > exceptions)
      owner.m_poisoned = true;
  }
};
DataChunkOutput::DataChunkOutput(OutputStream *output) : m_pOut(output) {
  if (!output)
    throw ERROR_BAD_ARG;
}
void DataChunkOutput::ensureWritable() const {
  if (m_poisoned || m_finished)
    throw ERROR_BAD_ARG;
}
void DataChunkOutput::finish() {
  if (m_finished)
    return;
  DataChunkWriteGuard transaction(*this);
  if (!m_openSizes.empty() || m_bytes.empty())
    throw ERROR_BAD_ARG;
  EncodedStream candidate;
  m_contents.write(candidate);
  appendBytes(candidate.bytes, m_bytes.data(), m_bytes.size());
  const Int size = static_cast<Int>(candidate.bytes.size());
  // Borrowed sinks must write into their own candidate. Their eventual file
  // rename/publication is not implied to be atomic by this generic API.
  if (m_pOut->write(candidate.bytes.data(), size) != size)
    throw ERROR_CORRUPT_FILE_FORMAT;
  m_finished = true;
}
void DataChunkOutput::openDataChunk(const char *name,
                                    DataChunkVersionType version) {
  DataChunkWriteGuard transaction(*this);
  if (!name || ::strnlen(name, 256) == 0 || ::strnlen(name, 256) > 255)
    throw ERROR_BAD_ARG;
  requireBytes(m_bytes, 10);
  reserveBytes(m_bytes, 10);
  if (m_openSizes.size() == m_openSizes.capacity())
    m_openSizes.reserve(std::max<std::size_t>(8, m_openSizes.capacity() * 2));
  const auto id = m_contents.allocateID(AsciiString(name));
  appendWord(m_bytes, id, 4);
  appendWord(m_bytes, version, 2);
  m_openSizes.push_back(m_bytes.size());
  appendWord(m_bytes, 0, 4);
}
void DataChunkOutput::closeDataChunk() {
  DataChunkWriteGuard transaction(*this);
  if (m_openSizes.empty())
    return;
  const auto offset = m_openSizes.back();
  const auto size = static_cast<UnsignedInt>(m_bytes.size() - offset - 4);
  for (unsigned n = 0; n < 4; ++n)
    m_bytes[offset + n] = static_cast<unsigned char>(size >> (8 * n));
  m_openSizes.pop_back();
}
void DataChunkOutput::writeInt(Int value) {
  DataChunkWriteGuard transaction(*this);
  if (m_openSizes.empty())
    throw ERROR_BAD_ARG;
  appendWord(m_bytes, std::bit_cast<UnsignedInt>(value), 4);
}
void DataChunkOutput::writeReal(Real value) {
  DataChunkWriteGuard transaction(*this);
  if (!std::isfinite(value))
    throw ERROR_BAD_ARG;
  writeInt(std::bit_cast<Int>(value));
}
void DataChunkOutput::writeByte(Byte value) {
  DataChunkWriteGuard transaction(*this);
  if (m_openSizes.empty())
    throw ERROR_BAD_ARG;
  appendWord(m_bytes, static_cast<unsigned char>(value), 1);
}
void DataChunkOutput::writeArrayOfBytes(const char *bytes, Int count) {
  DataChunkWriteGuard transaction(*this);
  if (m_openSizes.empty() || count < 0)
    throw ERROR_BAD_ARG;
  appendBytes(m_bytes, bytes, static_cast<std::size_t>(count));
}
void DataChunkOutput::writeAsciiString(const AsciiString &string) {
  DataChunkWriteGuard transaction(*this);
  if (m_openSizes.empty() || string.getLength() > 65535)
    throw ERROR_BAD_ARG;
  requireBytes(m_bytes, static_cast<std::size_t>(string.getLength()) + 2);
  appendWord(m_bytes, static_cast<UnsignedInt>(string.getLength()), 2);
  appendBytes(m_bytes, string.str(),
              static_cast<std::size_t>(string.getLength()));
}
void DataChunkOutput::writeUnicodeString(const UnicodeString &string) {
  DataChunkWriteGuard transaction(*this);
  if (m_openSizes.empty())
    throw ERROR_BAD_ARG;
  std::vector<unsigned char> encoded;
  UnsignedInt units = 0;
  for (const WideChar *input = string.str(); *input; ++input) {
    const auto scalar = static_cast<UnsignedInt>(*input);
    if (scalar > 0x10ffffu || (scalar >= 0xd800u && scalar <= 0xdfffu))
      throw ERROR_BAD_ARG;
    const unsigned needed = scalar >= 0x10000u ? 2 : 1;
    if (units > 65535u - needed)
      throw ERROR_BAD_ARG;
    if (needed == 2) {
      const auto adjusted = scalar - 0x10000;
      appendWord(encoded, 0xd800u + (adjusted >> 10), 2);
      appendWord(encoded, 0xdc00u + (adjusted & 1023), 2);
    } else
      appendWord(encoded, scalar, 2);
    units += needed;
  }
  requireBytes(m_bytes, encoded.size() + 2);
  appendWord(m_bytes, units, 2);
  appendBytes(m_bytes, encoded.data(), encoded.size());
}
void DataChunkOutput::writeNameKey(NameKeyType key) {
  DataChunkWriteGuard transaction(*this);
  writeInt(
      std::bit_cast<Int>(packedName(m_contents, key, Dict::DICT_ASCIISTRING)));
}
void DataChunkOutput::writeDict(const Dict &dictionary) {
  DataChunkWriteGuard transaction(*this);
  if (m_openSizes.empty())
    throw ERROR_BAD_ARG;
  const Int count = dictionary.getPairCount();
  if (count < 0 || count > Dict::MAX_LEN)
    throw ERROR_BAD_ARG;
  appendWord(m_bytes, static_cast<UnsignedInt>(count), 2);
  for (Int n = 0; n < count; ++n) {
    const auto type = dictionary.getNthType(n);
    if (type < Dict::DICT_BOOL || type > Dict::DICT_UNICODESTRING)
      throw ERROR_BAD_ARG;
    writeInt(std::bit_cast<Int>(
        packedName(m_contents, dictionary.getNthKey(n), type)));
    switch (type) {
    case Dict::DICT_BOOL:
      writeByte(dictionary.getNthBool(n) ? 1 : 0);
      break;
    case Dict::DICT_INT:
      writeInt(dictionary.getNthInt(n));
      break;
    case Dict::DICT_REAL:
      writeReal(dictionary.getNthReal(n));
      break;
    case Dict::DICT_ASCIISTRING:
      writeAsciiString(dictionary.getNthAsciiString(n));
      break;
    case Dict::DICT_UNICODESTRING:
      writeUnicodeString(dictionary.getNthUnicodeString(n));
      break;
    default:
      throw ERROR_BAD_ARG;
    }
  }
}
