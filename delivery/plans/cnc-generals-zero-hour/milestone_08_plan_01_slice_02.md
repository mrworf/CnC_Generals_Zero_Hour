# M8 slice 02: retained font faces and English locale layout

## Goal and observable outcome

Selected-English UI text loads a VFS font from retained bytes or a Fontconfig-resolved system face, emits bounded glyph metrics/runs, wraps or truncates predictably, and uses deliberate fallback glyphs without atlas overrun.

## Scope and non-scope

Implement FreeType memory-face ownership, Fontconfig fallback lookup, glyph metrics and grayscale atlas placement, UTF-8 layout, wrapping, ellipsis truncation, missing-glyph replacement, and a tested English locale decision. Contextual shaping, bidirectional layout, universal locale coverage, screenshots, and pixels are excluded because the supplied locale is English and evidence does not require HarfBuzz/FriBidi.

## Dependencies and ordering

Requires slice 01 UI scene commands, M4 VFS byte access, and the existing FreeType/Fontconfig dependencies. It precedes control/flow coverage so text behavior is reusable there.

## Entry point and end-to-end behavior

`FontFace::from_memory` retains a shared byte owner for the entire `FT_Face` lifetime. `FontFace::from_system` asks Fontconfig for a family/path only when `LocalFontFile` is absent. `TextLayout::layout` decodes UTF-8, looks up glyphs and metrics, packs a bounded grayscale atlas, and returns positioned glyphs/lines with wrap or ellipsis policies. Missing code points resolve through the selected fallback face or U+FFFD and are logged once.

## Data/state transitions

Face state owns `library -> retained bytes -> face`; destruction reverses that order safely. Atlas shelves advance only after a glyph fits. Layout is side-effect free except bounded glyph caching and diagnostic accumulation. Failed load/layout leaves no partially usable face or atlas entry.

Authorization is not applicable. VFS bytes are read-only; system font lookup is local and no font is installed or registered.

## Validation, errors, and recovery

Positive tests use an installed font loaded both from copied test memory and Fontconfig, then cover metrics, baseline/bearing/advance, ASCII plus non-ASCII, wrap, ellipsis, and fallback. Negative tests cover empty/corrupt font bytes, missing family, malformed UTF-8, impossible atlas size, and invalid width. The locale-decision test asserts English is left-to-right simple codepoint layout and therefore HarfBuzz/FriBidi stay absent.

## Expected surfaces

`include/zh/ui/font.h`, `src/ui/font.cpp`, `tests/ui/test_ui_font.cpp`, `docs/porting/english-locale-decision.md`, `CMakeLists.txt`, and governing plan delivery record.

## Validation commands

Build and run `ui_font_tests`, then run the `ui` label in `linux-gcc-debug`. Milestone-wide four-preset validation follows slice 03.

## Acceptance and commit boundary

Memory ownership, fallback, metrics, wrapping/truncation, malformed input, and bounded-atlas behavior pass without retail data. Commit as `delivery: M8 slice 02 add font layout`.
