# N2 particle-admission worktree checkpoint

User-requested worktree review and commit, parent
1ea25c4e611bef7315a392c2372245d6c7fdaf49. This retains coherent unfinished
N2 plan01/slice03 work; it is not a new slice or completed milestone.

The initial inventory was four modified and three untracked files: original
ParticleSys header/manager, native admission header/source, CMake registration,
generated fixture and existing slice plan. All belong to the same ownership
batch. The review adds engine findings, this evidence and delivery resume state.
No unrelated nonignored edits were found. No deletion, reset, stash, recovery
import, retail asset edit or upstream-library modification occurred. Existing
ignored build trees, Python caches, retail link and recovery archive remain
ignored and uncommitted.

## Included behavior

Complete parsed particle chains acquire unpublished pooled candidates
parent-first, preserving IDs and source client RNG, and publish child-first,
preserving original manager order. Cycles reject before construction; missing
slaves retain leaf behavior. Guards retain candidates through fallible list
publication. Nested creation savepoints restore list/count/IDs, source RNG and
lazy template caches, including when a caught nested failure precedes outer
commit. Uncommitted creations retire immediately; accepted destroy remains
deferred. Valid unsigned initial-delay conversions retain truncation, while
negative/nonfinite/out-of-range inputs reject before undefined conversion.

The six test families cover actual source parsers/managers/clients, three
complete lifetimes, primary/child/grandchild and 16-node chains, no-slave/missing
paths, cycles, malformed delays, wrong worker/manager/client, replacement client
identity, uncommitted and locally caught nested admission, exact standard
failure/retry prefixes, bounded typed-pool exhaustion and whole-owner retirement.
Standard route terminals are 11/11/5; typed-chain terminal is 3. Every corrected
standard retry proves identical cold-cache demand, not just eventual success.

## Current-source focused validation

Frozen builds and focused6 pass in all three configurations:

- GCC: 0.51s, exit0.
- GCC ASan/UBSan/LSan: 4.22s, exit0.
- Clang ASan/UBSan/LSan: 2.41s, exit0.

All owned build/test processes terminated. Sanitizer CTest runs used the normal
host because restricted thread inspection prevents sandbox LSan execution.
Leak detection and 60-second family limits stayed enabled; no suppressions or
timeout increases. Selector: `^original_particle_admission_`; build target:
`original_particle_admission_fixture`, in the original-core GCC, GCC sanitizer
and Clang sanitizer configurations. Logs:
`/tmp/n2-particle-checkpoint-gcc-tests.log`,
`/tmp/n2-particle-checkpoint-{gcc,clang}-sanitize-{build,tests}.log`.

Canonical731 SHA256
37c0d390bacd2fa229d9e7073ec7614e90f2220da8032fe156bfb66da9e5693a;
toolchain-inclusive738 SHA256
8d948073057590ef9211a9ca615d52527f54a37615c5d79e636380594208ff15.
Fingerprint procedure is the existing changed-source procedure relative to
9e5c0e4f. All three normalized allocation manifests match: 18 lines, SHA256
1199bc473ebbc72a9670940af385863026e6264af2a168b765eb1801b1e1c411.

Earlier development failures were classified, not waived: the new standalone
header consumer exposed a missing File forward declaration; bounded exhaustion
initially used the growing production pool; replacement-client construction
initially collided with the existing DrawGroupInfo owner. The header declaration
and public, isolated fixture setup were corrected. Final six-family runs cover
both failure/retry paths and legitimate replacement-owner cleanup. Earlier logs
remain in `/tmp/n2-particle-admission-*`; they are not final acceptance.

## Limits and resume

The production pool remains initial1024/overflow32. The fixture explicitly uses
the supported public initial64/overflow0 configuration to test bounded pooled
failure; production growing-pool capacity behavior is not accepted here.
Savepoints cover creation, not deletion/mutation of pre-existing systems, and
require valid stable manager/client/configuration/template lifetimes. Fixtures
use parsed authored particle INI and constructor-valid client frame state, not
complete GameData/GameClient startup. Graphics abstract calls reject, not stub
successful rendering. Saved-graph validation and full particle simulation are
outside this evidence.

Continue the same N2 plan01/slice03. Before component acceptance, freeze and run
the related144 matrix (previous related138 plus these six families) in GCC and
both compiler sanitizer configurations. The earlier condition-walk matrix is
provenance for its committed source, not validation of this changed header and
manager. Then integrate concrete19 tracker/bone/effect and physics/AI/geometry
parents, lazy event semantics, camera actor-follow and actual startup/loaded
simulation. Configured full481 is unrun. Slice03 and N2 remain incomplete.
