# M22 slice 08E1B: bounded single-player load-screen owner closure

Compose 08E1B1–08E1B4 with one generated source-compatible SinglePlayer layout,
mission/text/video/audio owner set.  Retain exact source selection, layout
names and lifecycle; reject absent/foreign/stale/duplicate/failure paths and
prove reset/retry/removal/zero ownership.  No retail layouts/assets/pixels or
map load.  Depends on 08E1A; required by 08E1.

## Aggregate contract

`original_singleplayer_loadscreen_aggregate` runs the accepted generated
mapped-image, Mouse, stream-buffer, and actual source-owner witnesses in
dependency order. Each requires its project-owned two-generation terminal
marker; missing/non-zero/stale witnesses fail the aggregate. The owner witness
is B4's real WND→Mission→text/image/video/audio update/destruction route.
This adds no production owner and does not convert source `reset` (which only
clears the source pointers) into a zero-ownership claim. Retail media/layout,
pixels, decoder, and multiplayer remain excluded.
