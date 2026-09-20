#pragma once

#include "zh/foundation/types.h"

#include <stdexcept>
#include <string>
#include <string_view>

namespace zh::foundation {

class UnicodeError : public std::runtime_error {
public:
    UnicodeError(std::size_t position, const std::string& reason);
    std::size_t position() const noexcept { return position_; }

private:
    std::size_t position_;
};

std::u16string utf8_to_utf16(std::string_view input);
std::string utf16_to_utf8(std::u16string_view input);
std::string ascii_from_utf16(std::u16string_view input);

} // namespace zh::foundation
