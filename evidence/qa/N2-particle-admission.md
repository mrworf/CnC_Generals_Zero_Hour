# N2 particle admission: frozen related validation

Existing N2 plan01/slice03; parent8123f8cc, implementation checkpoint
a3590548bbed558e9c53e4a3b3cb968dd03a2df1. This closes the pending related matrix
from N2-particle-admission-checkpoint.md, not N2 or slice03 acceptance.

## Unchanged source and coverage

Canonical731 SHA256
37c0d390bacd2fa229d9e7073ec7614e90f2220da8032fe156bfb66da9e5693a;
toolchain-inclusive738 SHA256
8d948073057590ef9211a9ca615d52527f54a37615c5d79e636380594208ff15.
Changed-source fingerprint procedure remains relative to9e5c0e4f, as documented
in N2-condition-animation-producer.md. Both hashes were verified unchanged after
all three runs. Source, test and build registration changes were frozen;
only documentation/resume records changed during execution.

Related144 is the previously recorded model/Drawable/data/core/header/module/
template/runtime/client/font cohort plus the six particle-admission families.
It covers the changed ParticleSys shared header and original manager together
with parsed particle acquisition, whole condition-walk and query consumers.
Selector:
`^original_(particle_admission_|draw_provider_data_|drawable_queries_|w3d_(draw_data|assets|animation|document)_|data_|core_|header_paths$|graph_headers_|module_data_|template_backing_|runtime_(names|floating-point)|native_client_|native_font_composition)`.
Build targets are the existing cohort from N2-condition-animation-producer.md
plus original_particle_admission_fixture, with no framework or asset changes.

## Observed results

- GCC related144: PASS235.19s, exit0.
- GCC ASan/UBSan/LSan related144: PASS941.46s, exit0.
- Clang ASan/UBSan/LSan related144: PASS574.64s, exit0.

All three related-target builds completed successfully. No owned build/test job
remains live. Sanitizer tests ran on the normal host, with leak checks and
existing independent60s family limits enabled and unchanged. There were no
failures, timeouts, suppressions or production edits during the matrix.
Logs: `/tmp/n2-particle-<configuration>-frozen-build.log`,
`/tmp/n2-particle-gcc-frozen-related.log`, and
`/tmp/n2-particle-<sanitizer-configuration>-host-frozen-related.log`.

All320 normalized allocation prefix/terminal/retry rows agree across all three
configurations, SHA256
30f2f6a5c2d2888501cf83af9a83afc1c38c4b0175161ea47e1994f6030b393c.
Normalization removes only the numeric CTest prefix, preserving row order.
Particle standard terminals11/11/5 and typed-chain terminal3 retain every
failure/retry, cold-cache demand and three complete generated owner lifetimes.
See the checkpoint evidence for exact fixture setup, earlier resolved fixture
failures and production-domain limitations.

## Limits and next integration

This validates the implemented creation-only manager/savepoint component and
its related consumers. It does not validate production growing-pool capacity,
old-system mutation/deletion rollback, concrete19 factory/tick/effect admission,
saved particle graphs, complete particle simulation, rendering, or full original
startup/loaded simulation. Configured full481 remains unrun, slice03/N2 remain
incomplete. The earlier condition-walk evidence remains valid for its source;
this new matrix additionally covers the changed particle header/manager.

Source review for the next coupled batch is durable in original-engine-formats.md
and the existing slice03 plan: source-delayed state-particle creation; old tracker
retirement deferred through condition commit; whole-callback offside tracker
admission; distinct initial scaled model-space versus animated world-space bone
sampling; borrowed template/manager/client/Drawable lifetimes; and lazy clock/
hierarchy/embedded-event behavior. Proceed with the planned growing-pool test,
then concrete tracker/bone/effect and real physics/AI/geometry integration,
camera actor-follow and actual original startup. No replacement plan or milestone
was introduced, and no upstream reconciliation is required by these settled
source-contract findings.
