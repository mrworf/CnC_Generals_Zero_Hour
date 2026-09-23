# M22 08E1B3B: portable generated VideoBuffer allocation lifecycle

Use the original `VideoBuffer` base metadata after 08E1B3A to deliver a
project-owned portable headless allocation contract: accepted type, finite
extent, calculated pitch, lock/unlock, failure rollback, reset/re-entry and
zero ownership. Reject unknown format, zero/overflow extent and foreign or
stale buffer identity. No decoder, W3D texture, raw Direct3D, retail media or
pixels. Depends on 08E1B3A.
