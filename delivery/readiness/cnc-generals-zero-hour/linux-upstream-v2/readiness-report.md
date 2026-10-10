# Readiness report

## Overall readiness status

READY_WITH_EXTERNAL_DEPENDENCIES. N2 remaining-work planning and generated implementation
can proceed now; private startup, later physical validation and clean release environments
have explicit owners and future verification boundaries. This is planning readiness.

## Scope and evidence basis

Complete `linux-upstream-v2` packet, all N0–N7 contracts and approved delivery order.
Definition transaction: `e0a32f1c92b113733bbe588cfc00aeb7b019e4d7`; audited packet revision
and finalized planning commit are in `workflow/delivery-planning/handoff.yaml`.
Transaction-start HEAD: `9b320660d8679f5f8bbcf690b8c50dcdca675f2a`.
All 17 pre-existing modified code/test/build/slice paths are preserved and unaccepted.
FACT: package hashes/passes/ancestry, repository setup and fresh configure outputs inspected.
INFERENCE: future physical/retail/release facilities are obtainable, not currently verified.
No independent-agent review, feature test, GPU test, private probe or milestone acceptance.

## Automatically completed prerequisites

PRE-01/05/10: tool/library discovery, three fresh headless configure variants, packet and
source validation. Corrected `docs/build.md` stale no-SDL assertion to match current CMake.
Added concrete future preconditions/checks to N2–N7; N0/N1 historical contracts unchanged.
No package installation, service startup, download or product-code mutation.

## Confirmation-gated work

None. No action requiring confirmation is necessary for this planning checkpoint.

## Required user actions

No immediate user action blocks N2 generated development. At the named boundary:
1. PRE-04: asset owner supplies readable lawful original roots privately before N2 retail
   startup acceptance; implementer validates original post-init/teardown/unchanged integrity.
2. PRE-03/PRE-12: host owner provides supported GPU/display/input/audio facilities before
   physical N3/N4 validation; normal-host capability and execution evidence clear the gate.
3. PRE-09: maintainer provides usable clean distribution environments before N7; verified
   clean build/install matrix clears it. Docker CLI presence is insufficient.
Detailed instructions and failure boundaries are in the prerequisite manifest.

## Missing or unresolved prerequisites

No unresolved authority or unowned provider. Native root, H1, checkpoint/negative harness,
retail startup wrapper and fresh remaining-work plan are N2-owned outputs (PRE-11).
Missing execution today is implementation work, not an excuse to manufacture a bootstrap M0.
If an external requirement fails at its boundary, stop that acceptance as blocked; do not waive it.

## Dependency graph

Provider adjacency (each arrow names its prerequisite; external nodes explicit):

- Repository/current tools → N0,N1,N2 [PRE-01]; N0 → N1,N2 [PRE-01 baseline].
- Consumer-owned stock acquire/verify → N1,N3,N4,N7 [PRE-02].
- External host GPU/session → N1,N3,N4 [PRE-03].
- External asset owner → N2,N3,N4,N5,N6,N7 at retail boundaries [PRE-04].
- Declared host development packages → N1,N2,N3,N4 [PRE-05].
- N1 + N2 → N3,N4 [PRE-06]. N3 + N4 → N5 [PRE-07].
- N5 → N6 [PRE-08]. External distribution facilities → N7 [PRE-09].
- Committed definition/planning → N2,N3,N4,N5,N6,N7 [PRE-10].
- N2-owned plan/native qualification harness → N2 acceptance [PRE-11].
- External display/input/audio → N4 physical acceptance [PRE-12].
- N6-owned local UDP facility/harness → N6 acceptance [PRE-13].
- N6 → N7 [PRE-14].

No cycles, backward providers or missing provider edges. Consumer-owned work is an
internal completion dependency, not a cyclic milestone prerequisite.

## Proposed milestone order

Declared and proposed order remain N0,N1,N2,N3,N4,N5,N6,N7. N0/N1 are already completed.
N1/N2 independent graph branches do not authorize bypassing default order or acceptance.
No compiler/readiness correction cycle was required.

## Milestone 0

Not required. Existing N0 is the approved completed recovery/baseline outcome. Do not
repeat its reset/archive operations. No shared repository bootstrap gap needs another M0.

## Per-milestone readiness

