#include "PreRTS.h"

#include "Common/AsciiString.h"
#include "Common/file.h"
#include "PosixDevice/Common/PosixLocalFile.h"
#include "PosixDevice/Common/PosixLocalFileSystem.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fnmatch.h>
#include <system_error>

namespace {

std::filesystem::path nativePath(const Char *path)
{
	std::string value = path ? path : "";
	std::replace(value.begin(), value.end(), '\\', '/');
	return std::filesystem::path(value).lexically_normal();
}

AsciiString logicalPath(const std::filesystem::path& path)
{
	std::string value = path.generic_string();
	std::replace(value.begin(), value.end(), '/', '\\');
	return AsciiString(value.c_str());
}

}  // namespace

PosixLocalFileSystem::PosixLocalFileSystem() = default;
PosixLocalFileSystem::~PosixLocalFileSystem() = default;

void PosixLocalFileSystem::init() {}
void PosixLocalFileSystem::reset() {}
void PosixLocalFileSystem::update() {}

File *PosixLocalFileSystem::openFile(const Char *filename, Int access)
{
	if (filename == NULL || *filename == '\0')
		return NULL;

	const std::filesystem::path path = nativePath(filename);
	if (access & File::WRITE)
	{
		std::error_code error;
		if (!path.parent_path().empty())
			std::filesystem::create_directories(path.parent_path(), error);
		if (error)
			return NULL;
	}

	PosixLocalFile *file = newInstance(PosixLocalFile);
	if (!file->open(path.string().c_str(), access))
	{
		file->close();
		file->deleteInstance();
		return NULL;
	}
	file->deleteOnClose();
	return file;
}

Bool PosixLocalFileSystem::doesFileExist(const Char *filename) const
{
	std::error_code error;
	return std::filesystem::is_regular_file(nativePath(filename), error) && !error;
}

void PosixLocalFileSystem::getFileListInDirectory(const AsciiString& currentDirectory,
                                                   const AsciiString& originalDirectory,
                                                   const AsciiString& searchName,
                                                   FilenameList& filenameList,
                                                   Bool searchSubdirectories) const
{
	const std::filesystem::path base = nativePath(originalDirectory.str());
	const std::filesystem::path current = nativePath(currentDirectory.str());
	const std::filesystem::path directory = base / current;
	std::error_code error;
	if (!std::filesystem::is_directory(directory, error) || error)
		return;

	const auto accept = [&](const std::filesystem::directory_entry& entry) {
		if (!entry.is_regular_file(error) || error)
			return;
		const std::string name = entry.path().filename().string();
		if (fnmatch(searchName.str(), name.c_str(), FNM_CASEFOLD) != 0)
			return;
		filenameList.insert(logicalPath(entry.path()));
	};

	if (searchSubdirectories)
	{
		for (std::filesystem::recursive_directory_iterator it(directory, error), end;
		     it != end && !error; it.increment(error))
			accept(*it);
	}
	else
	{
		for (std::filesystem::directory_iterator it(directory, error), end;
		     it != end && !error; it.increment(error))
			accept(*it);
	}
}

Bool PosixLocalFileSystem::getFileInfo(const AsciiString& filename, FileInfo *fileInfo) const
{
	if (fileInfo == NULL)
		return FALSE;
	const std::filesystem::path path = nativePath(filename.str());
	std::error_code error;
	const std::uintmax_t size = std::filesystem::file_size(path, error);
	if (error)
		return FALSE;
	const auto writeTime = std::filesystem::last_write_time(path, error);
	if (error)
		return FALSE;
	const auto ticks = static_cast<std::uint64_t>(writeTime.time_since_epoch().count());
	fileInfo->sizeLow = static_cast<UnsignedInt>(size & 0xffffffffULL);
	fileInfo->sizeHigh = static_cast<UnsignedInt>((size >> 32U) & 0xffffffffULL);
	fileInfo->timestampLow = static_cast<UnsignedInt>(ticks & 0xffffffffULL);
	fileInfo->timestampHigh = static_cast<UnsignedInt>((ticks >> 32U) & 0xffffffffULL);
	return TRUE;
}

Bool PosixLocalFileSystem::createDirectory(AsciiString directory)
{
	if (directory.isEmpty())
		return FALSE;
	std::error_code error;
	const std::filesystem::path path = nativePath(directory.str());
	return (std::filesystem::is_directory(path, error) && !error) ||
	       (std::filesystem::create_directories(path, error) && !error);
}
