#pragma once

#include "Common/LocalFileSystem.h"

#include <filesystem>

class PosixLocalFileSystem : public LocalFileSystem
{
public:
	PosixLocalFileSystem();
	explicit PosixLocalFileSystem(std::filesystem::path readRoot);
	~PosixLocalFileSystem() override;

	void init() override;
	void reset() override;
	void update() override;
	File *openFile(const Char *filename, Int access = 0) override;
	Bool doesFileExist(const Char *filename) const override;
	void getFileListInDirectory(const AsciiString& currentDirectory,
	                            const AsciiString& originalDirectory,
	                            const AsciiString& searchName,
	                            FilenameList& filenameList,
	                            Bool searchSubdirectories) const override;
	Bool getFileInfo(const AsciiString& filename, FileInfo *fileInfo) const override;
	Bool createDirectory(AsciiString directory) override;

private:
	std::filesystem::path resolveReadPath(const Char *path) const;
	std::filesystem::path m_readRoot;
};
