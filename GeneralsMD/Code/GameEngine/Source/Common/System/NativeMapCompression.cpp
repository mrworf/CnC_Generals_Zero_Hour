/* Command & Conquer Generals Zero Hour(tm), Copyright 2025 Electronic Arts Inc.
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Token protocols derived from original Compression/EAC decoder sources.
 * The original codecs and adopted zlib sources are not modified. */
#include "Common/NativeMapCompression.h"
#include "Common/Errors.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <span>
#include <zlib.h>
namespace {
using Bytes = std::span<const unsigned char>;
[[noreturn]] void corrupt() { throw ERROR_CORRUPT_FILE_FORMAT; }
struct Reader {
  Bytes bytes;
  std::size_t position = 0;
  unsigned byte() {
    if (position == bytes.size())
      corrupt();
    return bytes[position++];
  }
  unsigned big(unsigned width) {
    unsigned value = 0;
    while (width--)
      value = (value << 8) | byte();
    return value;
  }
};
void refpack(Bytes input, std::size_t expected, char *output) {
  Reader reader{input};
  const unsigned type = reader.big(2);
  if (type != 0x10fb && type != 0x11fb && type != 0x90fb && type != 0x91fb)
    corrupt();
  const unsigned width = type & 0x8000 ? 4 : 3;
  if (type & 0x100)
    (void)reader.big(
        width); // Optional preceding size, skipped by original REF_decode.
  if (reader.big(width) != expected)
    corrupt();
  std::size_t produced = 0;
  auto literal = [&](unsigned count) {
    if (count > input.size() - reader.position || count > expected - produced)
      corrupt();
    if (output && count)
      std::memcpy(output + produced, input.data() + reader.position, count);
    reader.position += count;
    produced += count;
  };
  auto reference = [&](unsigned distance, unsigned count) {
    if (distance == 0 || distance > produced || count > expected - produced)
      corrupt();
    // Forward byte copying deliberately allows source-defined overlapping runs.
    if (output)
      for (unsigned n = 0; n < count; ++n)
        output[produced + n] = output[produced + n - distance];
    produced += count;
  };
  for (;;) {
    const unsigned first = reader.byte();
    if (!(first & 0x80)) {
      const unsigned second = reader.byte();
      literal(first & 3);
      reference(((first & 0x60) << 3) + second + 1, ((first & 0x1c) >> 2) + 3);
    } else if (!(first & 0x40)) {
      const unsigned second = reader.byte(), third = reader.byte();
      literal(second >> 6);
      reference(((second & 0x3f) << 8) + third + 1, (first & 0x3f) + 4);
    } else if (!(first & 0x20)) {
      const unsigned second = reader.byte(), third = reader.byte(),
                     fourth = reader.byte();
      literal(first & 3);
      reference(((first & 0x10) >> 4 << 16) + (second << 8) + third + 1,
                ((first & 0x0c) >> 2 << 8) + fourth + 5);
    } else {
      const unsigned count = ((first & 0x1f) << 2) + 4;
      if (count <= 112)
        literal(count);
      else {
        literal(first & 3);
        if (produced != expected)
          corrupt();
        return;
      }
    }
  }
}
void bytePairs(Bytes input, std::size_t expected, char *output) {
  Reader reader{input};
  const auto type = reader.big(2);
  if (type != 0x46fb && type != 0x47fb)
    corrupt();
  if (type == 0x47fb)
    (void)reader.big(3);
  if (reader.big(3) != expected)
    corrupt();
  const auto clue = reader.byte(), nodes = reader.byte();
  struct Node {
    bool branch = false;
    unsigned left = 0, right = 0;
  };
  std::array<Node, 256> graph{};
  for (unsigned n = 0; n < nodes; ++n) {
    const auto id = reader.byte(), left = reader.byte(), right = reader.byte();
    if (id == clue || graph[id].branch || left == clue || right == clue)
      corrupt();
    graph[id] = {true, left, right};
  }
  // Explicit bounded DFS: validate the whole graph, including unused branches.
  // Counts saturate at expected+1 so irrelevant large expansions need no
  // backing.
  std::array<unsigned char, 256> state{};
  std::array<std::size_t, 256> counts{};
  std::array<unsigned, 257> stack{};
  for (unsigned root = 0; root < 256; ++root) {
    std::size_t depth = 1;
    stack[0] = root;
    while (depth) {
      const auto id = stack[depth - 1];
      const auto &node = graph[id];
      if (state[id] == 2) {
        --depth;
        continue;
      }
      if (!node.branch) {
        counts[id] = 1;
        state[id] = 2;
        --depth;
        continue;
      }
      state[id] = 1;
      bool pushed = false;
      for (unsigned child : {node.left, node.right}) {
        if (state[child] == 1)
          corrupt();
        if (state[child] == 0) {
          if (depth == stack.size())
            corrupt();
          stack[depth++] = child;
          pushed = true;
          break;
        }
      }
      if (!pushed) {
        counts[id] =
            std::min(expected + 1, counts[node.left] + counts[node.right]);
        state[id] = 2;
        --depth;
      }
    }
  }
  std::size_t produced = 0;
  for (;;) {
    const auto id = reader.byte();
    if (id == clue) {
      const auto literal = reader.byte();
      if (!literal) {
        if (produced != expected)
          corrupt();
        return;
      }
      if (produced == expected)
        corrupt();
      if (output)
        output[produced] = static_cast<char>(literal);
      ++produced;
      continue;
    }
    if (counts[id] > expected - produced)
      corrupt();
    if (output) {
      std::size_t depth = 1, destination = produced;
      stack[0] = id;
      while (depth) {
        const auto next = stack[--depth];
        if (graph[next].branch) {
          if (depth + 2 > stack.size())
            corrupt();
          stack[depth++] = graph[next].right;
          stack[depth++] = graph[next].left;
        } else
          output[destination++] = static_cast<char>(next);
      }
    }
    produced += counts[id];
  }
}
struct BitReader {
  Bytes bytes;
  std::size_t position = 0;
  unsigned bits(unsigned count) {
    if (count > 32 || position > bytes.size() * 8 ||
        count > bytes.size() * 8 - position)
      corrupt();
    unsigned value = 0;
    while (count--) {
      value = (value << 1) | ((bytes[position / 8] >> (7 - position % 8)) & 1);
      ++position;
    }
    return value;
  }
  unsigned number() {
    unsigned width = 2;
    while (!bits(1)) {
      if (++width > 31)
        corrupt();
    }
    const auto value =
        std::uint64_t(bits(width)) + (std::uint64_t(1) << width) - 4;
    if (value > std::numeric_limits<unsigned>::max())
      corrupt();
    return static_cast<unsigned>(value);
  }
};
void huffman(Bytes input, std::size_t expected, char *output) {
  BitReader reader{input};
  auto type = reader.bits(16);
  const auto variant = type & ~0x8100u;
  if (variant != 0x30fb && variant != 0x32fb && variant != 0x34fb)
    corrupt();
  const unsigned width = type & 0x8000 ? 32 : 24;
  if (type & 0x100)
    (void)reader.bits(width);
  if (reader.bits(width) != expected)
    corrupt();
  const auto clue = reader.bits(8);
  std::array<unsigned, 17> count{}, firstCode{}, firstSymbol{};
  unsigned total = 0, code = 0, maximum = 0;
  for (unsigned length = 1; length <= 16; ++length) {
    code <<= 1;
    firstCode[length] = code;
    firstSymbol[length] = total;
    count[length] = reader.number();
    if (count[length] > 256 - total || count[length] > (1u << length) - code)
      corrupt();
    total += count[length];
    code += count[length];
    if (code == (1u << length)) {
      maximum = length;
      break;
    }
  }
  if (!maximum)
    corrupt();
  std::array<unsigned char, 256> symbols{};
  std::array<bool, 256> used{};
  unsigned previous = 255;
  for (unsigned n = 0; n < total; ++n) {
    const auto delta = reader.number();
    if (delta >= 256 - n)
      corrupt();
    unsigned remaining = delta + 1;
    do {
      previous = (previous + 1) & 255;
      if (!used[previous])
        --remaining;
    } while (remaining);
    symbols[n] = static_cast<unsigned char>(previous);
    used[previous] = true;
  }
  if (!used[clue])
    corrupt(); // Without the clue there can be no bounded EOF escape.
  std::size_t produced = 0;
  unsigned last = 0;
  auto emit = [&](unsigned value, unsigned copies) {
    if (copies > expected - produced)
      corrupt();
    if (output)
      std::memset(output + produced, static_cast<int>(value), copies);
    produced += copies;
    last = value;
  };
  for (;;) {
    unsigned next = 0, symbol = 0;
    bool found = false;
    for (unsigned length = 1; length <= maximum; ++length) {
      next = (next << 1) | reader.bits(1);
      if (next >= firstCode[length] &&
          next - firstCode[length] < count[length]) {
        symbol = symbols[firstSymbol[length] + next - firstCode[length]];
        found = true;
        break;
      }
    }
    if (!found)
      corrupt();
    if (symbol != clue) {
      emit(symbol, 1);
      continue;
    }
    const auto run = reader.number();
    if (run) {
      if (!produced)
        corrupt();
      emit(last, run);
    } else if (reader.bits(1)) {
      if (produced != expected)
        corrupt();
      break;
    } else
      emit(reader.bits(8), 1);
  }
  // Source undelta/acceleration is byte modular, not overflowing signed Int.
  if (output && variant != 0x30fb) {
    unsigned velocity = 0, position = 0;
    for (std::size_t n = 0; n < expected; ++n) {
      velocity = (velocity + static_cast<unsigned char>(output[n])) & 255;
      position = variant == 0x34fb ? (position + velocity) & 255 : velocity;
      output[n] = static_cast<char>(position);
    }
  }
}
struct InflateOwner {
  z_stream stream{};
  InflateOwner() {
    // Public zlib allocator boundary: same array owner on both paths,
    // no exceptions escape its C callbacks, and allocation faults surface
    // as Z_MEM_ERROR for the game-owned transaction to unwind.
    stream.zalloc = [](voidpf, uInt count, uInt size) noexcept -> voidpf {
      if (size && count > std::numeric_limits<std::size_t>::max() / size)
        return nullptr;
      return new (
          std::nothrow) unsigned char[static_cast<std::size_t>(count) * size]();
    };
    stream.zfree = [](voidpf, voidpf pointer) noexcept {
      delete[] static_cast<unsigned char *>(pointer);
    };
    const int status = inflateInit(&stream);
    if (status == Z_MEM_ERROR)
      throw std::bad_alloc();
    if (status != Z_OK)
      corrupt();
  }
  ~InflateOwner() { inflateEnd(&stream); }
  InflateOwner(const InflateOwner &) = delete;
};
void validateZlib(Bytes input, std::size_t expected) {
  InflateOwner owner;
  auto &stream = owner.stream;
  stream.next_in = const_cast<Bytef *>(input.data());
  stream.avail_in = static_cast<uInt>(input.size());
  std::array<unsigned char, 16384> scratch{};
  for (;;) {
    stream.next_out = scratch.data();
    stream.avail_out = static_cast<uInt>(scratch.size());
    const auto beforeIn = stream.total_in, beforeOut = stream.total_out;
    const int status = inflate(&stream, Z_NO_FLUSH);
    if (status == Z_MEM_ERROR)
      throw std::bad_alloc();
    if (stream.total_out > expected)
      corrupt();
    if (status == Z_STREAM_END) {
      if (stream.total_out != expected)
        corrupt();
      return;
    }
    if (status != Z_OK ||
        (beforeIn == stream.total_in && beforeOut == stream.total_out))
      corrupt();
  }
}
void decodeZlib(Bytes input, char *output, std::size_t expected) {
  InflateOwner owner;
  auto &stream = owner.stream;
  stream.next_in = const_cast<Bytef *>(input.data());
  stream.avail_in = static_cast<uInt>(input.size());
  stream.next_out = reinterpret_cast<Bytef *>(output);
  stream.avail_out = static_cast<uInt>(expected);
  const int status = inflate(&stream, Z_FINISH);
  if (status == Z_MEM_ERROR)
    throw std::bad_alloc();
  if (status != Z_STREAM_END || stream.total_out != expected)
    corrupt();
}
} // namespace
NativeMapData decodeNativeMapData(NativeMapData input) {
  if (input.size <= 0 || !input.bytes)
    throw ERROR_BAD_ARG;
  const Bytes bytes(reinterpret_cast<const unsigned char *>(input.bytes.get()),
                    static_cast<std::size_t>(input.size));
  bool ref = false, zlib = false, pairs = false, huff = false, other = false;
  if (bytes.size() >= 4) {
    ref = std::memcmp(bytes.data(), "EAR\0", 4) == 0;
    zlib = bytes[0] == 'Z' && bytes[1] == 'L' && bytes[2] >= '1' &&
           bytes[2] <= '9' && bytes[3] == 0;
    pairs = std::memcmp(bytes.data(), "EAB\0", 4) == 0;
    huff = std::memcmp(bytes.data(), "EAH\0", 4) == 0;
    other = std::memcmp(bytes.data(), "NOX\0", 4) == 0;
  }
  if (!ref && !zlib && !pairs && !huff && !other)
    return input;
  if (bytes.size() < 8)
    corrupt();
  if (other)
    throw ERROR_INVALID_FILE_VERSION; // Pending codecs cannot masquerade as an
                                      // uncompressed map.
  const unsigned expected = unsigned(bytes[4]) | (unsigned(bytes[5]) << 8) |
                            (unsigned(bytes[6]) << 16) |
                            (unsigned(bytes[7]) << 24);
  if (expected == 0 ||
      expected > unsigned(std::numeric_limits<std::int32_t>::max()))
    corrupt();
  const auto compressed = bytes.subspan(8);
  // Complete range/token/output-count validation precedes declared-size
  // allocation.
  if (ref)
    refpack(compressed, expected, nullptr);
  else if (pairs)
    bytePairs(compressed, expected, nullptr);
  else if (huff)
    huffman(compressed, expected, nullptr);
  else
    validateZlib(compressed, expected);
  auto output = std::make_unique<char[]>(expected);
  if (ref)
    refpack(compressed, expected, output.get());
  else if (pairs)
    bytePairs(compressed, expected, output.get());
  else if (huff)
    huffman(compressed, expected, output.get());
  else
    decodeZlib(compressed, output.get(), expected);
  return {std::move(output), static_cast<std::int32_t>(expected)};
}
