# M22 plan 01 slice 07E0F: factory-attached rigid source pixels

## Outcome and dependency

Requires accepted 07E0E production-order opt-in original display/UI/terrain bootstrap and accepted 07C2/07E0D local one-rigid physical source frames. In the opt-in original factory profile only, an owned generated rigid W3D object attaches to the production-published `RTS3DScene`, survives an original `GameClient::update`/`W3DDisplay::draw`, produces changed physical pixels relative to the empty frame, detaches, and leaves no source owner after teardown. This does not promote map-loaded terrain or the normal production factory.

## Scope, state and failure

Use the canonical original asset loader, display scene, view camera and device edge already published by 07E0E. The packet is generated from repository-owned fixture data; retail symlink content is read-only and not required. Object, scene and asset refs are explicit across attach/detach, failed load and second fresh process. No synthetic renderer command may stand in for `GameClient`/`W3DDisplay` draw. Unsupported terrain/effects remain guarded. Authorization is not applicable to this local opt-in test profile.

## Tests and acceptance

Positive empty-versus-rigid pixel oracle on the production factory's fixed 800×600 BGRA8 target, source owner/ref counts, detach absence and two fresh generations. The generated packet is loaded through the base `FileClass` overload explicitly; selecting the derived filename overload would convert the RAM file to `UNKNOWN` and fail the positive runtime control. Negative malformed/missing packet recovery leaves the edge and factory owners clean. Run focused source and host Vulkan validation, five full non-GPU suites with leak-capable GCC/Clang sanitizer controls, source/provider/ledger checks, and commit independently with evidence. This child must pass before the full 07E production-scene claim.

## Result

The opt-in production-order original factory loads the generated rigid packet through the explicit base `FileClass` overload, creates `TEST.ZERO01`, attaches it only to the factory-owned `RTS3DScene`, and frames it with the accepted authored source camera. The first canonical `GameClient`/`W3DDisplay` frame changes 74,850 physical pixels while the absence control changes zero. Reset and GameMain teardown leave all published owners null and the same residual 26 as the empty/device control after accepted 07E0F1 task-pool retirement. Missing and malformed packets fail closed with residual 26 and zero owners. GCC Debug and Clang Release pass the full fresh-process contract 30/30 each; five exact-tree non-GPU suites pass 194/194. Evidence: [07E0F](../../evidence/cnc-generals-zero-hour/milestone_22_slice_07e0f.md).
