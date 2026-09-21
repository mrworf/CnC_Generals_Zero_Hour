#include "PreRTS.h"

#include "LinuxBIGArchive.h"
#include "Common/ArchiveFile.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/file.h"
#include "Common/LocalFileSystem.h"
#include "Common/RAMFile.h"
#include "Common/StreamingArchiveFile.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr UnsignedInt kBIGHeaderSize = 16U;
constexpr UnsignedInt kMaximumBIGEntries = 1000000U;
constexpr std::size_t kMaximumBIGLogicalName = 1023U;

UnsignedInt readBigEndian(File *file)
{
	UnsignedByte bytes[4]{};
	if (file == NULL || file->read(bytes, sizeof(bytes)) != sizeof(bytes))
		throw std::runtime_error("truncated BIG archive integer");
	return (UnsignedInt(bytes[0]) << 24U) | (UnsignedInt(bytes[1]) << 16U) |
		(UnsignedInt(bytes[2]) << 8U) | UnsignedInt(bytes[3]);
}

UnsignedInt readLittleEndian(File *file)
{
	UnsignedByte bytes[4]{};
	if (file == NULL || file->read(bytes, sizeof(bytes)) != sizeof(bytes))
		throw std::runtime_error("truncated BIG archive integer");
	return UnsignedInt(bytes[0]) | (UnsignedInt(bytes[1]) << 8U) |
		(UnsignedInt(bytes[2]) << 16U) | (UnsignedInt(bytes[3]) << 24U);
}

std::string canonicalLogicalPath(std::string logical)
{
	if (logical.empty() || logical.size() > kMaximumBIGLogicalName)
		throw std::runtime_error("invalid BIG archive entry name");
	std::replace(logical.begin(), logical.end(), '/', '\\');
	if (logical.front() == '\\' || logical.back() == '\\' || logical.find(':') != std::string::npos)
		throw std::runtime_error("invalid BIG archive entry path");
	std::size_t start = 0;
	while (start < logical.size())
	{
		const std::size_t end = logical.find('\\', start);
		const std::string component = logical.substr(start, end == std::string::npos ? end : end - start);
		if (component.empty() || component == "." || component == "..")
			throw std::runtime_error("invalid BIG archive entry path");
		for (unsigned char value : component)
			if (value < 0x20U || value == 0x7fU)
				throw std::runtime_error("invalid BIG archive entry name");
		if (end == std::string::npos) break;
		start = end + 1U;
	}
	std::transform(logical.begin(), logical.end(), logical.begin(),
		[](unsigned char value) { return static_cast<char>(std::tolower(value)); });
	return logical;
}

class LinuxBIGFile final : public ArchiveFile
{
public:
	LinuxBIGFile() { m_file = NULL; }
	File *openFile(const Char *filename, Int access = 0) override
	{
		const ArchivedFileInfo *info = find(filename);
		if (info == NULL || m_file == NULL) return NULL;
		RAMFile *file = BitTest(access, File::STREAMING) ?
			static_cast<RAMFile *>(newInstance(StreamingArchiveFile)) : newInstance(RAMFile);
		file->deleteOnClose();
		if (!file->openFromArchive(m_file, info->m_filename, static_cast<Int>(info->m_offset),
			static_cast<Int>(info->m_size)))
		{
			file->close();
			return NULL;
		}
		if (!(access & File::WRITE)) return file;
		File *local = TheLocalFileSystem->openFile(filename, access);
		if (local != NULL) file->copyDataToFile(local);
		file->close();
		return local;
	}
	Bool getFileInfo(const AsciiString& filename, FileInfo *fileInfo) const override
	{
		const ArchivedFileInfo *info = find(filename.str());
		if (info == NULL || fileInfo == NULL || m_file == NULL) return FALSE;
		if (!TheLocalFileSystem->getFileInfo(AsciiString(m_file->getName()), fileInfo)) return FALSE;
		fileInfo->sizeHigh = 0;
		fileInfo->sizeLow = info->m_size;
		return TRUE;
	}
	void closeAllFiles() override {}
	AsciiString getName() override { return m_name; }
	AsciiString getPath() override { return m_path; }
	void setSearchPriority(Int) override {}
	void close() override
	{
		if (m_file != NULL)
		{
			m_file->close();
			m_file = NULL;
		}
	}
	void setIdentity(const Char *path)
	{
		m_path = path;
		m_name = std::filesystem::path(path ? path : "").filename().string().c_str();
	}
	void addEntry(const std::string& canonical, const ArchivedFileInfo& info)
	{
		if (!m_entries.emplace(canonical, info).second)
			throw std::runtime_error("duplicate BIG archive logical path");
		const std::size_t split = canonical.find_last_of('\\');
		const std::string path = split == std::string::npos ? "" : canonical.substr(0, split + 1U);
		addFile(AsciiString(path.c_str()), &info);
	}
private:
	const ArchivedFileInfo *find(const Char *filename) const
	{
		if (filename == NULL) return NULL;
		std::string key;
		try { key = canonicalLogicalPath(filename); }
		catch (...) { return NULL; }
		auto found = m_entries.find(key);
		return found == m_entries.end() ? NULL : &found->second;
	}
	std::map<std::string, ArchivedFileInfo> m_entries;
	AsciiString m_name;
	AsciiString m_path;
};

