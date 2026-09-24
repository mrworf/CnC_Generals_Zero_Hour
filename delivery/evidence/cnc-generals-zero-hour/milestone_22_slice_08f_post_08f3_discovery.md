# M22 08F post-08F3 discovery: display shroud-refresh prerequisite

Parent commit: `559c0e16601d668cb8f27c980ea28f16e13a073e` (accepted 08F3).
This is a generated-only diagnostic and plan checkpoint, not 08F or 08F4
implementation acceptance. No retail provider/content was opened and no
private retail identifiers, paths, bytes, hashes, images, or raw process
output are recorded. All exploratory production/test edits were removed
before commit; accepted 08F0/default behavior remains unchanged. The
unrelated renderer diagnostic remains unstaged.

The generated mission trial used a read-only project-owned logical map
extended with a generated flat visual blend tile, generated terrain texture,
and positive partition cell size. A separate explicit construction selector
passed the accepted 08F0 parser stop, and source-ordered terrain attachment,
water reattachment, `enableWaterGrid(FALSE)`, and accepted 08F3
`updateMapOverrides` reached fixed terrain-loaded and radar-complete stage
categories. These changes were diagnostic only and are not 08F acceptance.

The first process abort after radar was a second exception from map-loaded
`removeAllBibs` during teardown, masking the original failure. An exact
generated-selector, map-owned empty-bib guard was tested locally; no bib
producer exists in this fixture and all creation APIs remain fail-closed.
With that guard, the diagnostic returned through normal rollback with zero
published graphics owners and zero Recording resources. The accepted
source process-pool allocation delta persisted and is not an ownership
claim. The empty-bib cleanup remains selector-local 08F work, not a new
bib implementation or proof of populated retail bib behavior.

The unmasked next failure is `W3DDisplay`'s typed-pending physical method
during source `GameLogic::startNewGame` → `PartitionManager::init` →
`refreshShroudForLocalPlayer`. Native refresh first calls display
`clearShroud`, then radar clear, then per-cell display/radar setters.
Native display clear is a no-op; native `setShroudLevel` maps the logical
category to configured alpha, writes the original terrain `W3DShroud`,
and calls terrain `notifyShroudChanged`. The CPU shroud grid and setter
exist, but CPU display clear/setter and CPU terrain notification are absent.
Native terrain notification touches a prop buffer only if present; the
bounded CPU map has no prop producer. The ordered [08F4 owner plan](../../plans/cnc-generals-zero-hour/milestone_22_plan_01_slice_08f4.md)
therefore separates terrain notification (08F4A) from display dispatch
(08F4B). Neither child admits 08F construction. After both are accepted,
08F resumes water reattachment/order and probes the next source stage.

Read-only checkpoint: source call order and native/CPU branches inspected;
generated GCC Debug diagnostic reached the fixed terrain and radar stages,
then localized the display method after restoring safe empty-bib cleanup;
all trial code/test wiring removed; `git diff --check` clean. No broad
acceptance was run because the checkpoint changes no production/test code.
