# M22 08E1C: generated `MultiPlayerLoadScreen` owner evidence

This slice composes the original `MultiPlayerLoadScreen` with a generated,
project-owned `Menus/MultiplayerLoadScreen.wnd`, source `GameInfo`/`GameSlot`,
INI-backed template/settings/map metadata, and a generated preview file.  It
does not read retail layouts, images, audio, maps, pixel baselines, or network
transport, and makes no fidelity claim for any of those inputs.

The dedicated owner witness uses two isolated generations.  It proves the
original constructor/init path creates the live WND tree, selects the local
FactionAmerica fallback image, maps occupied human/AI slots, binds a source
preview and start markers, takes the offline-null update/mouse route, applies
mapped remote progress, retains an unoccupied slot as inactive, rejects a
missing WND, and queues the reset-detached WND through the original window
manager.  Each generation gives its generated preview descriptor a private
`ImageCollection`; it is destroyed with that collection before the next
generation.  The source reset behavior is intentionally preserved: it clears
the base load-screen pointer, after which the manager owns queued destruction.

- Focused GCC and Clang coupling, identity, removal, and ledger checks passed
  21/21 each.  The exact `MultiPlayerLoadScreen::init(GameInfo*)` identity and
  removal checks are separate CTest entries.
- Fresh final-tree non-GPU/non-LAN suites passed 255/255 in every configured
  build: GCC Debug (225.33s), GCC Release (131.30s), Clang Debug (210.44s),
  Clang Release (80.95s), GCC ASAN/UBSAN (583.75s), and Clang ASAN/UBSAN
  (455.12s).  Canonical sandbox sanitizer runs use `detect_leaks=0` because
  LSan requires host ptrace capability.
- Strict host `detect_leaks=1` focused coupling passed with no leak report:
  GCC 18/18 (87.48s) and Clang 18/18 (63.99s).
- Physical Vulkan validation passed 9/9.  Serial local-socket LAN passed 4/4
  in GCC Debug, GCC Release, Clang Debug, Clang Release, GCC sanitizer, and
  Clang sanitizer.