| Milestone | Preconditions/checks | Current result / blocker |
| --- | --- | --- |
| N0 | PRE-01; preserved status and named evidence | Completed history; no reset or archive reinspection |
| N1 | PRE-01/02/03/05; preserved stock gate evidence | Completed history; no current GPU acceptance claim |
| N2 | PRE-01/04/05/10/11; package, tools, configure, fresh plan | Ready to plan/implement H1; PRE-04 required at retail acceptance |
| N3 | PRE-03/04/05/06/10; N1+N2 evidence, upstream verify, host Vulkan | Await N2; physical and retail checks before acceptance |
| N4 | PRE-03/04/05/06/10/12; N1+N2, packages/session/media | Await N2; actual output validation remains required |
| N5 | PRE-04/07/10; N3+N4 original-runtime acceptance | Await providers; H1 is not match or persistence proof |
| N6 | PRE-04/08/10/13; N5 and local two-process UDP | Await N5; transport/harness belongs to N6 |
| N7 | PRE-02/04/09/10/14; N6 and clean facilities/upgrade | Await N6; clean env/install/input facilities unverified |

N2–N7 contracts now carry executable inspections and expected results. N0/N1 retain
existing readiness sections as completed history; status/evidence controls completion.

## Clean-environment simulation

Conceptual walkthrough, not an executed clean build or acceptance run:

1. Start from committed active source, tools/development packages declared in docs/build.md,
   approved baseline and definition; no recovery archive, assets, caches or hidden SDK.
2. N1's historical reproducible recipe acquires locked upstream sources into ignored output,
   builds supported tools and exercises physical GPU gates with declared host facilities.
3. N2 configures independently of bgfx; current CMake discovers SDL3/FreeType/Fontconfig,
   zlib/OpenSSL3 plus compilers. Fresh temporary configurations passed all three variants.
   N2 creates its actual root/H1 and evidence harness; assets are unnecessary until HQ-06.
4. N3/N4 consume accepted N1/N2 and provide full original world/interaction/media under
   physical and retail validation. Newly reached providers belong to their named outcomes.
5. N5 consumes those for complete gameplay/persistence; N6 provides local network integration.
6. N7 consumes accepted N6, acquires declared clean distro facilities and executes asset-free
   packaging, installed smoke and upstream upgrade regression under source/input constraints.

Every entry prerequisite has an earlier, local or explicitly external provider. Tools on a
clean machine require documented OS provisioning by its owner, not assumed ambient state.
Network acquisition, GPU/audio, actual retail startup and distribution builds were not run.
Current configure success includes preserved dirty code but does not certify its compilation.

## Commands run and results

- Installed compiler `validate_milestones.py status.yaml --allow-existing-status completed
  --allow-existing-status in_progress`: eight contracts/records passed.
- GCC/Clang/CMake/Ninja/Python version commands, pkg-config for declared libraries and
  tool-presence queries: passed; exact current versions in the prerequisite manifest.
- Three `cmake -S . -B <fresh-temporary-directory> -G Ninja -DCMAKE_BUILD_TYPE=Debug
  -DZH_BUILD_RENDERER=OFF -DCMAKE_CXX_COMPILER=<g++|clang++>` configurations, with
  `-DZH_SANITIZE=ON` for both sanitizer variants: all exit 0. Configuration only.
- Definition/artifact hashes, producer ancestry, link/graph/status preservation and
  dirty-byte checks: verified by planning validator before transaction closure.
- Git diff whitespace/staged ownership checks: required at final closure.
- Initial optional `rg --files scripts ...` discovery returned exit 2 because the repository
  has no scripts directory; installed skill validator used instead. No check weakened.

## Files changed

Readiness owns `docs/build.md`, this report, `prerequisite-manifest.md` and N3–N7
readiness sections. N2 compiler contract already carries the audited PRE-01/04/05/10/11
requirements/checks. Exact combined ownership is in `workflow/delivery-planning/readiness-result.yaml`
and `compiler-result.yaml`. Specialists make no independent commits.

## Remaining blockers

None for starting N2 remaining-work planning/generated implementation. PRE-04 is mandatory
before retail acceptance; PRE-03/12 and PRE-09 remain later external verification boundaries.
No ready status advances any implementation milestone or accepts existing dirty work.

## Exact recommended next step

In this repository, run `milestone-delivery-orchestrator` starting at N2 using
`workflow/delivery-planning/handoff.yaml`; first create a fresh minimal-slice governing
plan for remaining HQ-01–07 work and reconcile old slice03/resume pointers in delivery-owned
state. Preserve accepted slices and all pre-existing edits. Do not resume the old unbounded
slice directive. This planning run stops before invoking that downstream workflow.
