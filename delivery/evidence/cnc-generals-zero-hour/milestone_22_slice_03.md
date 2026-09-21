# M22 slice 03 — reached original GameClient draw behavior

Outcome: the canonical original `W3DModuleFactory` in the mutually exclusive
full probe constructs all ten concrete reached original W3D draw classes.
The original GameClient scenario, scene/asset owners, render objects, animation,
physics, containment, and draw overrides decide behavior; the Linux test
observer records outcomes but owns neither geometry nor render decisions.

An owned W3D chunk fixture generates HLOD, hierarchy, two meshes including
linear-offset tread material, four tire pivots, public supply bone, and a
two-frame animation. With four original scenario objects, the source produces
ten loaded HLODs, one animation, two supply visibility transitions, two tank
tread UV scrolls, three distinct wheel captures, and two dependency
block/release pairs. The client-before-logic pristine-bone query returns zero;
the positive one-bone query occurs inside the actual original
`GameLogic::update` latch. The original `OverlordContain` creates a portable
rider; each of three source Overlord overrides independently releases the
rider's blocked dependency and invokes its original draw path. Removing rider
capacity fails closed before scenario evidence; an optional missing model
produces nine, not ten, HLODs and no animation. Original
`W3DAssetManager`/`W3DFileSystem` load `ABBarracks_AC` from the read-only retail
`W3DZH.big` for a separate real-asset witness.

The test-only environment profile and logic-phase observer are compiled only
for the full probe. Production `zh_original_main` still compiles and links
its M20/M21 schema configuration; slice 04 must supply physical device
translation before switching production and claiming retail scene rendering.
The full-probe link-map/compile-command check rejects mixed schema/full draw
definitions, loss of any ten concrete providers, or absence of the original
WW3D scene/material providers; it includes explicit provider-removal controls.
No GPU output, recording, or visual acceptance is claimed here.

Validation: full GCC Debug build and CTest 138/138 passed; full Clang Debug
build and CTest 138/138 passed, with host UDP access required for two existing
LAN cases. Focused original full-draw scenario, identity, provider-removal
tests pass 3/3 on both toolchains, including read-only retail model load.
Original source-dependency ledger and `git diff --check` pass. No content
within the original-game symlink was modified.
