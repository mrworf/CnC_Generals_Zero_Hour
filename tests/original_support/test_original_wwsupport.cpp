#include "zh/original_support/ww_support.h"

#include <iostream>
#include <stdexcept>
#include <string>

int main()
{
    try {
        const auto witness = zh::original_support::run_ww_support_probe();
        if (!zh::original_support::rejects_truncated_chunk()) throw std::runtime_error("truncated chunk negative control changed");
        std::cout << witness << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
