#include "zh/foundation/unicode.h"

#include <sstream>

namespace zh::foundation {
namespace {

[[noreturn]] void fail(std::size_t position, const char* reason)
{
    throw UnicodeError(position, reason);
}

void append_utf16(std::u16string& output, UInt32 code_point, std::size_t position)
{
    if (code_point > 0x10ffffU || (code_point >= 0xd800U && code_point <= 0xdfffU)) {
        fail(position, "invalid Unicode scalar value");
    }
    if (code_point <= 0xffffU) {
        output.push_back(static_cast<WideChar>(code_point));
        return;
    }
    code_point -= 0x10000U;
    output.push_back(static_cast<WideChar>(0xd800U + (code_point >> 10U)));
    output.push_back(static_cast<WideChar>(0xdc00U + (code_point & 0x3ffU)));
}

void append_utf8(std::string& output, UInt32 cp)
{
    if (cp <= 0x7fU) {
        output.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7ffU) {
        output.push_back(static_cast<char>(0xc0U | (cp >> 6U)));
        output.push_back(static_cast<char>(0x80U | (cp & 0x3fU)));
    } else if (cp <= 0xffffU) {
        output.push_back(static_cast<char>(0xe0U | (cp >> 12U)));
        output.push_back(static_cast<char>(0x80U | ((cp >> 6U) & 0x3fU)));
        output.push_back(static_cast<char>(0x80U | (cp & 0x3fU)));
    } else {
        output.push_back(static_cast<char>(0xf0U | (cp >> 18U)));
        output.push_back(static_cast<char>(0x80U | ((cp >> 12U) & 0x3fU)));
        output.push_back(static_cast<char>(0x80U | ((cp >> 6U) & 0x3fU)));
        output.push_back(static_cast<char>(0x80U | (cp & 0x3fU)));
    }
}

} // namespace

UnicodeError::UnicodeError(std::size_t position, const std::string& reason)
    : std::runtime_error("Unicode error at byte/code-unit " + std::to_string(position) + ": " + reason),
      position_(position)
{
}

std::u16string utf8_to_utf16(std::string_view input)
{
    std::u16string output;
    output.reserve(input.size());
    for (std::size_t i = 0; i < input.size();) {
        const auto first = static_cast<UInt8>(input[i]);
        UInt32 cp = 0;
        std::size_t length = 0;
        UInt32 minimum = 0;
        if (first <= 0x7fU) {
            cp = first;
            length = 1;
        } else if (first >= 0xc2U && first <= 0xdfU) {
            cp = first & 0x1fU;
            length = 2;
            minimum = 0x80U;
        } else if (first >= 0xe0U && first <= 0xefU) {
            cp = first & 0x0fU;
            length = 3;
            minimum = 0x800U;
        } else if (first >= 0xf0U && first <= 0xf4U) {
            cp = first & 0x07U;
            length = 4;
            minimum = 0x10000U;
        } else {
            fail(i, "invalid UTF-8 leading byte");
        }
        if (length > input.size() - i) {
            fail(i, "truncated UTF-8 sequence");
        }
        for (std::size_t n = 1; n < length; ++n) {
            const auto continuation = static_cast<UInt8>(input[i + n]);
            if ((continuation & 0xc0U) != 0x80U) {
                fail(i + n, "invalid UTF-8 continuation byte");
            }
            cp = (cp << 6U) | (continuation & 0x3fU);
        }
        if (cp < minimum) {
            fail(i, "overlong UTF-8 sequence");
        }
        append_utf16(output, cp, i);
        i += length;
    }
    return output;
}

std::string utf16_to_utf8(std::u16string_view input)
{
    std::string output;
    output.reserve(input.size());
    for (std::size_t i = 0; i < input.size(); ++i) {
        UInt32 cp = input[i];
        if (cp >= 0xd800U && cp <= 0xdbffU) {
            if (i + 1 >= input.size()) {
                fail(i, "truncated UTF-16 surrogate pair");
            }
            const UInt32 low = input[++i];
            if (low < 0xdc00U || low > 0xdfffU) {
                fail(i, "invalid UTF-16 low surrogate");
            }
            cp = 0x10000U + ((cp - 0xd800U) << 10U) + (low - 0xdc00U);
        } else if (cp >= 0xdc00U && cp <= 0xdfffU) {
            fail(i, "unpaired UTF-16 low surrogate");
        }
        append_utf8(output, cp);
    }
    return output;
}

std::string ascii_from_utf16(std::u16string_view input)
{
    std::string output;
    output.reserve(input.size());
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (input[i] > 0x7fU) {
            fail(i, "non-ASCII code unit in ASCII-only field");
        }
        output.push_back(static_cast<char>(input[i]));
    }
    return output;
}

} // namespace zh::foundation
