// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/NativeMemoryProfiles.h"
#include "Common/FileOwner.h"
#include "Common/FileSystem.h"
#include "Common/GameMemory.h"
#include "Common/file.h"
#include <string_view>
void loadOriginalMemoryProfile(FileSystem &files) {
  FileCloseOwner file(
      files.openFile("Data/INI/MemoryPools.ini", File::READ | File::BINARY));
  if (!file) {
    userMemoryLoadPoolProfile({});
    return;
  }
  const auto size = file->size();
  if (size < 0)
    throw ERROR_BAD_ARG;
  auto bytes = std::make_unique<char[]>(static_cast<std::size_t>(size));
  if (file->read(bytes.get(), size) != size)
    throw ERROR_BAD_ARG;
  userMemoryLoadPoolProfile(
      std::string_view(bytes.get(), static_cast<std::size_t>(size)));
}
