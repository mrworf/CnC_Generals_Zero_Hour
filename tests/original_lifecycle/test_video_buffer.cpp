#include "PreRTS.h"
#include "GameClient/VideoPlayer.h"

#include <cstdio>
#include <cstring>
#include <vector>

namespace {
int failures;
void check(bool value, const char *message) { if (!value) { std::fprintf(stderr, "FAIL: %s\n", message); ++failures; } }

class GeneratedVideoBuffer final : public VideoBuffer
{
public:
	explicit GeneratedVideoBuffer(Type format) : VideoBuffer(format), m_locked(FALSE), m_fail_next(FALSE), m_last_ok(TRUE) {}
	Bool allocate(UnsignedInt width, UnsignedInt height) override
	{
		free();
		const UnsignedInt bytes = bytes_per_pixel(m_format);
		const unsigned long long total = static_cast<unsigned long long>(width) * height * bytes;
		if (m_fail_next || bytes == 0 || width == 0 || height == 0 || width > 4096 || height > 4096 || total > 16u * 1024u * 1024u)
		{
			m_fail_next = FALSE; m_last_ok = FALSE; return FALSE;
		}
		m_bytes.assign(static_cast<size_t>(total), 0);
		m_width = m_textureWidth = width; m_height = m_textureHeight = height; m_pitch = width * bytes;
		m_last_ok = TRUE; return TRUE;
	}
	void free() override { m_bytes.clear(); m_locked = FALSE; VideoBuffer::free(); m_pitch = 0; }
	void *lock() override { if (!valid() || m_locked) { m_last_ok = FALSE; return NULL; } m_locked = TRUE; m_last_ok = TRUE; return m_bytes.data(); }
	void unlock() override { if (!m_locked) { m_last_ok = FALSE; return; } m_locked = FALSE; m_last_ok = TRUE; }
	Bool valid() override { return !m_bytes.empty() && m_pitch != 0; }
	Bool upload(const void *data, size_t bytes)
	{
		if (!valid() || m_locked || data == NULL || bytes != m_bytes.size()) { m_last_ok = FALSE; return FALSE; }
		std::memcpy(m_bytes.data(), data, bytes); m_last_ok = TRUE; return TRUE;
	}
	void fail_next_allocate() { m_fail_next = TRUE; }
	Bool last_ok() const { return m_last_ok; }
	static UnsignedInt bytes_per_pixel(Type type)
	{
		switch (type) { case TYPE_R8G8B8: return 3; case TYPE_X8R8G8B8: return 4; case TYPE_R5G6B5: case TYPE_X1R5G5B5: return 2; default: return 0; }
	}
private:
	std::vector<UnsignedByte> m_bytes; Bool m_locked; Bool m_fail_next; Bool m_last_ok;
};

void run_generation()
{
	GeneratedVideoBuffer unknown(VideoBuffer::TYPE_UNKNOWN);
	check(!unknown.allocate(4, 4) && !unknown.valid(), "unknown format was admitted");
	GeneratedVideoBuffer buffer(VideoBuffer::TYPE_X8R8G8B8);
	check(!buffer.allocate(0, 4) && !buffer.allocate(4, 0) && !buffer.allocate(4097, 1), "invalid extent was admitted");
	buffer.fail_next_allocate(); check(!buffer.allocate(4, 2) && !buffer.valid(), "injected allocation failure retained ownership");
	check(buffer.allocate(4, 2) && buffer.valid() && buffer.pitch() == 16, "portable buffer extent/pitch changed");
	check(buffer.lock() != NULL && buffer.lock() == NULL && !buffer.last_ok(), "double lock was admitted");
	buffer.unlock(); buffer.unlock(); check(!buffer.last_ok(), "double unlock was admitted");
	std::vector<UnsignedByte> frame(32, 0x7f);
	check(!buffer.upload(NULL, frame.size()) && !buffer.upload(frame.data(), frame.size() - 1), "invalid generated frame was admitted");
	check(buffer.upload(frame.data(), frame.size()), "generated frame replacement failed");
	buffer.free(); check(!buffer.valid() && buffer.lock() == NULL, "stale buffer remained usable after free");
	check(buffer.allocate(2, 2) && buffer.pitch() == 8, "buffer recreation failed");
	buffer.free(); check(!buffer.valid(), "buffer teardown retained ownership");
}
}
int main() { run_generation(); run_generation(); std::puts("original generated video buffer: generations=2 bytes=0"); return failures ? 1 : 0; }
