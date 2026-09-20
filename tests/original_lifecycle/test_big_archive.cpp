#include "PreRTS.h"

#include "LinuxBIGArchive.h"
#include "Common/ArchiveFile.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/file.h"
#include "Common/FileSystem.h"
#include "Common/GameEngine.h"
#include "Common/LocalFileSystem.h"
#include "PosixDevice/Common/PosixLocalFileSystem.h"
#include "zh/original_process.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <unistd.h>

GameEngine *TheGameEngine = NULL;

namespace {

int failures = 0;

void check(bool condition, std::string_view message)
{
	if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

void appendBE(std::vector<unsigned char>& out, UnsignedInt value)
{
	out.push_back(static_cast<unsigned char>(value >> 24U));
	out.push_back(static_cast<unsigned char>(value >> 16U));
	out.push_back(static_cast<unsigned char>(value >> 8U));
	out.push_back(static_cast<unsigned char>(value));
}

void appendLE(std::vector<unsigned char>& out, UnsignedInt value)
{
	out.push_back(static_cast<unsigned char>(value));
	out.push_back(static_cast<unsigned char>(value >> 8U));
	out.push_back(static_cast<unsigned char>(value >> 16U));
	out.push_back(static_cast<unsigned char>(value >> 24U));
}

void setBE(std::vector<unsigned char>& out, std::size_t offset, UnsignedInt value)
{
	for (unsigned index = 0; index != 4; ++index)
		out[offset + index] = static_cast<unsigned char>(value >> (24U - index * 8U));
}

void setLE(std::vector<unsigned char>& out, std::size_t offset, UnsignedInt value)
{
	for (unsigned index = 0; index != 4; ++index)
		out[offset + index] = static_cast<unsigned char>(value >> (index * 8U));
}

using Entry = std::pair<std::string, std::string>;

std::vector<unsigned char> makeBIG(std::string_view identifier, const std::vector<Entry>& entries)
{
	std::vector<unsigned char> table;
	UnsignedInt offset = 16U;
	for (const Entry& entry : entries)
		offset += 8U + static_cast<UnsignedInt>(entry.first.size()) + 1U;
	UnsignedInt dataOffset = offset;
	for (const Entry& entry : entries)
	{
		appendBE(table, dataOffset);
		appendBE(table, static_cast<UnsignedInt>(entry.second.size()));
		table.insert(table.end(), entry.first.begin(), entry.first.end());
		table.push_back(0);
		dataOffset += static_cast<UnsignedInt>(entry.second.size());
	}
	std::vector<unsigned char> out(identifier.begin(), identifier.end());
	appendLE(out, dataOffset);
	appendBE(out, static_cast<UnsignedInt>(entries.size()));
	appendBE(out, offset);
	out.insert(out.end(), table.begin(), table.end());
	for (const Entry& entry : entries) out.insert(out.end(), entry.second.begin(), entry.second.end());
	return out;
}

void writeBytes(const std::filesystem::path& path, const std::vector<unsigned char>& bytes)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream stream(path, std::ios::binary | std::ios::trunc);
	stream.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

std::string readFile(File *file)
{
	if (file == NULL) return {};
	std::string result(static_cast<std::size_t>(file->size()), '\0');
	const Int read = file->read(result.data(), static_cast<Int>(result.size()));
	file->close();
	if (read < 0) return {};
	result.resize(static_cast<std::size_t>(read));
	return result;
}

bool rejected(ArchiveFileSystem *archives, const std::filesystem::path& root,
	const std::string& name, const std::vector<unsigned char>& bytes)
{
	writeBytes(root / name, bytes);
	try
	{
		ArchiveFile *archive = archives->openArchiveFile(name.c_str());
		if (archive != NULL) delete archive;
		return false;
	}
	catch (const std::exception& error)
	{
		return std::string(error.what()).find("BIG archive") != std::string::npos;
	}
	catch (...) { return false; }
}

}

int run()
{
	std::array<char, 96> rootTemplate{};
	const std::string templatePath = (std::filesystem::temp_directory_path() / "zh-m20-big-XXXXXX").string();
	std::copy(templatePath.begin(), templatePath.end(), rootTemplate.begin());
	const Char *createdRoot = ::mkdtemp(rootTemplate.data());
	if (createdRoot == NULL) { std::cerr << "FAIL: cannot create fixture root\n"; return 1; }
	const std::filesystem::path root(createdRoot);
	std::error_code ignored;

	PosixLocalFileSystem localFiles(root);
	TheLocalFileSystem = &localFiles;
	localFiles.init();
	std::unique_ptr<ArchiveFileSystem> archives(zh::original_runtime::createLinuxBIGArchiveFileSystem());

	const auto bigf = makeBIG("BIGF", {{"Data/INI/MixedCase.ini", "alpha"}, {"Empty.bin", ""}});
	const auto big4 = makeBIG("BIG4", {{"stream/path.bin", "0123456789"}});
	writeBytes(root / "valid-f.big", bigf);
	writeBytes(root / "valid-4.big", big4);
	ArchiveFile *direct = archives->openArchiveFile("valid-f.big");
	check(direct != NULL, "BIGF opens");
	check(readFile(direct ? direct->openFile("data\\ini\\MIXEDCASE.INI", File::READ) : NULL) == "alpha",
		"case-folded slash-normalized RAM lookup");
	check(readFile(direct ? direct->openFile("empty.bin", File::READ) : NULL).empty(), "zero-length entry opens");
	check(direct && direct->openFile("missing.ini", File::READ) == NULL, "missing entry returns null");
	delete direct;
	direct = archives->openArchiveFile("valid-4.big");
	File *stream = direct ? direct->openFile("STREAM\\PATH.BIN", File::READ | File::STREAMING) : NULL;
	check(stream != NULL, "BIG4 streaming entry opens");
	Char buffer[8]{};
	check(stream && stream->read(buffer, 4) == 4 && std::string(buffer, 4) == "0123", "streaming read begins at entry");
	check(stream && stream->seek(-2, File::END) == 8 && stream->read(buffer, 8) == 2 &&
		std::string(buffer, 2) == "89", "streaming seek/read clamps to entry");
	check(stream && stream->seek(100, File::START) == 10 && stream->read(buffer, 1) == 0,
		"streaming seek cannot cross entry end");
	if (stream) stream->close();
	delete direct;

	check(archives->openArchiveFile("absent.big") == NULL, "failed archive open returns null");
	check(rejected(archives.get(), root, "short.big", {'B', 'I', 'G'}), "short header rejected");
	auto malformed = bigf;
	std::copy_n("NOPE", 4, malformed.begin());
	check(rejected(archives.get(), root, "identifier.big", malformed), "invalid identifier rejected");
	malformed = bigf; setLE(malformed, 4, static_cast<UnsignedInt>(malformed.size() + 1U));
	check(rejected(archives.get(), root, "declared-size.big", malformed), "declared size mismatch rejected");
	malformed = bigf; setBE(malformed, 8, 1000001U);
	check(rejected(archives.get(), root, "count.big", malformed), "oversized count rejected");
	malformed = bigf; setBE(malformed, 12, 15U);
	check(rejected(archives.get(), root, "table-low.big", malformed), "table before header rejected");
	malformed = bigf; setBE(malformed, 12, static_cast<UnsignedInt>(malformed.size() + 1U));
	check(rejected(archives.get(), root, "table-high.big", malformed), "table after EOF rejected");
	malformed = bigf; setBE(malformed, 8, 100U);
	check(rejected(archives.get(), root, "table-count.big", malformed), "count impossible for table rejected");
	malformed = makeBIG("BIGF", {{"unterminated", "x"}}); malformed[16U + 8U + std::string("unterminated").size()] = 'x';
	check(rejected(archives.get(), root, "unterminated.big", malformed), "unterminated name rejected");
	malformed = makeBIG("BIGF", {{std::string(1024U, 'a'), "x"}});
	check(rejected(archives.get(), root, "long-name.big", malformed), "oversized name rejected");
	malformed = bigf; setBE(malformed, 16, 1U);
	check(rejected(archives.get(), root, "offset-before-table.big", malformed), "offset before data rejected");
	malformed = bigf; setBE(malformed, 16, static_cast<UnsignedInt>(malformed.size() + 1U));
	check(rejected(archives.get(), root, "offset-after-eof.big", malformed), "offset after EOF rejected");
	malformed = bigf; setBE(malformed, 20, 0xffffffffU);
	check(rejected(archives.get(), root, "range-overflow.big", malformed), "overflowing entry range rejected");
	malformed = makeBIG("BIGF", {{"same/path", "a"}, {"SAME\\PATH", "b"}});
	check(rejected(archives.get(), root, "duplicate.big", malformed), "canonical duplicate rejected");
	malformed = makeBIG("BIGF", {{"../escape", "a"}});
	check(rejected(archives.get(), root, "traversal.big", malformed), "path traversal rejected");
	malformed = bigf; malformed.insert(malformed.begin() + static_cast<std::ptrdiff_t>(16U + 8U + 23U + 1U), 0);
	setLE(malformed, 4, static_cast<UnsignedInt>(malformed.size()));
	check(rejected(archives.get(), root, "table-junk.big", malformed), "table size mismatch rejected");

	const auto first = makeBIG("BIGF", {{"shared.txt", "first"}, {"first-only.txt", "one"}});
	const auto second = makeBIG("BIGF", {{"shared.txt", "second"}, {"second-only.txt", "two"}});
	writeBytes(root / "load/01-first.big", first);
	writeBytes(root / "load/02-second.big", second);
	std::unique_ptr<ArchiveFileSystem> ordered(zh::original_runtime::createLinuxBIGArchiveFileSystem());
	check(ordered->loadBigFilesFromDirectory("load", "*.big", FALSE), "ordered archives load");
	check(readFile(ordered->openFile("shared.txt")) == "first", "first archive wins without overwrite");
	ordered->closeArchiveFile("load\\01-first.big");
	check(readFile(ordered->openFile("shared.txt")) == "second", "close rebuild removes stale first location");
	check(readFile(ordered->openFile("first-only.txt")).empty(), "closed archive-only entry is unpublished");
	ordered->closeAllArchiveFiles();
	ordered->closeAllArchiveFiles();
	check(ordered->openFile("second-only.txt") == NULL, "repeated cleanup leaves no stale entries");

	std::unique_ptr<ArchiveFileSystem> overwritten(zh::original_runtime::createLinuxBIGArchiveFileSystem());
	check(overwritten->loadBigFilesFromDirectory("load", "*.big", TRUE), "overwrite archives load");
	check(readFile(overwritten->openFile("shared.txt")) == "second", "later archive wins with overwrite");
	writeBytes(root / "partial/01-valid.big", first);
	writeBytes(root / "partial/02-invalid.big", {'B', 'A', 'D'});
	std::unique_ptr<ArchiveFileSystem> transactional(zh::original_runtime::createLinuxBIGArchiveFileSystem());
	check(!transactional->loadBigFilesFromDirectory("partial", "*.big", FALSE), "mixed valid/malformed load fails");
	check(transactional->openFile("shared.txt") == NULL, "failed multi-archive load publishes nothing");

	std::ofstream(root / "shared.txt", std::ios::binary | std::ios::trunc) << "loose";
	FileSystem multiplexer;
	TheArchiveFileSystem = overwritten.get();
	TheFileSystem = &multiplexer;
	check(readFile(multiplexer.openFile("shared.txt", File::READ | File::BINARY)) == "loose",
		"loose file precedence is preserved");
	TheFileSystem = NULL;
	TheArchiveFileSystem = NULL;
	TheLocalFileSystem = NULL;
	archives.reset();
	ordered.reset();
	overwritten.reset();
	transactional.reset();
	std::filesystem::remove_all(root, ignored);
	if (failures) return 1;
	std::cout << "M20 BIG archive: valid=BIGF/BIG4 lookup/override/ram/stream cleanup; malformed/range=closed\n";
	return 0;
}

int main()
{
	initMemoryManager();
	const std::size_t poolBaseline = zh::original_process::live_pool_allocations();
	const int result = run();
	if (zh::original_process::live_pool_allocations() != poolBaseline)
	{
		std::cerr << "FAIL: BIG archive pool ownership did not return to baseline\n";
		return 1;
	}
	return result;
}
