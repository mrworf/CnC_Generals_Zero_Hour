# M24 slice 03 — original recorder and deterministic replay

Grade: production original-source recorder integration with a project-owned, source-parsed CkMp skirmish; cumulative retail and full-suite acceptance remains slice 04.

The Linux engine records source `GameMessage` commands through `RecorderClass::updateRecord` into the original `GENREP` layout. It publishes a completed replay from a unique XDG staging file with flush, sync and rename. A fresh process starts the same original replay through `RecorderClass::playbackFile`, original `MSG_NEW_GAME` dispatch, `GAME_REPLAY` mode and `InitRandom` seed. The recorded and replayed frame, complete `GameLogic` CRC and simulation RNG seed CRC agree. The same GCC Debug recording also replays unchanged in GCC Release, Clang Debug and Clang Release. No `ZHSG` or fixture-only replay codec is used.

Linux preflight validates the complete bounded original header and command stream before clearing the live game or changing recorder mode: local filename, `GENREP`, terminated fixed-width strings, source version/EXE/INI CRCs, parsed source game info and available MapCache map, command frame order, types, argument widths and EOF. Build-time and display-version strings are permitted to differ between Debug and Release; source numeric version and data CRCs remain required. The test mutates actual `.rep` bytes for bad magic, short header/command, wrong version/EXE/INI CRC, missing map, unsupported command type and invalid frame. Missing and path-traversal names also fail. Each startup rejection retains the live source frame, object count, CRC and recorder mode, and leaves the candidate bytes untouched. An intentionally wrong expected playback CRC is detected after replay advancement.

The original recorder's UTF-16 code units are now written/read as 16-bit values on Linux; glibc wide I/O would otherwise produce incompatible 32-bit fields. The source MapCache registers the project-owned map for skirmish setup, and narrow source headless guards handle an empty script list and absent observer-button UI. These are not renderer/M22 changes. Client/audio RNG independence is carried by M21 and is checked cumulatively in slice 04.

Validation:

- Four preset builds of `zh_original_main` — pass.
- `ctest --test-dir build/<preset> -R '^original_persistence_(save|replay)$' --output-on-failure` — 2/2 pass in GCC/Clang Debug/Release.
- `ctest --test-dir build/linux-gcc-debug -R '^original_(simulation|persistence)_' --output-on-failure` — 13/13 pass.
- One GCC Debug-produced source `.rep`, replayed by all four preset executables in separate processes — matching frame, GameLogic CRC, RNG seed CRC and `GAME_REPLAY` mode.
- `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/linux-clang-sanitized -R '^original_persistence_replay$' --output-on-failure` — 1/1 pass with ASan/UBSan enabled. LeakSanitizer itself cannot run under this environment's ptrace supervision.

Full four-preset asset-free suites, retail read-only gate, final source-identity and ledger checks remain slice 04.
