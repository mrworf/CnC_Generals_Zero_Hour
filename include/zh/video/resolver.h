#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace zh::data { class VirtualFileSystem; }

namespace zh::video {

std::vector<std::string> localized_movie_candidates(std::string_view movie_path, std::string_view language);
std::string resolve_localized_movie(const data::VirtualFileSystem& vfs,
    std::string_view movie_path, std::string_view language);

} // namespace zh::video
