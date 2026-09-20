#pragma once

#include <memory>

namespace zh::data { class VirtualFileSystem; }

namespace zh::audio {

class MiniaudioVfs {
public:
    explicit MiniaudioVfs(const data::VirtualFileSystem& vfs);
    ~MiniaudioVfs();

    MiniaudioVfs(const MiniaudioVfs&) = delete;
    MiniaudioVfs& operator=(const MiniaudioVfs&) = delete;

    void* native_handle() noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace zh::audio
