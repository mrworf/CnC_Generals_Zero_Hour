#include "zh/data/inventory.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <unistd.h>

namespace {
int failures = 0;
void check(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

template <typename Function>
void expect_error(Function&& function, std::string_view expected)
{
    try { function(); check(false, "expected data error"); }
    catch (const zh::data::DataError& error) {
        check(std::string_view(error.what()).find(expected) != std::string_view::npos, "data error diagnostic");
    }
}

void write_bytes(const std::filesystem::path& path, const std::vector<unsigned char>& bytes)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path, std::ios::binary);
    stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void put_le32(std::vector<unsigned char>& bytes, std::size_t offset, std::uint32_t value)
{
    bytes[offset] = static_cast<unsigned char>(value);
    bytes[offset + 1] = static_cast<unsigned char>(value >> 8U);
    bytes[offset + 2] = static_cast<unsigned char>(value >> 16U);
    bytes[offset + 3] = static_cast<unsigned char>(value >> 24U);
}

std::vector<unsigned char> signature(std::string_view text, std::size_t size)
{
    std::vector<unsigned char> bytes(size, 0);
    std::copy(text.begin(), text.end(), bytes.begin());
    return bytes;
}
}

int main()
{
    using namespace zh::data;
    const auto root = std::filesystem::temp_directory_path() /
        ("zh-inventory-test-" + std::to_string(static_cast<long long>(::getpid())));
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
    write_bytes(root / "sample.ini", {'x', '=', '1', '\n'});
    write_bytes(root / "sample.csf", {' ', 'F', 'S', 'C'});
    write_bytes(root / "sample.w3d", {1, 0, 0, 0, 0, 0, 0, 0});
    auto dds = signature("DDS ", 128); put_le32(dds, 4, 124); put_le32(dds, 12, 1); put_le32(dds, 16, 1);
    write_bytes(root / "sample.dds", dds);
    std::vector<unsigned char> tga(18, 0); tga[2] = 2; tga[12] = 1; tga[14] = 1;
    write_bytes(root / "sample.tga", tga);
    auto wav = signature("RIFF", 12); std::copy_n("WAVE", 4, wav.begin() + 8); write_bytes(root / "sample.wav", wav);
    write_bytes(root / "sample.mp3", {'I', 'D', '3'});
    write_bytes(root / "sample.bik", {'B', 'I', 'K', 'i', 0, 0, 0, 0});
    write_bytes(root / "sample.ttf", {0, 1, 0, 0});
    write_bytes(root / "sample.otf", {'O', 'T', 'T', 'O'});
    write_bytes(root / "sample.refpack", {0x10, 0xfb});
    write_bytes(root / "sample.zlib", {0x78, 0x9c});
    write_bytes(root / "sample.fx", {'f', 'x'});

    const auto manifest_path = root / "manifest.tsv";
    {
        std::ofstream manifest(manifest_path);
        manifest << "locale\tEnglish\n";
        manifest << "@precedence\tloose > archive\ttest ordering\n";
        manifest << "sample.ini\tini\tconfiguration\n";
        manifest << "sample.csf\tcsf\tstrings\n";
        manifest << "sample.w3d\tw3d\tmodel\n";
        manifest << "sample.dds\tdds\ttexture\n";
        manifest << "sample.tga\ttga\timage\n";
        manifest << "sample.wav\twav\taudio\n";
        manifest << "sample.mp3\tmp3\tmusic\n";
        manifest << "sample.bik\tbink\tvideo\n";
        manifest << "sample.ttf\ttruetype\tfont\n";
        manifest << "sample.otf\topentype\tfont\n";
        manifest << "sample.refpack\trefpack\tcompression\n";
        manifest << "sample.zlib\tzlib\tcompression\n";
        manifest << "sample.fx\twwshade\teffect\n";
    }
    const auto manifest = load_corpus_manifest(manifest_path);
    const auto vfs = VirtualFileSystem::mount({root, root, "English", {}});
    std::ostringstream output;
    verify_corpus(vfs, manifest, "english", output);
    check(output.str().find("format=w3d count=1 sample=sample.w3d") != std::string::npos,
        "stable format count and sample");
    check(output.str().find("corpus verification ok") != std::string::npos, "successful corpus result");
    check(output.str().find("expectation=precedence=loose > archive") != std::string::npos,
        "logical-only expectation reported");

    for (const auto format : {AssetFormat::ini, AssetFormat::csf, AssetFormat::w3d, AssetFormat::dds,
             AssetFormat::tga, AssetFormat::wav, AssetFormat::mp3, AssetFormat::bink, AssetFormat::truetype,
             AssetFormat::opentype, AssetFormat::refpack, AssetFormat::zlib, AssetFormat::wwshade}) {
        if (format == AssetFormat::ini) {
            const zh::foundation::UInt8 nul = 0;
            expect_error([&] { validate_asset_metadata(format, "broken.asset", {&nul, 1}, 1); }, "corrupt broken.asset");
        } else {
            expect_error([&] { validate_asset_metadata(format, "broken.asset", {nullptr, 0}, 0); }, "corrupt broken.asset");
        }
    }

    auto missing = manifest;
    missing.required.push_back({"missing.ini", AssetFormat::ini, "required"});
    expect_error([&] { std::ostringstream sink; verify_corpus(vfs, missing, "English", sink); }, "missing required logical asset");
    expect_error([&] { std::ostringstream sink; verify_corpus(vfs, manifest, "French", sink); }, "does not match selected locale");
    { std::ofstream(manifest_path) << "locale\tEnglish\nthing.bin\tmystery\tunsupported\n"; }
    expect_error([&] { load_corpus_manifest(manifest_path); }, "unsupported format");
    write_bytes(root / "required.nox", {'N', 'O', 'X'});
    const auto nox_vfs = VirtualFileSystem::mount({root, root, "English", {}});
    expect_error([&] { std::ostringstream sink; verify_corpus(nox_vfs, manifest, "English", sink); }, "unsupported NOX");

    std::filesystem::remove_all(root, ignored);
    return failures == 0 ? 0 : 1;
}
