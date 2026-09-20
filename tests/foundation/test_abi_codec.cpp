#include "zh/foundation/byte_codec.h"
#include "zh/foundation/format.h"
#include "zh/foundation/numeric.h"
#include "zh/foundation/unicode.h"

#include <cfenv>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace zh::foundation;

namespace {

void check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

template <typename Exception, typename Function>
void rejects(Function function, const char* message)
{
    try {
        function();
    } catch (const Exception&) {
        return;
    }
    throw std::runtime_error(message);
}

std::vector<UInt8> fixture()
{
    ByteWriter writer(128);
    writer.write_u8(0xa5);
    writer.write_u16_le(0x1234);
    writer.write_u16_be(0x5678);
    writer.write_u32_le(0x89abcdefU);
    writer.write_u32_be(0x10203040U);
    writer.write_u64_le(0x0123456789abcdefULL);
    writer.write_f32_le(-0.0F);
    writer.write_f32_le(1.5F);
    writer.write_utf16le(u"A\U0001f642", 8);
    return writer.bytes();
}

void test_codec()
{
    const auto bytes = fixture();
    const auto hex = bytes_to_hex({bytes.data(), bytes.size()});
    check(hex == "a534125678efcdab8910203040efcdab8967452301000000800000c03f0300000041003dd842de", "fixture bytes changed");
    ByteReader reader({bytes.data(), bytes.size()});
    check(reader.read_u8() == 0xa5, "u8 decode");
    check(reader.read_u16_le() == 0x1234, "u16 LE decode");
    check(reader.read_u16_be() == 0x5678, "u16 BE decode");
    check(reader.read_u32_le() == 0x89abcdefU, "u32 LE decode");
    check(reader.read_u32_be() == 0x10203040U, "u32 BE decode");
    check(reader.read_u64_le() == 0x0123456789abcdefULL, "u64 LE decode");
    check(std::signbit(reader.read_f32_le()), "signed zero decode");
    check(reader.read_f32_le() == 1.5F, "float decode");
    check(reader.read_utf16le(8) == u"A\U0001f642", "UTF-16LE decode");
    check(reader.remaining() == 0, "fixture fully consumed");

    ByteWriter exact(1);
    exact.write_u8(1);
    rejects<CodecError>([&] { exact.write_u8(2); }, "writer overflow accepted");
    check(exact.bytes().size() == 1, "overflow partially mutated writer");
    const UInt8 truncated[] = {1, 2, 3};
    ByteReader short_reader({truncated, sizeof(truncated)});
    rejects<CodecError>([&] { short_reader.read_u32_le(); }, "truncated field accepted");
    check(short_reader.position() == 0, "truncated read advanced cursor");
}

void test_synthetic_big_header()
{
    const UInt8 big_header[] = {
        'B', 'I', 'G', 'F',
        0x40, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x02,
        0x00, 0x00, 0x00, 0x20,
    };
    ByteReader reader({big_header, sizeof(big_header)});
    check(reader.read_bytes(4) == std::vector<UInt8>({'B', 'I', 'G', 'F'}), "BIG identifier");
    check(reader.read_u32_le() == 64, "BIG archive size");
    check(reader.read_u32_be() == 2, "BIG file count");
    check(reader.read_u32_be() == 32, "BIG directory size");
    check(reader.remaining() == 0, "BIG header fully consumed");
    ByteReader truncated({big_header, sizeof(big_header) - 1});
    truncated.read_bytes(12);
    rejects<CodecError>([&] { truncated.read_u32_be(); }, "truncated BIG header accepted");
}

void test_unicode_and_format()
{
    const std::string utf8 = "Latin \xe6\xbc\xa2\xf0\x9f\x99\x82";
    check(utf16_to_utf8(utf8_to_utf16(utf8)) == utf8, "Unicode round trip");
    check(ascii_from_utf16(u"Map_01") == "Map_01", "ASCII conversion");
    rejects<UnicodeError>([] { utf8_to_utf16("\xc0\x80"); }, "overlong UTF-8 accepted");
    rejects<UnicodeError>([] { utf16_to_utf8(std::u16string(1, static_cast<char16_t>(0xd800))); }, "lone surrogate accepted");
    rejects<UnicodeError>([] { ascii_from_utf16(u"caf\u00e9"); }, "lossy ASCII accepted");
    const std::vector<std::u16string_view> args{u"Zero Hour", u"Linux"};
    check(format_utf16(u"{} on {{}} {}", args, 32) == u"Zero Hour on {} Linux", "UTF-16 format");
    rejects<FormatError>([&] { format_utf16(u"{}", args, 4); }, "format overflow accepted");
    rejects<FormatError>([] { format_utf16(u"{", {}, 4); }, "bad format accepted");
}

void test_numeric()
{
    ScopedRoundToNearest rounding;
    check(is_round_to_nearest(), "rounding mode not established");
    check(checked_float_to_i32(2.9F, IntegralRounding::truncate) == 2, "truncate positive");
    check(checked_float_to_i32(-2.9F, IntegralRounding::truncate) == -2, "truncate negative");
    check(checked_float_to_i32(-2.1F, IntegralRounding::floor) == -3, "floor negative");
    check(checked_float_to_i32(-2.9F, IntegralRounding::ceiling) == -2, "ceiling negative");
    check(checked_float_to_i32(2.5F, IntegralRounding::nearest) == 2, "nearest even positive");
    check(checked_float_to_i32(-3.5F, IntegralRounding::nearest) == -4, "nearest even negative");
    rejects<NumericError>([] { checked_float_to_i32(std::numeric_limits<float>::infinity(), IntegralRounding::truncate); }, "infinity accepted");
    rejects<NumericError>([] { checked_float_to_i32(std::numeric_limits<float>::quiet_NaN(), IntegralRounding::truncate); }, "NaN accepted");
    rejects<NumericError>([] { checked_float_to_i32(1.0e30F, IntegralRounding::truncate); }, "overflow accepted");
}

} // namespace

int main(int argc, char** argv)
{
    try {
        if (argc == 2 && std::string(argv[1]) == "--emit-fixture") {
            const auto bytes = fixture();
            std::cout << bytes_to_hex({bytes.data(), bytes.size()}) << '\n';
            return 0;
        }
        check(argc == 1, "unknown test argument");
        test_codec();
        test_synthetic_big_header();
        test_unicode_and_format();
        test_numeric();
        std::cout << "portable ABI and codec tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
