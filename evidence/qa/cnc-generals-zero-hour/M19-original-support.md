# M19 original-support evidence

Evidence grade is stated per result. Retail data, GPU/display access and the read-only retail symlink are outside this milestone and were not accessed.

## Slice 01 — source classification and identity contract

Grade: source-inventory assurance (not original-source runtime).

The canonical CMake inventory resolves to 3,293 unique source paths and the generated table classifies each exactly once. At this checkpoint 2,353 are deliberately excluded/deferred to their named source-integration owner, 857 are tool-only, and 83 are platform-replaced. No source is yet claimed as production-compiled; slices 02 and 03 promote the accepted original support providers after compiler, link and runtime proof exists.

Validation:

- `python3 tools/original_source_classification.py --check` — passed, 3,293 paths.
- `python3 -m unittest tests.original_support.test_source_classification` — passed, four positive/negative cases.
- `ctest --test-dir build/linux-gcc-debug -L 'original-support|source-identity' --output-on-failure` — passed, 2/2.

The negative controls reject a missing inventory policy and production providers whose path identifies bootstrap, toy or fixture code. The committed table contains repository-relative source names only; it includes no retail path, bytes or hashes.

## Production closure

Incomplete by design at this checkpoint. Original compression and WW support compiler/link/runtime evidence follow in slices 02 and 03. Original GameEngine and device providers remain owned by M20–M25; fixture/component results from M0–M14 do not establish their integration.
