// SPDX-License-Identifier: GPL-3.0-or-later
// Deliberately isolated from the original global Byte typedef; zlib's public
// Byte is unsigned. No dependency headers are rewritten or macro-renamed.
#include "ZlibFixture.h"
#include <stdexcept>
#include <zlib.h>
std::string generatedZlibMap(const std::string &bytes, int level) {
  uLongf length = compressBound(bytes.size());
  std::string data(length, '\0');
  if (compress2(reinterpret_cast<Bytef *>(data.data()), &length,
                reinterpret_cast<const Bytef *>(bytes.data()), bytes.size(),
                level) != Z_OK)
    throw std::runtime_error("generated public-zlib fixture");
  data.resize(length);
  std::string result("ZL0\0", 4);
  result[2] = char('0' + level);
  const auto size = static_cast<unsigned>(bytes.size());
  for (unsigned shift : {0, 8, 16, 24})
    result += char(size >> shift);
  return result + data;
}