class LinuxArchiveFileSystem final : public ArchiveFileSystem
{
public:
	void init() override
	{
		loadBigFilesAtRoot(AsciiString(""), AsciiString("*.big"), FALSE);
		const char *generalsRoot = std::getenv("ZH_GENERALS_DATA_ROOT");
		if (generalsRoot && *generalsRoot)
			loadBigFilesAtRoot(AsciiString(generalsRoot), AsciiString("*.big"), FALSE);
	}
	void update() override {}
	void reset() override {}
	void postProcessLoad() override {}
	ArchiveFile *openArchiveFile(const Char *filename) override
	{
		File *input = TheLocalFileSystem->openFile(filename, File::READ | File::BINARY);
		if (input == NULL) return NULL;
		try
		{
			const Int signedSize = input->size();
			if (signedSize < static_cast<Int>(kBIGHeaderSize))
				throw std::runtime_error("truncated BIG archive header");
			const UnsignedInt archiveSize = static_cast<UnsignedInt>(signedSize);
			Char identifier[5]{};
			if (input->read(identifier, 4) != 4 ||
				(std::strcmp(identifier, "BIGF") != 0 && std::strcmp(identifier, "BIG4") != 0))
				throw std::runtime_error("unsupported BIG archive header");
			const UnsignedInt declaredSize = readLittleEndian(input);
			const UnsignedInt count = readBigEndian(input);
			const UnsignedInt tableEnd = readBigEndian(input);
			if (declaredSize != archiveSize || archiveSize > UnsignedInt((std::numeric_limits<Int>::max)()) ||
				count > kMaximumBIGEntries || tableEnd < kBIGHeaderSize || tableEnd > archiveSize)
				throw std::runtime_error("invalid BIG archive table bounds");
			if (count > (archiveSize - kBIGHeaderSize) / 9U)
				throw std::runtime_error("invalid BIG archive entry count");

			auto archive = std::make_unique<LinuxBIGFile>();
			archive->setIdentity(filename);
			std::vector<std::pair<std::string, ArchivedFileInfo>> pendingEntries;
			pendingEntries.reserve(count);
			for (UnsignedInt index = 0; index < count; ++index)
			{
				const Int signedPosition = input->position();
				if (signedPosition < 0 || static_cast<UnsignedInt>(signedPosition) > archiveSize ||
					archiveSize - static_cast<UnsignedInt>(signedPosition) < 9U)
					throw std::runtime_error("truncated BIG archive entry table");
				ArchivedFileInfo info;
				info.m_archiveFilename = filename;
				info.m_offset = readBigEndian(input);
				info.m_size = readBigEndian(input);
				if (info.m_offset > archiveSize)
					throw std::runtime_error("BIG archive entry offset after EOF");
				if (info.m_size > archiveSize - info.m_offset)
					throw std::runtime_error("BIG archive entry size after EOF");
				if (info.m_offset > UnsignedInt((std::numeric_limits<Int>::max)()) ||
					info.m_size > UnsignedInt((std::numeric_limits<Int>::max)()))
					throw std::runtime_error("BIG archive entry exceeds runtime integer range");
				std::string logical;
				Bool terminated = FALSE;
				while (logical.size() <= kMaximumBIGLogicalName)
				{
					const Int namePosition = input->position();
					if (namePosition < 0 || static_cast<UnsignedInt>(namePosition) >= archiveSize)
						throw std::runtime_error("truncated BIG archive entry name");
					Char character = 0;
					if (input->read(&character, 1) != 1)
						throw std::runtime_error("truncated BIG archive entry name");
					if (character == 0) { terminated = TRUE; break; }
					logical.push_back(character);
				}
				if (!terminated)
					throw std::runtime_error("invalid BIG archive entry name");
				const std::string canonical = canonicalLogicalPath(std::move(logical));
				const std::size_t split = canonical.find_last_of('\\');
				info.m_filename = (split == std::string::npos ? canonical : canonical.substr(split + 1U)).c_str();
				pendingEntries.emplace_back(canonical, info);
			}
			if (input->position() < 0)
				throw std::runtime_error("BIG archive table position invalid");
			const UnsignedInt parsedTableEnd = static_cast<UnsignedInt>(input->position());
			if (parsedTableEnd > tableEnd)
				throw std::runtime_error("BIG archive entries exceed declared table boundary");
			for (const auto& entry : pendingEntries)
			{
				if (entry.second.m_size != 0 && entry.second.m_offset < parsedTableEnd)
					throw std::runtime_error("BIG archive entry overlaps parsed table");
				archive->addEntry(entry.first, entry.second);
			}
			archive->attachFile(input);
			return archive.release();
		}
		catch (...)
		{
			input->close();
			throw;
		}
	}
	void closeArchiveFile(const Char *filename) override
	{
		auto found = m_archiveFileMap.find(AsciiString(filename));
		if (found == m_archiveFileMap.end()) return;
		delete found->second;
		m_archiveFileMap.erase(found);
		m_loadOrder.erase(std::remove_if(m_loadOrder.begin(), m_loadOrder.end(),
			[&](const LoadedArchive& loaded) { return loaded.filename == AsciiString(filename); }), m_loadOrder.end());
		rebuildDirectoryTree();
	}
	void closeAllArchiveFiles() override
	{
		for (auto& entry : m_archiveFileMap) delete entry.second;
		m_archiveFileMap.clear();
		m_loadOrder.clear();
		m_rootDirectory.clear();
	}
	void closeAllFiles() override
	{
		for (auto& entry : m_archiveFileMap) entry.second->closeAllFiles();
	}
	Bool loadBigFilesFromDirectory(AsciiString directory, AsciiString mask, Bool overwrite) override
	{
		return loadBigFiles(directory, mask, overwrite, TRUE);
	}
private:
	Bool loadBigFilesAtRoot(AsciiString directory, AsciiString mask, Bool overwrite)
	{
		return loadBigFiles(directory, mask, overwrite, FALSE);
	}
	Bool loadBigFiles(AsciiString directory, AsciiString mask, Bool overwrite, Bool recursive)
	{
		FilenameList files;
		TheLocalFileSystem->getFileListInDirectory(directory, AsciiString(""), mask, files, recursive);
		std::vector<std::pair<AsciiString, std::unique_ptr<ArchiveFile>>> pending;
		try
		{
			for (const AsciiString& filename : files)
			{
				if (m_archiveFileMap.find(filename) != m_archiveFileMap.end()) continue;
				ArchiveFile *archive = openArchiveFile(filename.str());
				if (archive != NULL) pending.emplace_back(filename, archive);
			}
		}
		catch (...)
		{
			return FALSE;
		}
		for (auto& candidate : pending)
		{
			loadIntoDirectoryTree(candidate.second.get(), candidate.first, overwrite);
			m_archiveFileMap.emplace(candidate.first, candidate.second.release());
			m_loadOrder.push_back({candidate.first, overwrite});
		}
		return pending.empty() ? FALSE : TRUE;
	}
	struct LoadedArchive { AsciiString filename; Bool overwrite; };
	void rebuildDirectoryTree()
	{
		m_rootDirectory.clear();
		for (const LoadedArchive& loaded : m_loadOrder)
		{
			auto found = m_archiveFileMap.find(loaded.filename);
			if (found != m_archiveFileMap.end())
				loadIntoDirectoryTree(found->second, loaded.filename, loaded.overwrite);
		}
	}
	std::vector<LoadedArchive> m_loadOrder;
};

}

ArchiveFileSystem *zh::original_runtime::createLinuxBIGArchiveFileSystem()
{
	return new LinuxArchiveFileSystem;
}
