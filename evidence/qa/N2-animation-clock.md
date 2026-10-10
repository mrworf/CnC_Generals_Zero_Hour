# N2 source single-motion timing through current Drawable pose

Parent `abe51f117456fd33cab85dc0555c1a8a515c5e72`; existing N2 plan01/slice03.
Supporting timed-consumer checkpoint, not concrete runtime19, whole startup or
N2 acceptance. No adopted dependency, retail assets or recovery content changed.

Game-owned NativeModelAnimationClock follows original animobj.cpp:472,949,1048,
1089,1123 and rendobj.h:333 (paths under Libraries/Source/WWVegas/WW3D2).
Six explicit uint32 mode representations retain source values. Staged value state
preserves unsigned32 elapsed time, direction, rate multiplier, one-step endpoint
wrapping/reflection, duplicate-time previous-frame retention and retained-frame
completion. Defined malformed mode, nonfinite input/arithmetic and zero-frame
rejections avoid enum/integer conversion UB. Single-frame clips, clock wrap,
zero/accelerated multipliers and oversized forward/backward timing are exercised.

Protected current-model timed admission owns a complete model/clip/pose/clock
candidate. Advancement publishes world, pose, frame/direction and sync together;
late pose rejection cannot consume elapsed time. Actual registered ModuleFactory,
ThingTemplate, Drawable and ObjectDrawInterface tests exercise source30fps forward
and backward advancement, corrected same-time retry and zero-allocation reads.
Manual publication clears automatic advancement but retains its frame for later
source replacement previous-frame semantics. Parent/thread/logic-phase rules
remain those of N2-current-model-queries.md.

The source oversized forward pingpong branch can produce a negative frame. Clock
math retains it; native decoded-pose admission rejects unsafe sampling and retains
accepted clock/pose for retry. This is an explicit malformed sampling boundary,
not a source bug repair, modulo substitution or oversized-pose parity claim.

Cold and populated candidate failure/retry sweeps now admit timed current state,
checking the accepted clock on successful retries. Every prefix and retry remains
in each independent family, with three complete owner lifetimes. Standard and
pooled constructor exhaustion are separately retained. Timed current replacement
terminals cold173/seeded176; zero standard demand for advancement/accepted reads.
Retained projectile cold245/seeded0, wide cold247/seeded2, constructor standard1
and module-pool2. All191 allocation-manifest lines match across all configurations.

## Validation

Initial focused14 PASS32.25s; final manual-previous-frame focused2 PASS4.41s.
Frozen canonical714 source SHA256
`ea62022022b7af5c80307a6a249653a504ce04ca7c08dab3416e8e613f6d0216`;
toolchain-inclusive721 SHA256
`dd62a69b1f88618e71ba48affedb466f368b5f3bdb42bdb075ab833d47f98da7`.
Hash procedure relative to9e5c0e4f matches N2-current-model-queries.md.
Related100 PASS GCC40.37s/GCCsan173.46s/Clangsan106.74s; all processes exited0,
no live jobs remain. Configured456 full suite unrun; not N2 acceptance.
Logs `/tmp/n2-animation-clock-<configuration>-frozen-{build,related}.log`;
same target/CTest selection as N2-current-model-queries.md, plus two animation
families. All ASan/UBSan/LSan checks and60s family timeouts unchanged.

Pending: client sync-time owner/frame publication, source weighted animation
selection/client RNG/flags/transitions, actual pause/completion/event/sound callbacks;
all-LOD bounds/OBBOX and concrete19 runtime providers; condition transfer,
remaining materials/prototypes, full root startup and loaded deterministic
simulation. No render/GPU/retail parity claim and no fake successful draw output.
