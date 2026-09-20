#include "miniaudio.h"

#include <iostream>
#include <string_view>

int main()
{
    static_assert(MA_VERSION_MAJOR == 0);
    static_assert(MA_VERSION_MINOR == 11);
    static_assert(MA_VERSION_REVISION == 25);

    if (std::string_view(ma_version_string()) != "0.11.25") {
        std::cerr << "unexpected miniaudio runtime version: " << ma_version_string() << '\n';
        return 1;
    }

    ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 2, 48000);
    if (config.format != ma_format_f32 || config.channels != 2 || config.sampleRate != 48000) {
        std::cerr << "miniaudio decoder configuration is unavailable\n";
        return 1;
    }

    std::cout << "miniaudio 0.11.25: local decoder capability ok\n";
    return 0;
}
