# M8 delivery plan: UI, fonts, and input integration

## Governing contract

Deliver [M8 UI, fonts, and input integration](../../milestones/cnc-generals-zero-hour/M8-ui-fonts.md) from the authoritative Linux port plan. The transaction starts at `70270a78e323bcb02e3065775deba2a85cf7bbc4`. M8 owns deterministic UI command generation and selected-English text behavior; M14 owns pixels and GPU acceptance.

## Scope and invariants

The implementation records clipped 2D/UI quads, blend and half-pixel state, cursor layering, menu/loading transitions, text runs, and focus/resize reactions through the M7 recording renderer. It loads archive fonts from retained memory, resolves system fallbacks through Fontconfig only when no local font was selected, and closes the English-locale shaping gate with executable evidence. SDL physical controls and UTF-8 composition feed an engine-neutral UI control model; text never comes from key presses.

Internet-only actions are unavailable locally and complete immediately without any network attempt or waiting state. Retail content remains read-only and is not required by ordinary tests. Authorization is not applicable to these in-process UI facilities; all external text, font bytes, geometry, and events are bounded and validated before state mutation.

## Dependency-ordered slices

1. [Slice 01: recorded 2D/UI scene generation](milestone_08_plan_01_slice_01.md)
2. [Slice 02: retained font faces and English locale layout](milestone_08_plan_01_slice_02.md)
3. [Slice 03: SDL-driven UI controls and representative flows](milestone_08_plan_01_slice_03.md)

All slice plans were written and inspected before production edits. Each slice is separately testable and revertible. Final validation builds all four supported presets and runs `ctest --preset <preset> -L 'platform|renderer-contract|ui' --output-on-failure`; UI shader compilation is part of every build.

## Acceptance

Automated streams cover menu, loading, tooltip, subtitle/caption, save/player/game names, chat editing and composition, wrapping, truncation, mixed ASCII/non-ASCII, missing glyph fallback, cursor layers, focus, and resize. Negative tests prove invalid clips, geometry, font inputs, composition ranges, and unavailable Internet actions cannot corrupt state or start external work.

## Delivery record

Commits are recorded after each completed slice.

- Slice 01: `672731d` (`delivery: M8 slice 01 record UI scenes`)
- Slice 02: pending (`delivery: M8 slice 02 add font layout`)
- Slice 03: pending (`delivery: M8 slice 03 integrate UI input flows`)
