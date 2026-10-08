// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
class FileSystem;
// Load only through the already-mounted original read-only filesystem.
void loadOriginalMemoryProfile(FileSystem &files);
