# N2 plan02 slice 04 — Failure, corrected retry and ownership

## Goal and observable outcome

Every specified H1 startup/admission failure reports non-success, retires partial owners and accepts corrected input in the same process.

## Scope and non-scope

Included: Missing/malformed/provider/late-allocation control; exact standard and pooled fault terminals; same borrowed configuration identity; valid parent cleanup and interruption handling.
Excluded: No invented universal transactional rollback after a mutated simulation tick.
This delivers only N2 R07 qualification, never later milestone acceptance.

## Dependencies and ordering

Plan02 slice 03 completed and committed. Exact contract: HQ-05, HQ-07 in `product/headless-qualification.md` and the
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

Positive corrected H1 retry and terminal; negative every failure ordinal, malformed required INI/truncated map/missing provider; GCC and Clang sanitizer owner checks. Preserve every failure/retry pair and scoped resource baseline.
During implementation run focused affected tests first; after stabilization run
related tests. At milestone end run the complete required frozen matrix.
Use `cmake -S . -B build/original-core-gcc -G Ninja -DCMAKE_BUILD_TYPE=Debug
-DZH_BUILD_RENDERER=OFF`, analogous GCC/Clang sanitizer build directories with
`-DZH_SANITIZE=ON`, `cmake --build`, and `ctest --test-dir ... --output-on-failure`.
Target-specific commands must be written here once tests/targets exist; no
command absent from repository evidence is claimed as already runnable.
A timeout or unavailable private input is incomplete, never a pass.

## Expected implementation surfaces

`cmake/OriginalRuntime.cmake`, original GameEngine/GameLogic/native provider
source as required by the reached call path, `tests/original/`, `tools/`,
`docs/build.md`, and a public `evidence/qa/` report. Inspect the 17 dirty
pre-existing paths before touching overlapping files; stage only owned hunks.

## Acceptance criteria

- [ ] Every specified H1 startup/admission failure reports non-success, retires partial owners and accepts corrected input in the same process.
- [ ] Positive and negative behavior above is directly demonstrated by
  executable source or a bounded public report, with current source identity.
- [ ] Ownership, input integrity and accepted existing regression guarantees
  remain intact; no private data is committed.

## Commit boundary

One reviewable behavior commit when all slice checks pass. Include this updated
slice plan, directly required code/tests/docs/evidence and an updated governing
index. Do not stage unrelated or unaccepted pre-existing dirty work.
