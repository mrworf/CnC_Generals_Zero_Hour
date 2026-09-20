#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace zh::foundation {

class FormatError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

std::u16string format_utf16(
    std::u16string_view pattern,
    const std::vector<std::u16string_view>& arguments,
    std::size_t capacity_limit);

} // namespace zh::foundation
