# M22 08F0 prerequisite discovery

The accepted F0A commit is `122832bc96ed2ed96af91313ab610b1c0a487272`.
Only generated project-owned mission input was executed during this
investigation.  No retail provider or content was traversed.

A conjunctive trial selector crossed 08D's original active-water reset and
observed its fixed completion marker.  The subsequent update found the active
water owner detached as 08D specifies.  A trial that retained the live display
scene, guarded this exact detached owner, and suppressed the pre-map frame
reached `GameLogic::loadMapINI`.  It then failed at the Linux
`W3DDisplay::doSmartAssetPurgeAndPreload` pending method.  The process returned
the existing typed failure status with zero published graphics owners and
Recording resources.  The original Windows method reads an optional usage
list and calls `Free_Assets_With_Exclusion_List` even when the file is absent.

Skipping that method would omit a real source ownership transition.  Trial
code, diagnostic logging and the incomplete selector test were removed.
08F0B now owns the display handoff; 08F0 resumes after that independently
validated prerequisite.  The unrelated renderer diagnostic remains unstaged.
