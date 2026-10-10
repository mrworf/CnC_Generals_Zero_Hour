# N2 condition-walk implementation checkpoint

User-requested sorting/commit review, starting at 926c41d9. This is unfinished
N2 plan01/slice03 work, not a new plan or completed slice/milestone.

The initial nonignored inventory was eight modified and two untracked files.
All ten are coupled original-runtime implementation, build, fixture or plan
changes. No unrelated nonignored work was found. No deletion, stash, reset,
recovery import, asset edit or upstream-library modification was performed.
Generated build trees and disposable logs remain outside tracked source.

## Included behavior

NativeModelDrawConditionBatch enrolls initialized source producer owners from
raw module lists. It retains exact previous accepted current/validated owners
on publication and snapshots metadata, namespace, assets and client RNG.
Drawable dirty/forced walks keep original notification order; a later error
restores the cohort without allocating or calling simulation. Deferred flags
remain requested; forced flags and dirty publication recover their earlier
boundary. Pristine dispatch opens the root before dirty notifications and keeps
it through query/cache admission. First-leaf initialization stages finite root
conversion before accepting ownership.

Pause, manual frame, duration and ordinary animation advancement now stage
copy-on-first-mutation when inside an active root. Outside such a scope the
existing ordinary fast path remains. This prevents callbacks from mutating the
accepted object before its owner is retained. Dedicated nested callback and
locally caught rejection cases remain pending; existing walk tests do not
establish their complete acceptance.

## Validation boundary

Current-checkpoint builds and focused8 PASS in all three configurations:
GCC63.34s, GCC ASan/UBSan/LSan251.48s, Clang ASan/UBSan/LSan148.00s;
all processes exited0, no live build/test sessions remain. Diff whitespace
checks pass. Canonical728 SHA256
ad4d121b8b2b532baa34e6a07ba491304e146f0b38a43a958ea2e2f23a6dc0a5;
toolchain-inclusive735 SHA256
7f66720a6c4c78f2b014312739cbd974ca3a721b5faa8abaf680befd9720d317.
Fingerprint procedure matches N2-condition-animation-producer.md relative to
9e5c0e4f. No source changes occurred during these three runs. The current
sanitizer logs retain three whole-owner manifests per route, dirty119,
forced119/query164, with every failure/retry. This focused evidence does not
replace the pending related matrix or the missing nested-mutation cases.
Earlier eight-family ownership results (63.33 seconds, terminals dirty119,
forced119/query164) preceded the copy-on-mutation extension and are historical,
not acceptance of the latest source. The earlier shared-producer related128
matrix belongs to payload43fc6b1e, not this new checkpoint.

Focused selector:
`^original_drawable_queries_producer-(walk-|first-root)`.
Build target: `original_drawable_queries_fixture` in the three original-core
GCC, GCC sanitizer and Clang sanitizer build configurations. Sanitizer tests
run on the normal host because the sandbox previously prevented LeakSanitizer
thread inspection. Leak checks and existing per-family timeouts stay enabled.
Logs: `/tmp/n2-worktree-condition-build.log`,
`/tmp/n2-worktree-condition-tests.log`, and
`/tmp/n2-worktree-condition-{gcc,clang}-sanitize-{build,tests}.log`.

## Resume

Continue this same governing transaction: add coupled mutation/rollback and
locally caught failure tests, then freeze source and run the related GCC and
both compiler sanitizer matrices. The configured full473 suite is unrun.
The default build defines DIRTY_CONDITION_FLAGS; its dormant immediate branch
is not exercised by this focused cohort. Enrollment currently covers initialized
native producer owners, not arbitrary new module construction/destruction or
future effect resources during a root scope. Concrete19 effects/movement/event
parents, camera actor-follow, actual startup and loaded simulation still need
their production admission/lifetime contracts. Slice03 and N2 remain incomplete.
