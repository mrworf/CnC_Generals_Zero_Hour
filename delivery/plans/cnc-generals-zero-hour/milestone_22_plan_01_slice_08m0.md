# M22 plan 01 slice 08M0: empty terrain bib cleanup prerequisite

## Goal and observable outcome

The original client/UI may clear terrain bibs during map construction even
when no bib was ever produced. Under the exact CPU-only, map-loaded terrain
owner state, `W3DTerrainVisual::removeAllBibs` treats that empty-list cleanup
as an idempotent no-op. It neither claims active bib storage nor changes any
terrain, graphical or gameplay owner. This closes the fixed post-08L abort
category before slice 08 retail scene admission resumes.

## Boundary and dependency

Depends on accepted 08L and precedes slice 08. Verify the source call chain
from unconditional `InGameUI::destroyPlacementIcons` cleanup to the terrain
visual, and expose only the minimum read-only bib-owner state needed to prove
the list is absent. Admit the no-op only for the accepted CPU-only visual,
height-map and loaded-map identities with no bib buffer. Reject missing,
foreign or active owners; do not use a selector-only or unconditional
`removeAllBibs` success and do not reset the world as a shortcut.

`addFactionBib`, drawable bibs, per-ID removal, highlighting, bib geometry,
textures, GPU buffers and raw Direct3D rendering remain fail-closed. Source
placement/AI feedback can reach those active producers later; if a corrected
redacted continuation does, stop and plan 08M1 as a separate active-bib
storage/render/removal owner before continuing slice 08. Do not infer that
retail maps have no bib producers merely because this first call was empty.

## Tests and validation

Generated direct-source positive: map-loaded terrain with absent bib buffer,
UI cleanup and repeated clear leave exact owner identities, counts and
Recording operations unchanged through two generations and reset/re-entry.
Negative: missing or foreign terrain visual, mismatched height-map owner,
unloaded map and any present bib buffer reject before mutation. Active
`addFactionBib`/drawable producer attempts continue to reject; existing
generated construction, terrain and scene controls remain green. Run six
complete GCC/Clang Debug, Release and ASan+UBSan builds and canonical
nonretail suites (`-LE gpu|lan|retail`, sanitizer leak detection disabled),
focused strict host LSan, physical Vulkan, serial LAN, original dependency
ledger and diff checks on final source. The retail symlink/content remain
read-only and no private identifier or raw output enters evidence.

## Acceptance and commit boundary

One independently validated source-owner commit:
`delivery: M22 08M0 clear empty terrain bibs safely`.
Only after acceptance rerun the corrected single-separator redacted retail
continuation for slice 08, recording fixed aggregate/stage categories.
