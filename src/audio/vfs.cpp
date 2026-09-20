#include "zh/audio/vfs.h"

#include "zh/data/vfs.h"
#include "miniaudio.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <new>
#include <vector>

namespace zh::audio {
namespace {

struct OpenFile {
    std::vector<foundation::UInt8> bytes;
    std::size_t cursor = 0;
};

} // namespace

struct MiniaudioVfs::Impl {
    ma_vfs_callbacks callbacks{};
    const data::VirtualFileSystem* vfs = nullptr;

    explicit Impl(const data::VirtualFileSystem& source) : vfs(&source)
    {
        callbacks.onOpen = [](ma_vfs* raw, const char* path, ma_uint32 mode, ma_vfs_file* result) {
            if (raw == nullptr || path == nullptr || result == nullptr || mode != MA_OPEN_MODE_READ) return MA_INVALID_ARGS;
            auto& self = *static_cast<Impl*>(raw);
            try {
                const auto* resource = self.vfs->find(path);
                if (resource == nullptr || resource->size > std::numeric_limits<std::size_t>::max()) return MA_DOES_NOT_EXIST;
                auto file = std::make_unique<OpenFile>();
                file->bytes = self.vfs->read_prefix(path, static_cast<std::size_t>(resource->size));
                *result = file.release();
                return MA_SUCCESS;
            } catch (...) {
                return MA_ERROR;
            }
        };
        callbacks.onClose = [](ma_vfs*, ma_vfs_file raw_file) {
            delete static_cast<OpenFile*>(raw_file);
            return MA_SUCCESS;
        };
        callbacks.onRead = [](ma_vfs*, ma_vfs_file raw_file, void* destination, std::size_t requested, std::size_t* read) {
            if (raw_file == nullptr || (requested != 0 && destination == nullptr)) return MA_INVALID_ARGS;
            auto& file = *static_cast<OpenFile*>(raw_file);
            const auto available = file.bytes.size() - file.cursor;
            const auto amount = std::min(requested, available);
            if (amount != 0) std::memcpy(destination, file.bytes.data() + file.cursor, amount);
            file.cursor += amount;
            if (read != nullptr) *read = amount;
            return amount == requested ? MA_SUCCESS : MA_AT_END;
        };
        callbacks.onSeek = [](ma_vfs*, ma_vfs_file raw_file, ma_int64 offset, ma_seek_origin origin) {
            if (raw_file == nullptr) return MA_INVALID_ARGS;
            auto& file = *static_cast<OpenFile*>(raw_file);
            const auto base = origin == ma_seek_origin_start ? ma_int64{0} :
                origin == ma_seek_origin_current ? static_cast<ma_int64>(file.cursor) : static_cast<ma_int64>(file.bytes.size());
            if ((offset < 0 && base < -offset) || (offset > 0 && base > std::numeric_limits<ma_int64>::max() - offset)) return MA_BAD_SEEK;
            const auto next = base + offset;
            if (next < 0 || static_cast<ma_uint64>(next) > file.bytes.size()) return MA_BAD_SEEK;
            file.cursor = static_cast<std::size_t>(next);
            return MA_SUCCESS;
        };
        callbacks.onTell = [](ma_vfs*, ma_vfs_file raw_file, ma_int64* cursor) {
            if (raw_file == nullptr || cursor == nullptr) return MA_INVALID_ARGS;
            *cursor = static_cast<ma_int64>(static_cast<OpenFile*>(raw_file)->cursor);
            return MA_SUCCESS;
        };
        callbacks.onInfo = [](ma_vfs*, ma_vfs_file raw_file, ma_file_info* info) {
            if (raw_file == nullptr || info == nullptr) return MA_INVALID_ARGS;
            info->sizeInBytes = static_cast<ma_uint64>(static_cast<OpenFile*>(raw_file)->bytes.size());
            return MA_SUCCESS;
        };
    }
};

MiniaudioVfs::MiniaudioVfs(const data::VirtualFileSystem& vfs) : impl_(std::make_unique<Impl>(vfs)) {}
MiniaudioVfs::~MiniaudioVfs() = default;
void* MiniaudioVfs::native_handle() noexcept { return impl_.get(); }

} // namespace zh::audio
