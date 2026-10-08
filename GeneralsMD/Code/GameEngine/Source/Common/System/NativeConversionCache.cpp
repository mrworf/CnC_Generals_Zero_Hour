// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/Errors.h"
#include "Common/NativeUserStorage.h"
#include <algorithm>
#include <cstring>
#include <openssl/evp.h>
namespace {
using Digest = std::array<unsigned char, 32>;
Digest digest(std::span<const unsigned char> input) {
  std::array<unsigned char, EVP_MAX_MD_SIZE> output{};
  std::size_t size = 0;
  if (EVP_Q_digest(nullptr, "SHA256", nullptr, input.data(), input.size(),
                   output.data(), &size) != 1 ||
      size != 32)
    throw NativeStorageError();
  Digest result{};
  std::copy_n(output.begin(), result.size(), result.begin());
  return result;
}
void little(unsigned char *output, std::uint64_t value,
            unsigned count) noexcept {
  for (unsigned n = 0; n < count; ++n)
    output[n] = static_cast<unsigned char>(value >> (8 * n));
}
std::uint64_t little(const unsigned char *input, unsigned count) noexcept {
  std::uint64_t value = 0;
  for (unsigned n = 0; n < count; ++n)
    value |= std::uint64_t(input[n]) << (8 * n);
  return value;
}
constexpr std::size_t headerBytes = NativeCacheHeaderBytes;
class CacheAdmission final : public NativeCacheAdmission {
  NativeConversionKey m_key;
  const Digest &m_source;
  std::size_t m_maximum;
  Digest m_expected{};
  std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> m_context{
      nullptr, &EVP_MD_CTX_free};

public:
  CacheAdmission(NativeConversionKey key, const Digest &source,
                 std::size_t maximum)
      : m_key(key), m_source(source), m_maximum(maximum) {}
  bool header(std::span<const unsigned char> bytes,
              std::uint64_t total) override {
    if (bytes.size() != headerBytes ||
        std::memcmp(bytes.data(), "ZHC1", 4) != 0 ||
        little(bytes.data() + 4, 4) != m_key.kind ||
        little(bytes.data() + 8, 4) != m_key.version ||
        !std::equal(m_source.begin(), m_source.end(), bytes.begin() + 12))
      return false;
    const auto count = little(bytes.data() + 44, 8);
    if (total < headerBytes || count > m_maximum ||
        count != total - headerBytes)
      return false;
    std::copy_n(bytes.begin() + 52, m_expected.size(), m_expected.begin());
    return true;
  }
  void beginPayload() override {
    m_context.reset(EVP_MD_CTX_new());
    if (!m_context ||
        EVP_DigestInit_ex2(m_context.get(), EVP_sha256(), nullptr) != 1)
      throw NativeStorageError();
  }
  void payload(std::span<const unsigned char> bytes) override {
    if (EVP_DigestUpdate(m_context.get(), bytes.data(), bytes.size()) != 1)
      throw NativeStorageError();
  }
  bool finishPayload() override {
    std::array<unsigned char, EVP_MAX_MD_SIZE> result{};
    unsigned size = 0;
    if (EVP_DigestFinal_ex(m_context.get(), result.data(), &size) != 1 ||
        size != m_expected.size())
      throw NativeStorageError();
    return std::equal(m_expected.begin(), m_expected.end(), result.begin());
  }
};
} // namespace
NativeCacheResult loadOrConvertOriginalData(
    const NativeUserStorage &storage, std::span<const unsigned char> source,
    NativeConversionKey converterKey, std::size_t maximumOutputBytes,
    const NativeConverter &converter) {
  if (!converter || !converterKey.kind || !converterKey.version ||
      maximumOutputBytes > INT32_MAX)
    throw ERROR_BAD_ARG;
  Digest sourceDigest{};
  std::string filename;
  bool cacheUsable = false;
  try {
    sourceDigest = digest(source);
    std::array<unsigned char, 40> identity{};
    little(identity.data(), converterKey.kind, 4);
    little(identity.data() + 4, converterKey.version, 4);
    std::copy(sourceDigest.begin(), sourceDigest.end(), identity.begin() + 8);
    const auto key = digest(identity);
    filename.reserve(68);
    constexpr char hex[] = "0123456789abcdef";
    for (const auto value : key) {
      filename += hex[value >> 4];
      filename += hex[value & 15];
    }
    filename += ".zhc";
    cacheUsable = true;
    CacheAdmission admission(converterKey, sourceDigest, maximumOutputBytes);
    auto cached = storage.readCache(filename, maximumOutputBytes + headerBytes,
                                    admission);
    if (cached && cached->size() >= headerBytes &&
        std::memcmp(cached->data(), "ZHC1", 4) == 0 &&
        little(cached->data() + 4, 4) == converterKey.kind &&
        little(cached->data() + 8, 4) == converterKey.version &&
        std::equal(sourceDigest.begin(), sourceDigest.end(),
                   cached->begin() + 12)) {
      const auto count = little(cached->data() + 44, 8);
      if (count <= maximumOutputBytes &&
          count == cached->size() - headerBytes) {
        const auto checksum = digest(
            std::span<const unsigned char>(*cached).subspan(headerBytes));
        if (std::equal(checksum.begin(), checksum.end(),
                       cached->begin() + 52)) {
          cached->erase(cached->begin(), cached->begin() + headerBytes);
          return {std::move(*cached), true, true};
        }
      }
    }
  } catch (const std::exception &) {
    // Cache failure is not converter failure. No input-derived diagnostics.
  }
  auto output =
      converter(source); // Exactly one real conversion; errors propagate.
  if (output.size() > maximumOutputBytes)
    throw ERROR_CORRUPT_FILE_FORMAT;
  bool persisted = false;
  if (cacheUsable) {
    try {
      std::array<unsigned char, headerBytes> header{};
      std::memcpy(header.data(), "ZHC1", 4);
      little(header.data() + 4, converterKey.kind, 4);
      little(header.data() + 8, converterKey.version, 4);
      std::copy(sourceDigest.begin(), sourceDigest.end(), header.begin() + 12);
      little(header.data() + 44, output.size(), 8);
      const auto checksum = digest(output);
      std::copy(checksum.begin(), checksum.end(), header.begin() + 52);
      auto candidate = storage.beginWrite(NativeUserArea::Cache, filename);
      candidate->write(header.data(), static_cast<Int>(header.size()));
      candidate->write(output.data(), static_cast<Int>(output.size()));
      persisted = candidate->commit() == NativeCommitResult::Durable;
    } catch (const std::exception &) {
      // Unavailable/failed cache publication preserves the converted memory
      // owner.
    }
  }
  return {std::move(output), false, persisted};
}
