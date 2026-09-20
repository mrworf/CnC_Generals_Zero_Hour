# M15 single-player checkpoint — blocked

## Disposition

M15 is **not accepted**. The repository now has useful single-player integration
coverage, but the executable does not run original Zero Hour gameplay. Retail
corpus presence plus synthetic campaign/skirmish orchestration is not evidence
of completing a campaign mission or an all-faction retail skirmish.

Blocker: `SOURCE_GAMEPLAY_ENGINE_NOT_PORTED` (planning/architecture gap).

## Valid implemented evidence

- Campaign, skirmish (USA/China/GLA), and user-map requests traverse the ported
  data, XDG, deterministic persistence/replay, UI, world/effects recorder,
  optional audio/video resolution, pause/options, outcome, unload, and shutdown
  seams using a project-owned deterministic scenario model.
- Process tests run from an unrelated current directory, isolate all four XDG
  roots, reject missing required maps and invalid arguments, and write no state
  beside the executable or into retail roots.
- Corrupt save/replay tests preserve the active snapshot. A 1,000,000-tick run
  and 32 repeated save/load/replay/unload cycles complete deterministically.
- The English retail opt-in build verifies the existing text/data, speech,
  music/effects, and movie corpus and runs the integration coordinator against
  the read-only VFS. This proves corpus access and seam coordination only.

## Validation results

- Four canonical GCC/Clang Debug/Release builds: pass.
- Full mandatory non-LAN suite: 51/51 tests pass in each preset.
- Four-preset deterministic report comparison: 4 reports match, 4 checkpoints.
- Retail opt-in suite: `retail_data_verification`, `audio_corpus`,
  `video_corpus`, and `singleplayer_retail_integration` pass (4/4).
- Clang ASan+UBSan M15 suite: 4/4 pass outside the restricted ptrace sandbox.
  The same binaries cannot initialize LeakSanitizer inside that sandbox; this
  is an environment limitation, not a suppressed test failure.

These results qualify only the native seams and integration model that ran.
They do not satisfy representative retail gameplay acceptance.

## Source gameplay path investigation

The authoritative source path is intact in the legacy tree:

1. `GameMain()` creates a device-specific `GameEngine`, initializes it, and
   executes its loop.
2. `GameEngine::init()` creates the legacy local/archive file systems, parses
   the INI/object stores, creates audio, `GameClient`, AI, `GameLogic`, script
   engine, player list, recorder, radar, and victory conditions.
3. `GameLogic::startNewGame()` selects campaign/skirmish game information,
   loads map INI/object/team/script state, and enters source simulation.
4. `GameLogic::update()` and the script/victory systems drive actual mission and
   skirmish behavior.

None of that chain is linked into the native executable:

- CMake's `zh_game_engine` and `zh_game_device` targets compile only
  `src/bootstrap/component.cpp`. Their legacy source lists are target metadata,
  not compilation inputs.
- The legacy tree contains 586 GameEngine `.cpp` files, including 259 below
  `GameLogic`; no native target compiles this implementation.
- The only source `CreateGameEngine()` returns `Win32GameEngine`. Its factories
  instantiate Win32 local/BIG file systems, W3D client/logic, Miles audio, and
  other Win32 device classes. There is no Linux `GameEngine` factory adapting
  the new VFS, SDL platform/GPU, miniaudio, or FFmpeg interfaces to these legacy
  abstract classes.
- The first direct syntax probe of `GameLogic.cpp` reaches the shared
  `PreRTS.h`, which unconditionally includes ATL and a broad Win32/DirectInput
  header set; compilation stops at missing `atlbase.h`. This is before later
  Win32, DirectX 8, Miles, Bink, GameSpy, and legacy STL/API migration work.
- `zh::data::VirtualFileSystem`, `singleplayer::Session`, the renderer command
  generators, and the new media adapters are parallel project-owned APIs. They
  do not implement the legacy `FileSystem`/`ArchiveFileSystem`, `GameLogic`,
  `GameClient`, `AudioManager`, or video/UI interfaces consumed by the source
  engine.

## Why this is not a bounded M15 bridge

Connecting retail VFS to actual gameplay requires compiling and migrating the
source engine, introducing a Linux `GameEngine` implementation, and adapting
the prior milestone seams to the legacy interfaces across initialization,
memory/types, INI/object/module factories, map/script loading, simulation,
client/UI/rendering, media, persistence, and shutdown. That is the central port,
not a narrow M15 coordinator change. It reopens the architecture and delivery
assumptions behind M1–M13.

M15 and dependants must remain incomplete until delivery planning supplies and
executes source-engine migration milestones, after which this milestone can run
real representative campaign missions and all-faction skirmishes on the M14 GPU
system. LAN completion remains M16 and Windows persistence import remains M18.
