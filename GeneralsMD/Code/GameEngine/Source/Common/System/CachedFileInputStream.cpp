/* Command & Conquer Generals Zero Hour(tm), Copyright 2025 Electronic Arts Inc.
 * SPDX-License-Identifier: GPL-3.0-or-later */
#include "Common/AsciiString.h"
#include "Common/FileOwner.h"
#include "Common/FileSystem.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/NativeMapCompression.h"
#include "Common/NativeUserStorage.h"
#include <algorithm>
#include <cstring>

CachedFileInputStream::CachedFileInputStream(const NativeUserStorage *storage)
    : m_size(0), m_buffer(nullptr), m_pos(0), m_storage(storage?storage:TheNativeUserStorage) {}
CachedFileInputStream::~CachedFileInputStream() { close(); }
Bool CachedFileInputStream::open(AsciiString path) {
  if (!TheFileSystem)
    throw ERROR_BAD_ARG;
  std::string physical;
  const char* filename=path.str();
  if(m_storage && (filename[0]=='/' || filename[0]=='\\')) {
    physical=m_storage->mapIdentityPath(filename);
    filename=physical.c_str();
  }
  FileCloseOwner file(TheFileSystem->openFile(filename, File::READ | File::BINARY));
  if (!file)
    return false;
  const Int size = file->size();
  if (size <= 0)
    return false;
  auto candidate = std::make_unique<char[]>(static_cast<std::size_t>(size));
  if (file->read(candidate.get(), size) != size)
    throw ERROR_CORRUPT_FILE_FORMAT;
  NativeMapData data;
  if (m_storage) {
    const auto source = std::span<const unsigned char>(
        reinterpret_cast<const unsigned char *>(candidate.get()),
        static_cast<std::size_t>(size));
    const auto result = loadOrConvertOriginalData(
        *m_storage, source, {1, 1}, INT32_MAX,
        [](std::span<const unsigned char> input) {
          auto bytes = std::make_unique<char[]>(input.size());
          std::memcpy(bytes.get(), input.data(), input.size());
          auto decoded = decodeNativeMapData(
              {std::move(bytes), static_cast<Int>(input.size())});
          const auto *begin =
              reinterpret_cast<const unsigned char *>(decoded.bytes.get());
          return std::vector<unsigned char>(begin, begin + decoded.size);
        });
    auto backing = std::make_unique<char[]>(result.bytes.size());
    std::memcpy(backing.get(), result.bytes.data(), result.bytes.size());
    data = {std::move(backing), static_cast<Int>(result.bytes.size())};
  } else
    data = decodeNativeMapData({std::move(candidate), size});
  delete[] m_buffer;
  m_buffer = data.bytes.release();
  m_size = data.size;
  m_pos = 0;
  return true;
}
void CachedFileInputStream::close() {
  delete[] m_buffer;
  m_buffer = nullptr;
  m_size = m_pos = 0;
}
Int CachedFileInputStream::read(void *destination, Int count) {
  if (count < 0 || (!destination && count))
    throw ERROR_BAD_ARG;
  const Int available = m_size - m_pos;
  count = std::min(count, available);
  if (count)
    std::memcpy(destination, m_buffer + m_pos, static_cast<std::size_t>(count));
  m_pos += count;
  return count;
}
UnsignedInt CachedFileInputStream::tell() {
  return static_cast<UnsignedInt>(m_pos);
}
Bool CachedFileInputStream::absoluteSeek(UnsignedInt position) {
  // The original stream intentionally clamps beyond-end seeks.
  m_pos =
      static_cast<Int>(std::min(position, static_cast<UnsignedInt>(m_size)));
  return true;
}
Bool CachedFileInputStream::eof() { return m_pos == m_size; }
void CachedFileInputStream::rewind() { m_pos = 0; }
