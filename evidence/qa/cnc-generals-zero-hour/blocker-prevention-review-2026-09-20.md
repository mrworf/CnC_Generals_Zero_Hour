# Remaining blocker-prevention review

## Scope

Baseline `dfa5d92276a6c56beab936bc3d60b20184b282cd`; further owner-requested planning audit. Inspected new prerequisite contracts, original MemoryInit/GameMemory/UnicodeString bootstrap, retained source dependency evidence, final playable/release contracts and delivery adoption metadata. No original retail access, product implementation or runtime acceptance. Reuse prior source findings rather than claiming a fresh exhaustive audit of every translation unit.

## Executive Summary

Keep the new dependency order. Two narrow gaps need correction: allocator configuration must not depend on the later game filesystem, and release wording must not promote old component GPU evidence. RC-011 resolves them in existing owners. Known later external gates remain; source inspection cannot guarantee a blocker-free port.

## What Is Good

Good: M26–M28 now produce independently testable original providers before M20 integration. Full INI callbacks, registry-backed map metadata and complete GameClient remain with the coupled owner. The explicit pending membership and dependency order retain every unfinished milestone without renumbering history. Original source identity, failure tests and source-change invalidation are already requirements, not optional documentation.

## What Is Bad Or Risky

- Risky: `GeneralsMD/Code/GameEngine/Source/Common/System/MemoryInit.cpp:754–805` reads optional MemoryPools.ini using stdio and specifically forbids game filesystem/AsciiString because allocation setup precedes them. Routing this through M27 would make M26 cyclic. Its fixed buffers/unbounded name scan and integer counts also require bounded native handling before trusting overrides.
- Risky: `delivery/milestones/cnc-generals-zero-hour/M17-release.md` still says Arch M14 hardware evidence satisfies GPU acceptance and requests replay of M14–M16. That conflicts with the later original-source requirement already recorded in the same contract and migration authority. M22/M23 and original integrated M15/M16 are the required evidence.
- Risky: CMake installs shader/project files (`CMakeLists.txt:296–304`), but installation rules alone do not prove the original executable resolves them outside the build tree. The existing M17 runtime gate must explicitly use the installed executable with source/build resources unavailable to lookup.
- Risky: GPU/session availability, clean Ubuntu/Fedora environments, actual required asset/backend compatibility and long-running determinism remain future validation boundaries. A planning pass cannot mark these passed or eliminate them by relabeling ownership.

## What Should Change

Change: RC-011 makes pre-filesystem pool configuration an explicit M26 contract with acyclic, allocation-safe defaults/optional override handling and boundary tests. It corrects M17 hardware evidence and clarifies installed-prefix execution. Compiler/readiness must preserve the current order unless their audit finds a concrete structural defect. Recheck full mandatory coverage and dependency closure against the adoption handoff; do not start implementation during this review.

Read-only YAML validation of status.yaml and handoff.yaml passed: 29 unique milestones, 16 completed, 12 mandatory remaining and one optional. Every incomplete milestone occurs exactly once in the pending execution order; mandatory/optional membership matches the milestone records, every dependency precedes its consumer, no mandatory milestone directly depends on an optional one, and handoff membership matches. This verifies bookkeeping, not unimplemented source behavior. Compiler/readiness validate the final revised packet separately.

## What I Would Not Change Yet

Do not add more milestones for these bounded obligations, repeat completed component acceptance, install host packages, acquire hardware or change product scope. Do not attempt to remove all future risk through repeated prose-only audits. Build/link/runtime evidence during the already-authorized provider milestones is the next stronger test of the plan.

## Overall Opinion

The revised plan is reasonable to start once RC-011 is propagated and readiness is refreshed. This is a statement about assigned work and executable ordering, not proof that future original-engine tests will pass. Any new compiler/link/runtime dependency must update the checked ledger and its owning scope; genuine product/backend or external-environment blockers remain explicit stop conditions.
