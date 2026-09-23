# M22 slice 08E1B: bounded single-player load-screen owner closure

Compose 08E1A with one generated source-compatible SinglePlayer layout,
mission/text/video/audio owner set.  Retain exact source selection, layout
names and lifecycle; reject absent/foreign/stale/duplicate/failure paths and
prove reset/retry/removal/zero ownership.  No retail layouts/assets/pixels or
map load.  Depends on 08E1A; required by 08E1.
