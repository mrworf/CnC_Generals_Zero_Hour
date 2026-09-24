# M22 slice 08E1: mode-owned loading UI/display presentation lifecycle

## Goal

Close the missing aggregate boundary ahead of 08E: prove that the accepted
generated `SinglePlayerLoadScreen` and `MultiPlayerLoadScreen` owners execute
in source order as mutually exclusive UI modes over the same presentation
spine. This is a state/order proof only and does not claim retail layouts,
assets, audio, network transport, or pixels.

## Bounded contract

- Reuse the accepted 08E1A source service order, 08E1B actual single-player
  owner, and 08E1C actual multiplayer owner. Add no production owner,
  rendering path, or direct text shortcut.
- The shared `WindowManager`/DisplayString and mapped-image publications must
  be clean after both single-player generations before either multiplayer
  generation starts. The source order is therefore single-player lifecycle →
  zero shared UI/provider ownership → multiplayer lifecycle.
- The generated C++ owner target emits an aggregate terminal marker only after
  both two-generation lifecycles and final zero-ownership cleanup. A dedicated
  Python CTest rejects a missing owner, missing terminal marker, nonzero exit,
  or reordered markers. The C++ transition assertion rejects leaked/stale
  windows, display strings, campaign data, GameInfo/settings/map/template
  providers, or a mismatched mapped-image publication.
- No retail input, raw graphics API, layout bytes beyond existing generated
  fixtures, map loading, resize, audio output, or pixel output is admitted.

## Dependencies and acceptance

08E1 depends on 08E1A, 08E1B, and 08E1C. It is a prerequisite of 08E, which
alone owns GameLogic dispatch and redacted retail advancement. Generated
detailed proof, proportional six-config/host checks, ledger/evidence/index
update and an independent commit are required.

## Validation and acceptance

Run the new aggregate plus the accepted source-owner, provider-removal,
identity, and ledger checks in GCC and Clang. Then run the fresh six configured
non-GPU/non-LAN suites with exact registration reconciliation, canonical
sanitizer wrappers, strict host `detect_leaks=1` focus in both compilers,
proportional physical Vulkan validation, and serial local-socket LAN in all
six configurations. The aggregate must preserve a zero physical device/pixel
claim and leave all mode-owned state unpublised after generation two.

## Commit boundary

One independently reversible test-only aggregate commit: source transition
assertion, aggregate witness/CTest registration, governing row, source ledger,
and generated-only evidence. The user-owned renderer diagnostic remains
unstaged.
