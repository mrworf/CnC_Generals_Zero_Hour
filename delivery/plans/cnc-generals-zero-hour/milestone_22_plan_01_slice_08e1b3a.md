# M22 08E1B3A: original generic video-player stream registry lifecycle

## Goal

Exercise the original portable `VideoPlayer`/`VideoStream` ownership and
stream-list lifecycle with a project-owned generated headless stream.  This
is the first dependency of the later SinglePlayer video path, with no media
decoder, video buffer, layout, retail input, or pixel output.

## Scope and boundary

Compile the original `VideoPlayer.cpp` and `VideoStream.cpp` path and prove a
derived generated player can publish a known stream, service its `update`,
close it through the original list/removal mechanics, reset, and re-enter over
two generations.  Generated stream data is only counters/state; it cannot
decode or render bytes. The source Bink implementation is intentionally not
linked because it requires the proprietary Bink ABI; the W3D buffer is also
not linked because it requires legacy W3D/Direct3D texture ownership.

## Controls

The dedicated probe covers absent player publication, unknown/open failure,
duplicate stream rejection, invalid frame progression, injected stream update
failure followed by reset/retry, close/reset provider removal, stale stream
unreachability, and zero live streams after each generation.  A source-removal
link check proves the generic original provider, rather than a test-only
replacement, supplies the registry lifecycle.

## Acceptance

Focused GCC and Clang probes, original-source identity/removal evidence, the
repository proportional six-config and host validation matrix, ledger/evidence
updates, and a single independent commit.  This slice cannot claim frame
decode, buffer allocation, media compatibility, retail reachability, or
pixels.
