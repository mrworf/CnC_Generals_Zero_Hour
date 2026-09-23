#include "PreRTS.h"
#include "GameClient/VideoPlayer.h"

#include <cstdio>

namespace {
int failures;

void check(bool value, const char *message)
{
	if (!value)
	{
		std::fprintf(stderr, "FAIL: %s\n", message);
		++failures;
	}
}

class GeneratedStream final : public VideoStream
{
public:
	explicit GeneratedStream(VideoPlayer *player, Bool fail_update)
		: m_failUpdate(fail_update), m_updates(0), m_failed(FALSE)
	{
		m_player = player;
	}

	~GeneratedStream() override { ++destroyed; }

	void update() override
	{
		++m_updates;
		if (m_failUpdate)
			m_failed = TRUE;
	}

	int updates() const { return m_updates; }
	Bool failed() const { return m_failed; }
	static int destroyed;

private:
	Bool m_failUpdate;
	int m_updates;
	Bool m_failed;
};

int GeneratedStream::destroyed = 0;

class GeneratedVideoPlayer final : public VideoPlayer
{
public:
	GeneratedVideoPlayer() : m_failNextUpdate(FALSE), m_opens(0) {}

	VideoStreamInterface *open(AsciiString movie_title) override
	{
		if (movie_title != AsciiString("generated") || getVideo(movie_title) == NULL || m_firstStream != NULL)
			return NULL;
		GeneratedStream *stream = new GeneratedStream(this, m_failNextUpdate);
		if (stream == NULL)
			return NULL;
		m_firstStream = stream;
		++m_opens;
		return stream;
	}

	void fail_next_update() { m_failNextUpdate = TRUE; }
	void clear_failure() { m_failNextUpdate = FALSE; }
	Bool has_stream() const { return m_firstStream != NULL; }
	int opens() const { return m_opens; }

private:
	Bool m_failNextUpdate;
	int m_opens;
};

void add_generated_video(GeneratedVideoPlayer &player)
{
	Video descriptor;
	descriptor.m_internalName = AsciiString("generated");
	descriptor.m_filename = AsciiString("project-owned-generated-frame-sequence");
	player.addVideo(&descriptor);
	check(player.getNumVideos() == 1 && player.getVideo(AsciiString("generated")) != NULL,
		"original video descriptor registration changed");
	player.addVideo(&descriptor);
	check(player.getNumVideos() == 1, "duplicate original video descriptor was not replaced");
}

void run_generation(int generation)
{
	check(TheVideoPlayer == NULL, "stale video provider survived generation");
	{
		GeneratedVideoPlayer player;
		TheVideoPlayer = &player;
		add_generated_video(player);
		check(TheVideoPlayer->open(AsciiString("unknown")) == NULL, "unknown video title was admitted");
		player.fail_next_update();
		GeneratedStream *failed = static_cast<GeneratedStream *>(TheVideoPlayer->open(AsciiString("generated")));
		check(failed != NULL && player.has_stream(), "generated stream did not publish");
		check(TheVideoPlayer->open(AsciiString("generated")) == NULL, "duplicate active stream was admitted");
		TheVideoPlayer->update();
		check(failed->updates() == 1 && failed->failed(), "injected stream update failure was not observed");
		TheVideoPlayer->reset();
		check(!player.has_stream(), "original reset retained failed stream");
		player.clear_failure();
		GeneratedStream *retry = static_cast<GeneratedStream *>(TheVideoPlayer->open(AsciiString("generated")));
		check(retry != NULL && player.has_stream(), "stream retry did not republish");
		TheVideoPlayer->update();
		check(retry->updates() == 1 && !retry->failed(), "stream retry retained failure state");
		TheVideoPlayer->reset();
		check(!player.has_stream(), "original reset retained retry stream");
		check(player.opens() == 2, "unexpected generated stream open count");
	}
	check(TheVideoPlayer == NULL, "original provider destructor retained publication");
	(void)generation;
}
}

int main()
{
	for (int generation = 0; generation != 2; ++generation)
		run_generation(generation);
	check(GeneratedStream::destroyed == 4, "two-generation stream teardown count changed");
	check(TheVideoPlayer == NULL, "video provider teardown retained ownership");
	std::puts("original generated video stream registry: generations=2 streams=0");
	return failures ? 1 : 0;
}
