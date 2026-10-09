# N2 camera/View/Drawable lifetime checkpoint

Status: coupled lifetime checkpoint validated and committed.
Payload commit: `02e437e0748dc8744d582d3bdc056dc8e45999b2`.
Existing N2 plan01/slice03 continues; concrete frame/constraints/model/shake/
slave/UI/root and full369 remain pending. No framework, retail asset or recovery
file was changed.

Transaction parent `e13b9605d43b36474a3caf3c8b98ed7a99e1a29b`; initial tree clean.
The existing slice plan was refined before production edits. Current source:
canonical646 SHA256
`d86dd51f842f010d51ed6911aab8262929414417ee8c1839f0020e412426716e`;
inclusive653 SHA256
`690c7e7a115426af5f808603d6f75ce6371e47069dbcf90ce3bea9e4249a7214`.
Same source digest recipe as the navigation checkpoint;369 configured tests.
These fingerprints remained unchanged throughout the frozen broader matrix.

## Coupled behavior

Game-owned live-View registration/counting and exact retirement; all-live-View
Drawable borrow withdrawal at actual client and Drawable terminal paths, without
callbacks/allocations or dropping surviving Object lock IDs. Display null,
duplicate and cross-owner admission rejects before infallible ownership linking;
direct and whole owned-View retirement preserve valid tactical publication through
derived destruction, withdraw only the matching publication and retain unrelated
borrows. Live-chain registration is non-owning and receives exact count checks;
sanitizer reachability is not used as a substitute for ownership retirement.

Generated actual native spatial Views/Display ownership compose original client/
Drawable reset, cancellation, direct and whole retirement. Existing original
Object creation fixtures now prove Object survival across Drawable replacement
and callback-acquired camera borrow withdrawal in both binding failure phases.
All absent physical/frame/model methods explicitly reject in test adapters.

## Focused iteration history (all jobs terminal)

```sh
cmake --build build/original-core-<configuration> --target original_native_client_fixture original_logical_world_fixture original_script_camera_fixture -j 4
ctest --test-dir build/original-core-<configuration> -R '^original_(native_client_(camera-lifetime|view-ownership)|logical_world_factory-creation-(binding|camera)|script_camera_)' --output-on-failure
```

| Configuration | Exact job | Build/focused13 |
|---|---:|---|
| GCC | 24349 terminal0 | PASS13/14.98s with final count assertions |
| GCC ASan/UBSan/LSan | 36152 terminal0 | PASS13/64.15s before final count-assertion rebuild |
| Clang ASan/UBSan/LSan | 59586 terminal0 | PASS13/36.69s before final count-assertion rebuild |
| Clang ASan/UBSan/LSan | 92023 terminal0 | PASS13/36.31s with final count assertions |

Earlier GCC compile iteration16726 completed0 before the last source/test edits;
it is not current focused evidence. Logs `/tmp/n2-camera-lifetime-gcc-build.log`.
Current logs `/tmp/n2-camera-lifetime-gcc-build-final.log`,
`/tmp/n2-camera-lifetime-{gcc-sanitize,clang-sanitize}-build.log`, and
`/tmp/n2-camera-lifetime-{gcc,gcc-sanitize,clang-sanitize}-focused.log`.
Jobs were polled without restart on observation yield. Sanitizers/leak checks remained
unsuppressed with timeout60 per family. Diagnose a failure across its lifecycle
and parallel paths before corrections. After all3 focused passes, freeze the
coherent batch and run required full builds/related ownership/world matrix before
commit. Full369 remains the unrun N2 completion gate.

Source was frozen for full configured builds, focused13 and related ownership/
world regression checks: GCC95176, GCC sanitizer62601, Clang sanitizer83544.
Logs `/tmp/n2-camera-lifetime-<configuration>-frozen-{build,focused,related}.log`.
The frozen GCC sanitizer build includes the final repeat-count assertions.

All frozen jobs completed terminal0, full configured builds PASS:

| Configuration | Job | Focused13 | Related179 |
|---|---:|---:|---:|
| GCC | 95176 | PASS15.13s | PASS175.35s |
| GCC ASan/UBSan/LSan | 62601 | PASS64.66s | PASS771.03s |
| Clang ASan/UBSan/LSan | 83544 | PASS36.46s | PASS465.61s |

No failing test, sanitizer finding, timeout, suppression or coverage waiver was
observed. Full369 has not run and remains a milestone completion gate. This
checkpoint accepts only the coupled lifetime behavior described above, not N2
completion, concrete camera frame behavior or whole-game parity.

Source/lifetime findings: `docs/original-engine-formats.md`.

First GCC focused iteration29897 passed13/14.99s. Exact View-registration checks
were then added to every camera/logical-world repeat and final GCC24349 passed
13/14.98s. Final counts logs use `*-counts-{build,focused}.log`. Prior source
fingerprints canonical646/53f45db8 and inclusive653/1089be9a are superseded by
the current values above. No development failure or coverage waiver observed.
