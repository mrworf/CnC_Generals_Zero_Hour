# M22 slice 08E1A: base load-screen presentation service order

Close only the common source `LoadScreen::update(Int)` edge using generated
headless WindowManager and Display owners.  It proves the exact order
service-Windows → window update → display update → display draw, with no
mode-owned layout admission.  Absent, foreign, stale and duplicate owners,
plus update/draw failures, roll back cleanly across two generations with zero
providers/resources.  No retail content, pixels, raw graphics API, map load,
or mode-specific UI behavior is in scope.

08E1A depends on 08D and is required by 08E1B/08E1C.  Generated proof,
proportional acceptance, evidence/index/ledger and an independent commit are
required.

## Control boundary

The base edge owns no registry, resource handle, or mode object: its only
inputs are the singleton published Engine, WindowManager and Display
providers.  Generated two-generation headless ownership, existing provider
removal, window-list cleanup, and raw-allocation assertions therefore cover
re-entry, removal and zero ownership directly.  A foreign/stale/duplicate
`LoadScreen` cannot be represented at this edge because it is neither selected
nor stored here; those are mode-owner controls for 08E1B/08E1C.  E1A instead
proves fail-closed absent-provider rejection and does not invent non-source
handles or failure switches.  Window/display update and draw failures belong
to their already-owned providers; E1A preserves propagation/cleanup rather
than converting them into synthetic base-edge behavior.
