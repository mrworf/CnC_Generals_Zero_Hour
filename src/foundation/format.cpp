#include "zh/foundation/format.h"

namespace zh::foundation {

std::u16string format_utf16(
    std::u16string_view pattern,
    const std::vector<std::u16string_view>& arguments,
    std::size_t capacity_limit)
{
    std::u16string output;
    std::size_t argument = 0;
    for (std::size_t i = 0; i < pattern.size(); ++i) {
        if (pattern[i] == u'{' && i + 1 < pattern.size() && pattern[i + 1] == u'{') {
            if (output.size() == capacity_limit) throw FormatError("formatted UTF-16 output exceeds capacity");
            output.push_back(u'{');
            ++i;
        } else if (pattern[i] == u'}' && i + 1 < pattern.size() && pattern[i + 1] == u'}') {
            if (output.size() == capacity_limit) throw FormatError("formatted UTF-16 output exceeds capacity");
            output.push_back(u'}');
            ++i;
        } else if (pattern[i] == u'{' && i + 1 < pattern.size() && pattern[i + 1] == u'}') {
            if (argument >= arguments.size()) throw FormatError("not enough UTF-16 format arguments");
            if (arguments[argument].size() > capacity_limit - output.size()) {
                throw FormatError("formatted UTF-16 output exceeds capacity");
            }
            output.append(arguments[argument++]);
            ++i;
        } else {
            if (pattern[i] == u'{' || pattern[i] == u'}') throw FormatError("unmatched UTF-16 format brace");
            if (output.size() == capacity_limit) throw FormatError("formatted UTF-16 output exceeds capacity");
            output.push_back(pattern[i]);
        }
    }
    if (argument != arguments.size()) throw FormatError("too many UTF-16 format arguments");
    return output;
}

} // namespace zh::foundation
