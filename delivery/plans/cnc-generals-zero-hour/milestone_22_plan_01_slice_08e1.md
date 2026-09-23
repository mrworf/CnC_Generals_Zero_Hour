# M22 slice 08E1: mode-owned loading UI/display presentation lifecycle

## Goal

Admit the minimal generated source lifecycle behind the first 08E consumer:
the existing mode-owned `SinglePlayerLoadScreen` and `MultiPlayerLoadScreen`
owners, their WindowManager service, and `Display::update`/`draw` ordering.
This is a state/order proof only and does not claim retail layouts, assets,
or pixels.

## Bounded contract

- Generated source fixtures provide exact compatible window/display owners and
  exercise each selected mode owner through init, one bounded progress update,
  reset, destruction, retry and a second generation.
- The source ordering is service-Windows → WindowManager update → Display
  update → Display draw.  Absence is the existing no-op only when the factory
  returns null; foreign, stale, duplicate, unsupported and partially-created
  owners fail closed.
- Fixture creation/update failures roll back without retained window, display,
  provider, or Recording ownership.  No retail input, raw graphics API,
  layout bytes, map loading, resize, or pixel output is admitted.

## Dependencies and acceptance

08E1 depends on 08E1B and 08E1C.  It is a prerequisite of 08E, which alone owns
GameLogic dispatch and redacted retail advancement.  Generated detailed proof,
proportional six-config/host checks, ledger/evidence/index update and an
independent commit are required.
