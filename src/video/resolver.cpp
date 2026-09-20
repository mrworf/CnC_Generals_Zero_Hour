#include "zh/video/resolver.h"

#include "zh/data/vfs.h"
#include "zh/video/decoder.h"

#include <algorithm>
#include <cctype>

namespace zh::video {
namespace {
bool safe_component(std::string_view text)
{
    return !text.empty() && text != "." && text != ".."
        && std::all_of(text.begin(), text.end(), [](unsigned char character) {
            return std::isalnum(character) || character == '-' || character == '_';
        });
}

std::string normalize_movie_path(std::string_view path)
{
    if (path.empty() || path.front() == '/' || path.front() == '\\')
        throw VideoError("video movie path must be relative to Data");
    std::string result(path);
    std::replace(result.begin(), result.end(), '\\', '/');
    if (result.rfind("Data/", 0) == 0 || result.find("../") != std::string::npos
        || result.find("/..") != std::string::npos || result.back() == '/')
        throw VideoError("video movie path must be a safe Data-relative logical path");
    return result;
}
}

std::vector<std::string> localized_movie_candidates(std::string_view movie_path, std::string_view language)
{
    if (!safe_component(language)) throw VideoError("video language must be one safe path component");
    const auto relative = normalize_movie_path(movie_path);
    return {"Data/" + std::string(language) + "/" + relative, "Data/" + relative};
}

std::string resolve_localized_movie(const data::VirtualFileSystem& vfs,
    std::string_view movie_path, std::string_view language)
{
    const auto candidates = localized_movie_candidates(movie_path, language);
    for (const auto& candidate : candidates)
        if (const auto* resource = vfs.find(candidate)) return resource->logical_name;
    throw VideoError("video localized movie is absent; tried logical paths '" + candidates[0]
        + "' and fallback '" + candidates[1] + "'");
}

} // namespace zh::video
