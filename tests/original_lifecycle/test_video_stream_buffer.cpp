#include "PreRTS.h"
#include "GameClient/VideoPlayer.h"
#include <cstdio>
#include <vector>

namespace {
int failures;
void check(bool v, const char *m) { if (!v) { std::fprintf(stderr, "FAIL: %s\n", m); ++failures; } }
class Buffer final : public VideoBuffer {
public:
	Buffer() : VideoBuffer(TYPE_X8R8G8B8), locked(FALSE), fail(FALSE) {}
	Bool allocate(UnsignedInt w, UnsignedInt h) override { free(); if (fail || w != 2 || h != 1) { fail=FALSE; return FALSE; } bytes.assign(8, 0); m_width=m_textureWidth=w; m_height=m_textureHeight=h; m_pitch=8; return TRUE; }
	void free() override { bytes.clear(); locked=FALSE; VideoBuffer::free(); m_pitch=0; }
	void *lock() override { if (!valid() || locked) return NULL; locked=TRUE; return bytes.data(); }
	void unlock() override { locked=FALSE; }
	Bool valid() override { return bytes.size()==8 && m_pitch==8; }
	Bool write(const UnsignedByte *src, size_t count) { if (!valid() || locked || src==NULL || count!=bytes.size()) return FALSE; for(size_t i=0;i<count;++i) bytes[i]=src[i]; return TRUE; }
	void fail_next() { fail=TRUE; }
private: std::vector<UnsignedByte> bytes; Bool locked, fail;
};
class Stream final : public VideoStream {
public:
	Stream(VideoPlayer *p, Bool fail) : ready(TRUE), decoded(FALSE), rendered(FALSE), index(0), render_fail(fail) { m_player=p; }
	void frameDecompress() override { if (ready && index==0) decoded=TRUE; }
	void frameRender(VideoBuffer *b) override { static const UnsignedByte frame[8]={1,2,3,4,5,6,7,8}; if (!decoded || render_fail || b==NULL || !b->valid()) return; Buffer *owned=dynamic_cast<Buffer*>(b); if (owned && owned->write(frame, sizeof(frame))) rendered=TRUE; }
	void frameNext() override { if (rendered) ++index; }
	Int frameIndex() override { return index; }
	Int frameCount() override { return 1; }
	Int width() override { return 2; }
	Int height() override { return 1; }
	Bool done() const { return rendered && index==1; }
private: Bool ready, decoded, rendered, render_fail; Int index;
};
class Player final : public VideoPlayer {
public:
	Player() : fail(FALSE) {}
	VideoStreamInterface *open(AsciiString name) override { if (name != AsciiString("generated") || m_firstStream) return NULL; Stream *s=new Stream(this, fail); m_firstStream=s; return s; }
	void set_fail(Bool v) { fail=v; }
	Bool active() const { return m_firstStream != NULL; }
private: Bool fail;
};
void run() {
	check(TheVideoPlayer==NULL,"stale provider"); { Player p; TheVideoPlayer=&p; check(p.open(AsciiString("unknown"))==NULL,"unknown open"); Buffer b; check(!b.allocate(3,1),"bad extent"); check(b.allocate(2,1),"allocate"); Stream *s=static_cast<Stream*>(p.open(AsciiString("generated"))); check(s && p.open(AsciiString("generated"))==NULL,"open/duplicate"); s->frameRender(&b); check(!s->done(),"render before decompress"); s->frameDecompress(); s->frameRender(&b); s->frameNext(); check(s->done(),"source order"); p.reset(); check(!p.active(),"close on reset"); p.set_fail(TRUE); s=static_cast<Stream*>(p.open(AsciiString("generated"))); s->frameDecompress(); s->frameRender(&b); s->frameNext(); check(!s->done(),"injected render failure"); p.reset(); p.set_fail(FALSE); s=static_cast<Stream*>(p.open(AsciiString("generated"))); s->frameDecompress(); s->frameRender(&b); s->frameNext(); check(s->done(),"retry"); p.reset(); b.free(); check(!b.valid(),"zero buffer"); } check(TheVideoPlayer==NULL,"provider removal");
}
}
int main(){run();run();std::puts("original generated stream-buffer: generations=2 resources=0");return failures?1:0;}
