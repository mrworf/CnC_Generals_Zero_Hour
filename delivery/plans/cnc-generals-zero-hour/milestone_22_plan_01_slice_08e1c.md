# M22 slice 08E1C: bounded multiplayer load-screen owner closure

## Goal

Exercise the original `MultiPlayerLoadScreen` constructor, `init(GameInfo *)`,
logic-fallback `update(Int)`, `processProgress`, `reset`, and destructor as one
generated, project-owned transaction.  The result is a live original WND tree
whose slot/name/side/progress/preview state is driven by the accepted C0
providers, not a replacement screen implementation.

## Scope and dependencies

The source owner is `GameClient/GUI/LoadScreen.cpp`; its WND parser,
`GameWindowManager`, display strings, mouse, audio no-device boundary, and
`GameLogic` fallback are already exercised by accepted 08E1A's headless
lifecycle target.  Accepted 08E1C0 supplies the source `GameInfo`/`GameSlot`,
INI-backed `PlayerTemplateStore`/`MultiplayerSettings`, map metadata/preview,
and start-marker contracts.

The fixture adds `Menus/MultiplayerLoadScreen.wnd` to that target's isolated
file root.  It parses a generated FactionAmerica template, one colour, map
metadata, occupied human and AI slots, and a generated preview descriptor.  No
retail WND, image, audio, map, pixel, network transport, or gameplay load is
read or claimed.  The empty generated load-screen music intentionally leaves
audio fidelity outside this slice.

## Observable transaction

For two independent generations, the original owner must:

1. construct the generated WND through `winCreateFromScript`, select the
   local FactionAmerica template/image fallback, map occupied source slots to
   live progress/name/side/team windows, and preserve the valid map preview
   and marker path;
2. take the original offline-null `GameLogic::processProgress` update branch,
   clear the source mouse tooltip, route mapped remote progress through
   `processProgress`, and retain an unoccupied third slot as inactive;
3. reject a missing generated layout before the owner transaction, then
   successfully re-enter with a fresh source owner;
4. `reset`, destroy queued WND resources, unpublish every C0/global owner,
   and leave zero windows, display strings, generated preview descriptors, or
   stale singleton pointers before the next generation.

The owner never enables a physical audio device or network transport.  It
uses the source null-network branch only; online transport remains explicitly
out of scope.

## Validation

Add the dedicated `original_multiplayer_loadscreen` CTest entry and source
removal/identity checks for `MultiPlayerLoadScreen::init(GameInfo*)`.  Run its
GCC and Clang focused lifecycle/provider tests, ledger/identity checks, the
fresh six configured non-GPU/non-LAN suites, canonical sanitizer wrappers,
strict host LSan for both sanitizers, proportional host Vulkan, and serial LAN
in all six configurations.  Record generated-only evidence and make one
isolated commit.  Depends on 08E1A and 08E1C0; required by 08E1.
