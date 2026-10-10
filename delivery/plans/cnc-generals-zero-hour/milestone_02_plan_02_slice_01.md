# N2 plan02 slice 01 — Original native entry and root admission

## Goal and observable outcome

An actual native executable documents its modes, initializes the original GameEngine on generated root inputs, reaches post-init, and retires its complete owner graph.

## Scope and non-scope

Included: Native GameEngine subclass/factory and CLI; complete reached startup services; original INI/CSF and separate user storage; reverse cleanup with valid singleton parents.
Excluded: No map/world loading, gameplay ticks, physical output, retail parity or successful link-only probe.
This delivers only N2 R07 qualification, never later milestone acceptance.

## Dependencies and ordering

Accepted plan01 core/data slices; none in plan02. Exact contract: HQ-01, HQ-04, UX-HQ-01/02/04 in `product/headless-qualification.md` and the
current N2 milestone. Preserve accepted plan01 core/data components and
apply current source-format/ownership findings from `docs/original-engine-formats.md`.

## Entry point and application behavior

The documented native headless entry or its actual original-runtime test
invocation exercises this outcome end to end. It must use original factories,
parsers and owner lifecycle, never a replacement simulator or link-only probe.

## Data, state and permissions

Ordinary local user; read-only generated/private data and separate user storage; no remote account or elevated privilege. Record source state before/after relevant transitions and retire acquired
ownership through legitimate source cleanup. Generated test bytes may be
committed; retail bytes/derivatives and private selectors must not be.

## Validation, errors and recovery

Positive generated-root original post-init and repeated clean shutdown; negative missing root, required INI/provider and storage refusal; scoped fault/owner tests. Preserve every failure/retry pair and scoped resource baseline.
During implementation run focused affected tests first; after stabilization run
related tests. At milestone end run the complete required frozen matrix.
Use `cmake -S . -B build/original-core-gcc -G Ninja -DCMAKE_BUILD_TYPE=Debug
-DZH_BUILD_RENDERER=OFF`, analogous GCC/Clang sanitizer build directories with
`-DZH_SANITIZE=ON`, `cmake --build`, and `ctest --test-dir ... --output-on-failure`.
Target-specific commands must be written here once tests/targets exist; no
command absent from repository evidence is claimed as already runnable.
A timeout or unavailable private input is incomplete, never a pass.

The reached target and focused selector are now runnable in each configured
`build/original-core-{gcc,gcc-sanitize,clang-sanitize}` directory:

```sh
cmake --build build/original-core-gcc --target zero_hour_headless original_native_headless_root_fixture -j 4
ctest --test-dir build/original-core-gcc -R '^original_native_headless_' --output-on-failure
```

The same build and CTest commands with the other two directory names passed
in host execution context. The related GCC debug selector passed 10/10:
`^original_(environment_(root-prefix|ownership)|native_font_(composition-root-reset|manager)|audio_admission_(functional|lifetimes)|native_client_(functional|tactical-startup)|lod_(functional|negative))$`.

## Expected implementation surfaces

`cmake/OriginalRuntime.cmake`, original GameEngine/GameLogic/native provider
source as required by the reached call path, `tests/original/`, `tools/`,
`docs/build.md`, and a public `evidence/qa/` report. Inspect the 17 dirty
pre-existing paths before touching overlapping files; stage only owned hunks.

## Acceptance criteria

- [x] An actual native executable documents its modes, initializes the original GameEngine on generated root inputs, reaches post-init, and retires its complete owner graph.
- [x] Positive and negative behavior above is directly demonstrated by
  executable source or a bounded public report, with current source identity.
- [x] Ownership, input integrity and accepted existing regression guarantees
  remain intact; no private data is committed.

## Commit boundary

One reviewable behavior commit when all slice checks pass. Include this updated
slice plan, directly required code/tests/docs/evidence and an updated governing
index. Do not stage unrelated or unaccepted pre-existing dirty work.
