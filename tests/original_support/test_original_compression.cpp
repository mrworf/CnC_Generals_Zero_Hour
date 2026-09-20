#include "zh/foundation/compression.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace zh::foundation;

namespace {

void check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

template <typename Function>
void rejects(Function function, const char* message)
{
    try {
        function();
    } catch (const CompressionError&) {
        return;
    }
    throw std::runtime_error(message);
}

} // namespace

int main()
{
    try {
        const std::vector<UInt8> input{
            'G', 'e', 'n', 'e', 'r', 'a', 'l', 's', ' ', 'Z', 'e', 'r', 'o', ' ', 'H', 'o', 'u', 'r',
            0, 1, 2, 3, 0, 1, 2, 3, 'G', 'e', 'n', 'e', 'r', 'a', 'l', 's'};
        const auto encoded = original_refpack_encode({input.data(), input.size()}, 4096);
        check(original_refpack_decode({encoded.data(), encoded.size()}, 4096, "owned/refpack") == input,
            "original RefPack round trip changed bytes");
        check(original_refpack_provider() == "GeneralsMD EAC Refpack 1.01", "original provider witness changed");

        auto truncated = encoded;
        truncated.pop_back();
        rejects([&] { original_refpack_decode({truncated.data(), truncated.size()}, 4096, "owned/truncated"); },
            "truncated original RefPack stream accepted");
        rejects([&] { original_refpack_decode({encoded.data(), encoded.size()}, input.size() - 1, "owned/limited"); },
            "original RefPack size boundary ignored");
        rejects([&] { original_refpack_encode({input.data(), input.size()}, 8); },
            "original RefPack encode size boundary ignored");

        std::cout << "original-support runtime provider=" << original_refpack_provider()
                  << " input=" << input.size() << " encoded=" << encoded.size() << " roundtrip=ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
