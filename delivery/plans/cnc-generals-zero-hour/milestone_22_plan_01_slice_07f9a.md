# M22 plan 01 slice 07F9A: projected shroud resource and source material pass

## Outcome and dependency

Requires 07F9's generated map owner. Restore the smallest source-owned shroud
resource path required before the original `RTS3DScene` can render a loaded
terrain: default-filter `W3DShroud::render(CameraClass*)` projects its bounded
map state into an owned texture, and the existing source shroud material pass
installs/uninstalls that texture without activating tracks, water, shadows,
particles, smudges, map objects, or physical pixels.

## Discovery and scope

The attempted 07FA frame reached the source `RTS3DScene` shroud material pass
after the expected display ordering, but the CPU shroud had no projected
texture and the original material path faulted. This is a mandatory source
dependency, not an optional visual enhancement. Keep it separate from 07FA's
view/update/traversal work so projection/resource ownership can be tested and
reverted independently.

## Acceptance

Generated read-only maps prove successful default projection and source
material install/uninstall, absent-camera/uninitialized/filter rejection,
injected texture-allocation failure rollback and retry, reset/re-entry and zero
resource teardown. A coupled no-map visual control retains 07F9's intended
missing-stream `FALSE` result without publishing a map owner. Recording evidence must identify the original shroud owner
and must make no pixel claim. 07FA remains responsible for source ordering,
view update and the full terrain scene frame.

The coupled control also restores its stack-published display, terrain visual,
and tactical view globals during assertion unwinding. This is a stale F9-era
test expectation/exception-safety repair discovered by F9A, not a product
behavior change.

## Commit boundary

One independent commit with CPU source, generated fixture, focused tests,
ledger/evidence and governing-plan index. Retail content remains untouched.
