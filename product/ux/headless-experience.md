# Headless qualification experience

Scope: developer/runner/asset-owner/maintainer flows HJ-01–05. This is not a new game
UI design system and does not replace original menus, layout, art, font/locale or input
behavior. Original R05/R09 preservation direction is already approved in the port plan;
D-BASELINE-001 records that authority. H1 is nonvisual. No new substantial visual surface
or consequential navigation/design alternative is introduced, so new visual concepts and
a new Product Owner design approval are not applicable to this bounded synthesis.

## Constraints and open design space

Required: native explicit-root entry, finite execution, truthful stage/result, source
fidelity, immutable retail inputs and actionable redacted errors. Open low-consequence
choices: CLI spelling, help layout and local diagnostic formatting, within those rules.
No new game launcher, GUI debugger, onboarding wizard or end-user headless game mode
is specified. The qualification runner is a development/testing entry.

## Information architecture and interaction

UX-HQ-01: Document entry options and prerequisites together: generated H1 versus private
startup probe, explicit data roots, separate writable storage, deterministic seed and
bounded ticks. Avoid displaying private roots in shared evidence. Help must be usable
without launching a world or opening retail data. Exact syntax is implementation-owned.

UX-HQ-02: Distinguish initialization, load, running, teardown and completed/incomplete/failed.
Report a public stage and actionable category for missing input, invalid data, unavailable
required provider, storage failure or execution failure. Never label an indexed archive,
linked binary, silent audio device or unrendered scene as successful full-game operation.
Failures return non-success to automation; success is scoped to the selected probe.

UX-HQ-03: Correcting an input permits the contract's retry. A failed world is retired before
fresh startup where recovery is not promised. Controlled interruption is a failed/incomplete
attempt with cleanup; the tool must not silently continue a partially executed tick. Do
not add destructive recovery/reset suggestions or write into retail roots to fix an error.

UX-HQ-04: Keep public evidence textual, readable without color, and accessible to ordinary
terminal reading/selection and automation. No information may rely on transient animation,
sound or pointer interaction. Status labels and help remain understandable when wrapped;
private source identifiers remain omitted. No new graphical responsive layout, touch UI,
focus trap or screen-reader game conformance claim is introduced.

UX-HQ-05: Keep full-game acceptance distinct. Successful H1 explicitly means headless
foundation qualification only. The retail initialization probe does not mean a campaign,
rendered scene, audible playback, replay or LAN match succeeded. Original player-facing
loading/error/return-to-menu requirements remain unchanged under R05/R09/R10.

## Observable conformance checklist

- UX-HQ-C01 → UX-HQ-01: a developer can identify required roots/storage/probe and bounded
  execution from documented help without launching the original runtime.
- UX-HQ-C02 → UX-HQ-02: missing data/provider and a failed tick produce distinct actionable
  categories and non-success, with no private root/name leakage or false parity claim.
- UX-HQ-C03 → UX-HQ-03: corrected input can retry; controlled interruption exits with cleanup
  and an incomplete result rather than being counted as a completed lifetime.
- UX-HQ-C04 → UX-HQ-04: required status remains clear in plain text with color disabled and
  no audio/display device. This is developer-interface intent, not a platform release gate.
- UX-HQ-C05 → UX-HQ-05: both success reports name their narrow scope and cannot be mistaken
  for whole-game rendering/playback or match acceptance.

No exceptions, unresolved UX choices or new explicit Product Owner preferences/dislikes.
No runtime conformance, screenshot review or full-game accessibility certification performed.
