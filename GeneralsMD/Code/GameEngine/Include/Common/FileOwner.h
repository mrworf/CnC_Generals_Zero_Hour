// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/file.h"
#include <memory>
struct FileCloser {
    void operator()(File* file) const noexcept {if(file)file->close();}
};
using FileCloseOwner=std::unique_ptr<File,FileCloser>;
