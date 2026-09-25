# M22 plan 01 slice 08L3A0: borrowed W3D file-factory owner

## Goal and observable outcome

A generated modeled Object constructed during the original W3D display scene
route resolves its model through the source `W3DFileSystem` and
`WW3DAssetManager` graph. The borrowed asset/scene lifetime has a valid file
factory for the whole construction window. Failure and reset unwind only the
factory owned by that window, and retry can load the model again.

## Scope, dependencies and exclusions

Depends on 08L2 atomic Object/Drawable construction and precedes 08L3A bridge
admission. Close the `OriginalDrawOwners` borrowed-versus-owned lifecycle in
`src/original_runtime/linux_game_engine.cpp` and its exact W3D file-factory
publication boundary. `W3DFileSystem` sets `_TheFileFactory` on construction
and clears it on destruction; `W3DDisplay::init` normally creates
`TheW3DFileSystem`. The generated `OriginalDrawOwners` borrow path currently
checks assets/scene but permits `_TheFileFactory == NULL`, causing modeled
construction to dereference it in `WW3DAssetManager::Load_3D_Assets`.

Preserve owner identity and source order. If the display already owns a valid
`TheW3DFileSystem`, borrow it and do not delete it. If a bounded generated
window must create one, publish it only for that window and restore prior
factory/global pointers exactly on success, exception, reset and teardown.
Do not create a competing asset manager, substitute a model, bypass the source
file provider, admit retail scenes, alter physical Vulkan, or change general
file-system semantics.

## Investigation before production edits

- Establish why the display's `TheW3DFileSystem` and `_TheFileFactory` differ
  during generated scene construction, and identify all owner transitions
  through display init, reset, `OriginalDrawOwners` and display destruction.
- Check whether a pre-existing foreign factory can be borrowed safely or must
  cause fail-closed rejection; preserve that prior owner on every exit.
- Use a project-owned W3D model and generated source route to witness an
  actual `W3DModelDraw` load, not only pointer presence.

Investigation result: the CPU-only `W3DDisplay::init` branch publishes scenes
and assets without constructing `W3DFileSystem`; the native branch does create
it. The earlier standalone draw window then destroys its own factory and
leaves `_TheFileFactory` null. The apparent non-null `TheW3DFileSystem` value
in a debugger was a non-linked debug-symbol artifact: the pre-change probe
binary had no such linked symbol. The corrected CPU branch owns and publishes
that source provider before WW3D initialization, and the generated window
borrows its exact identity. A standalone window restores its prior factory.

## Tests and validation

Positive: modeled generated construction loads the owned W3D model through
the original asset manager, preserves display file-factory identity when
borrowed, releases a window-owned factory if needed, and re-enters over two
generations. Negative: missing model/file provider, mismatched foreign
factory, injected failure before and after factory publication, and reset
failure leave no dangling `_TheFileFactory` or `TheW3DFileSystem`, no asset or
Recording resource owner, and permit clean retry. Witness exact immediate
pre-teardown owner state without retail content or raw paths.

Run focused modeled source/provider-removal and accepted generated
construction routes, all dependency ledgers, then the governing M22 six-build
and six-suite gates, strict host LSan, physical Vulkan controls, serial LAN
and diff checks on the final tree.

## Acceptance and commit boundary

One independently reviewable source-owner lifecycle commit:
`delivery: M22 08L3A0 close borrowed W3D file factory`.
