# N2 plan02 slice01 — original native root startup

The `zero_hour_headless --startup` process calls the original `GameEngine::init`
through a concrete native factory, then retires the complete owner graph. The
generated fixture also invokes that documented CLI on its generated root.
Generated-root validation executes two consecutive startup/shutdown cycles in
one process. Separate cases reject a missing mount, a missing required GameData
table, and user storage inside the read-only mount. The latter two cases repair
the same generated root and then complete original startup and teardown.

## Source and input boundaries

- `NativeHeadlessGameEngine` supplies the actual original GameLogic and logical
  GameClient, explicit read-only data mounts, separate native user storage,
  original metadata parsing, and explicit rejection of physical output methods.
- Generated GameData includes parsed camera fields. Generated water and weather
  definitions satisfy the source reset/override lifetime; generated audio
  metadata and a dummy music existence marker do not claim audio decoding.
- The fixed native display-string pool profile follows the original allocator's
  required first-allocation lookup. No vendor source or private asset changed.
- The CLI reports only generic outcomes. It never echoes supplied selectors.

## Frozen focused checks

Commands, after building `zero_hour_headless` and
`original_native_headless_root_fixture` in each configured directory:

```sh
ctest --test-dir build/original-core-gcc -R '^original_native_headless_' --output-on-failure
ctest --test-dir build/original-core-gcc-sanitize -R '^original_native_headless_' --output-on-failure
ctest --test-dir build/original-core-clang-sanitize -R '^original_native_headless_' --output-on-failure
```

Five of five cases passed in each configuration with ASan/UBSan/LSan enabled
for sanitizer builds. The sanitizer tests ran in the host execution context:
the sandbox denies LeakSanitizer process inspection, including on `--help`,
so its pre-execution failures are not application results. GCC debug also
passed ten related configuration, LOD, audio, client, font and root-cleanup
regressions selected in the slice plan. No full N2 suite was run for this slice.
The tested `native_headless_root.cpp` bytes have SHA-256
`b943582be0ecc4e0d9b7d8d6413f88e3713324627787540a8d9c1ec876cf403f`;
the production factory and CLI SHA-256 values are
`82e938c39be9df58904c0f1a179ce13a6f0f1835aacb4872f5704a161516a52d`
and `bab8c4ab2fccfd2b797096c9878dff83d7cc45c5414fa087b296381c8d4949ff`.

## Limit

This proves original post-init and retirement on a generated minimal input
cohort. It does not load H1, run original simulation ticks, prove canonical
checkpoint equality, exercise private retail assets, or accept N2. Those are
the next four slices and the N2 milestone gate.
