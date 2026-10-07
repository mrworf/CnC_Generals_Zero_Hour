# Readiness report

## Overall readiness status

READY_WITH_EXTERNAL_DEPENDENCIES. N0/N1 can execute now; retail-content completeness
and clean distribution facilities remain explicit later-boundary prerequisites.

## Scope and evidence basis

Complete linux-upstream-v2 packet, source commit d0483ca911b029460fb46d878fe260e90ec8d071.
The new graph replaces the old graph by explicit user authority. Hardware/tool
checks are observed; future feature acceptance is not.

## Automatically completed prerequisites

PRE-01–03,05 checked with read-only commands. No system packages were installed.
The authorized N0 reset/recovery is separately evidenced, not a readiness repair.

## Confirmation-gated work

None. The user already approved the reset and new implementation; no agents.

## Required user actions

PRE-04: provide the original base-game/Zero Hour data if N2 validation reports
missing content; never upload it into the repository. Existing link is retained.
PRE-09: provide a working clean host/container environment if N7's setup check
cannot acquire one. Neither currently prevents N1.

## Missing or unresolved prerequisites

No unowned entry prerequisite. Stock bgfx suitability is N1's explicitly required
output; it is not falsely treated as known acceptance. A failed N1 requires
architecture reassessment rather than a dependency patch.

## Dependency graph

N0 --PRE-01--> N1 and N2.
N1 + N2 --PRE-06--> N3 and N4.
N3 + N4 --PRE-07--> N5 --PRE-08--> N6.
N6 plus external PRE-09 --> N7.
External PRE-04 enters N2/N3/N4/N5 at their retail checks.

## Proposed milestone order

Declared and audited order: N0,N1,N2,N3,N4,N5,N6,N7. No cycle, missing provider or
backward dependency. Shared constraints R02–R05 apply across consumers.

## Milestone 0

Not required. N0 is the explicitly authorized recovery/specification outcome,
not a readiness-created generic bootstrap bucket.

## Per-milestone readiness

N0: archives verified and baseline restored; packet closure completes the outcome.
N1: tools, network, GPU and validation available; build/fixtures are N1 outputs.
N2: own original-runtime toolchain/data admission; PRE-04 checked before private use.
N3/N4: wait for both N1 and N2 accepted outputs.
N5: wait for original world and interactive/media integration.
N6: wait for playable deterministic process; own UDP entry diagnostic.
N7: wait for N6 and verify declared distribution facilities.

## Clean-environment simulation

Start with the original tree plus new committed docs, declared tools, system
development packages and official network access. N1 writes isolated build/output
directories and pins/compiles stock tools/runtime; no archived libraries are used.
N2 supplies original-source build/data validators. N3/N4 consume both contracts,
N5 combines them, N6 adds networking, and N7 rebuilds/packages in clean environments.
This is a dependency simulation, not a claim those implementations exist.

## Commands run and results

git bundle verify and original-subtree diff passed.
gcc/clang/cmake/ninja/python version checks and all declared pkg-config checks passed.
vulkaninfo and official git ls-remote failed in sandbox, then passed outside it.
Docker CLI exists; no daemon/image or retail-content probe was run.
The compiler's structural validator is run before planning commit.

## Files changed

New milestone packet, requirement traceability, this report and prerequisite
manifest, plus planning state/handoff. No readiness feature code or OS mutation.

## Remaining blockers

None for N1. PRE-04 and PRE-09 remain later external verification boundaries.

## Exact recommended next step

In the repository, create the N1 governing/slice plan, acquire pristine official
bgfx/bx/bimg into a fresh ignored build directory, and run the suitability gate.
