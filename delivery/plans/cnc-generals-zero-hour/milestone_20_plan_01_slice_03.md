# M20 plan 01 slice 03: identity, process isolation and evidence

Status: cancelled pending upstream replanning. Identity evidence against a substitute lifecycle would be misleading and is prohibited.

## Goal and outcome

Close M20 with reproducible proof that original `GameMain.cpp` and `GameEngine.cpp` compile, link and execute through the Linux factory/data lifecycle across supported presets without GPU/window acquisition or private-data leakage.

## Scope

Add compile/link/runtime identity controls, an isolated process test from an arbitrary working directory, optional gated retail initialization, lifecycle evidence, and any narrowly required corrections exposed by milestone validation.

## Non-scope

Do not advance M21 simulation or later presentation, persistence, LAN or release behavior. Retail validation remains optional and never changes the asset-free default suite.

## Dependencies and ordering

Requires slices 01–02. It is the final M20 slice and must not weaken their failure semantics.

## Entry point and behavior

The dedicated lifecycle executable calls `GameMain`, emits logical source/stage witnesses, and exits with a distinct nonzero status for initialization failure versus the successful M20 simulation-unavailable boundary. Identity tooling rejects missing original providers or missing witnesses.

## Data/state transitions

Process evidence records only derived stage names/counts and validation outcomes. Temporary XDG/fixture state is isolated and contains no retail content.

## Authorization

No authentication applies. Retail validation runs only when explicitly configured by the existing CMake gate and treats user roots as read-only.

## Validation and error handling

Run focused source identity and process tests, all four preset builds/full asset-free suites, and Clang ASan/UBSan original-lifecycle tests. Confirm missing-provider and malformed-data controls fail, arbitrary CWD succeeds, and no display/GPU variables or APIs are required.

## Expected surfaces

Identity/process tests and tools, CMake registration, governing/slice completion records, and `evidence/qa/cnc-generals-zero-hour/M20-original-lifecycle.md`.

## Validation commands

- Build and CTest all four native presets.
- Build `original_lifecycle_tests` and related process target under `linux-clang-sanitized`; run label `original-lifecycle` with ASan/UBSan.
- Run source classification and identity negative controls.
- If explicit retail roots are configured, run only the gated M20 initialization test and record logical/derived evidence.

## Acceptance and commit boundary

All M20 criteria map to concrete original-source integration evidence; all mandatory preset and sanitizer checks pass or a precise genuine external blocker is recorded. Commit evidence and validation tooling with this completed slice.
