# M22 slice 07E0F evidence: factory-attached rigid source pixels

## Source and ownership result

- The explicit opt-in factory profile remains required; default production startup is unchanged.
- A repository-generated 408-byte rigid W3D packet is read from the test temporary directory, wrapped in `RAMFileClass`, and passed through the explicitly selected `WW3DAssetManager::Load_3D_Assets(FileClass&)` base overload. If the hidden derived filename overload were selected, the RAM file would convert to `UNKNOWN` and the positive runtime test would fail before attachment.
- `TEST.ZERO01` is created by the canonical asset manager, attached only to `W3DDisplay::m_3DScene`, and its local ref is released. The factory-owned `W3DView` camera uses the same authored framing as the accepted rigid source fixture.
- The first real `GameClient::update` delegates through `W3DDisplay::draw` and `W3DView` into the original scene. Reset detaches scene ownership; GameMain destroys the display/assets before the edge and device. All original owner aliases are null at the measured boundary.
- The generated input and complete source fixture are temporary and read-only. No retail symlink path is opened or embedded in output/evidence.

## Pixel and failure controls

- Empty factory control: zero changed RGB pixels on the fixed production 800×600 BGRA8 target.
- Two fresh rigid generations: 74,850 changed RGB pixels each, with no Khronos validation diagnostic.
- Empty and rigid runs both finish at residual 26 versus process baseline 22, preserving the separately accepted four device/driver process-scoped allocations and adding no source allocation.
- Missing and malformed packet controls exit through startup rollback with residual 26, `owners=0`, and no final frame claim.
- The existing 07E0E bootstrap test remains green after extending its marker with `changed=0`.

## Acceptance gates

- Focused factory bootstrap + rigid tests: GCC Debug 2/2; Clang Release 2/2.
- Repeated complete rigid test (absence, two fresh present generations, missing and malformed rollback): GCC Debug 30/30; Clang Release 30/30.
- Freshly rebuilt exact-tree non-GPU suites: GCC Debug 194/194, GCC Release 194/194, Clang Release 194/194, GCC ASan/UBSan 194/194, Clang ASan/UBSan 194/194.
- Dependency-ledger integrity checks: 3/3.

The unrelated renderer diagnostic was isolated throughout acceptance and restored after the slice commit. This slice does not claim map terrain, shroud, tracks, shadows, particles, production-default factory selection, or retail scene coverage.
