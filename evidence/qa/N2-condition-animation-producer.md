# N2 shared source condition/animation producer

Implementation batch parent f44b668adea375546d391d75827e296b4bbd1701;
documentation-only inventory checkpoint 5daba00f. Existing N2 plan01/slice03.
Supporting shared abstract producer, not concrete19, full startup or N2 acceptance.

## Behavior and ownership

NativeModelDrawAnimationProducer owns source selected/pending condition pointers,
weighted record index, deferred duration, pause/saved mode and current
model/clip/pose/bounds/clock as one staged candidate. Successful publication
commits the original namespace, asset admission and original client RNG together.
GameClientRandomTransaction snapshots the original six-word client stream without
allocation or a replacement algorithm; nested commit/rollback is covered against
public client draws and independent logic/audio streams.

Actual registered generated DrawModule instances exercise condition, manual
frame, pause/resume and duration callbacks with the NativeHeadlessDisplay clock
published through an original ScriptEngine and attached camera owner. Same-model
reuse retains captured bone controls, root and ObjectScale. Recreation resets
pause and ObjectScale, while empty animation lists on reused HLODs retain the
previous clip/clock. No-model and non-HLOD paths retain source completion and
RNG distinctions. The source consumes RANDOMSTART for existing leaf models even
though they ignore playback; only HLOD consumes the speed-factor draw.

Generated cases cover repeated weighted records, random/first/last precedence,
common maintain-frame -1, idle index rejection sampling, wait before explicit
transition, duplicate current/pending notifications, cancellation/restart,
completion-time partition and deferred target duration, source pause sync/mode/
rate reset, manual signed conversion rejection, worker/parent/phase/root rejection,
and zero-standard-demand ordinary ticks. Unsupported effect application is
explicitly rejected, never accepted as a successful empty stub.

Cold initialization and seeded same-model candidate sweeps derive exact standard
allocation terminals, exercise every failed prefix and corrected same-owner
retry, and verify accepted selected/pending/index/clock/binding identity, asset
counts, standard live ownership, namespace pool used units, client cursor and
namespace cursor before fixture rollback. Producer operations do not acquire
Drawable IDs. The identical generated ScriptEngine/Display/camera/producer
cohort is warmed before complete-owner baselines. Three whole owner lifetimes
are retained in every query family.

A late invalid speed factor is rejected after source index/start RNG draws and
preserves the complete producer/client cursor. Its corrected retry proves exact
selected index/frame/rate. The original RNG API's defined std::invalid_argument
is caught explicitly by this malformed-factor test; unrelated exception classes
are not broadly accepted.

## Investigation history

The first weighted selection oracle tried to advance an idle clip which
RANDOMSTART could already place at the endpoint. Source completion precedes idle
reselection, so its test multiplier then applied to an animation already
replaced. The corrected fixture advances only an incomplete retained selection;
completion/reselection rules and RNG algorithm remain unchanged.

Expanded lifecycle review found a real parallel-path cursor gap: early return
for a leaf skipped its source randomized start draw. That was corrected as part
of this coherent batch, with independent no-model/leaf/HLOD checks. The new late
rejection test initially caught only engine ErrorCode/asset errors rather than
the original RNG API's std::invalid_argument; a source call-stack check established
the expected type. No assertion/full-build loop, timeout increase, leak suppression
or production semantics change was used to repair either fixture issue.

## Frozen validation

Focused producer/core12 PASS35.54s. Frozen canonical726 SHA256
f609dff2eaad783ca7fb6f484e669e98f73d91c16716c6399de03533d8f0f13f;
toolchain-inclusive733 SHA256
631a391a7fb17ccfd8f7e76d68f40acbdc772b82e40702b6111c360cbfb0864f.
Fingerprint procedure matches N2-drawable-queries.md relative to9e5c0e4f;
toolchain exclusions are tests/toolchain/. Documentation/status are excluded.
Related128 uses the same target/selector cohort as N2-runtime-frame-domain.md,
including seven new producer families and original core RNG checks.

Related128 PASS GCC149.73s, GCC ASan/UBSan/LSan584.05s and Clang
ASan/UBSan/LSan372.56s. All three processes exited0; no build/test jobs remain.
Producer terminals cold223/seeded56 preserve every failure/retry across three
lifetimes. All278 normalized prefix/terminal/retry manifest lines agree across
the three configurations, SHA256
b241d54105b75410c6bf43cd3fe4a8203664044371fadf35464891e54e8e1a93.
Normalization removes only CTest's numeric output prefix and retains each row
in source order. Both source fingerprints were rechecked unchanged after the
matrix. These results validate this supporting component cohort, not the
remaining integration gates below.
Logs: /tmp/n2-condition-producer-<configuration>-frozen-{build,related}.log.
All leak checks and independent60s family timeouts remain enabled.

The first sanitizer matrix attempts used the restricted sandbox. LeakSanitizer
reported fatal thread-inspection/ptrace limitations even for original_core_values
in both toolchains; these are invalid environment results, not passing ownership
evidence or source leak diagnoses. The identical core control passed outside the
sandbox for both compilers with leak checks still enabled. Only the two verified
sandbox CTest jobs and their active fixture children were terminated (sessions
retired143). The unchanged already-built matrix is now running on the normal
host and passed. No sanitizer suppression, ptrace_scope mutation, timeout change or rebuild
was used. Keep the failed-environment logs above; normal-host results are in
/tmp/n2-condition-producer-<sanitizer-configuration>-host-frozen-related.log.

## Remaining gates

Configured full465 is unrun. This producer is abstract and generated module
callbacks are narrower than actual multi-module Drawable condition walks. The
outer dirty/forced/immediate walks still need whole-walk publication/rollback;
source findings and exact functions are in docs/original-engine-formats.md.
First-leaf constructor root admission also needs unconditional finite conversion
and malformed/corrected retry coverage before concrete creators are enabled;
current generated first roots are finite, so this boundary is not accepted.
Source movement/Physics, getShouldAnimate, constructor-wide weather/time
validation, turret/recoil/material/hide-show/particle/audio/event ownership,
specialized concrete19 creators, camera actor-follow, complete native startup
and loaded deterministic simulation remain pending in the existing slice03.
No physical render, GPU, retail visual parity or full simulation claim.
Retail assets, adopted dependencies and recovery storage are untouched.
