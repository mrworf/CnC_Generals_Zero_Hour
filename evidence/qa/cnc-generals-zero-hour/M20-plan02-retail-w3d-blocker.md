# M20 plan 02 retail W3D registry blocker

Status: `UPSTREAM_REQUIRED` — delivery architecture/dependency reconciliation

The explicitly provisioned retail tree was accessed read-only through its ignored repository symlink. No private path, hash, byte, or host-specific identifier is recorded here. No physical graphics, audio, input, or network device was acquired.

## Finding

Plan 02 successfully resolves the former common-engine source-order blocker for owned fixtures, but its final retail gate exposes one remaining startup dependency that the accepted provider packet did not resolve. Retail `Object.ini` contains W3D draw-module declarations. Original `ThingTemplate::parseModuleName` asks the production `ModuleFactory` for each declaration's `ModuleData` while the object templates are being initialized, before execution or device acquisition.

The currently integrated common `ModuleFactory` does not register W3D draw modules. On the first reached `W3DDefaultDraw` declaration, `ModuleFactory::newModuleDataFromINI` returns null and original `ThingTemplate.cpp` dereferences that missing provider. This is not valid failure behavior and cannot be accepted as the retail gate.

The authoritative provider is `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/Common/Thing/W3DModuleFactory.cpp`. It extends the common factory with 19 W3D draw-module registrations. Compiling that actual registry makes its matching draw providers live. Those translation units and headers are still classified as excluded M22-M23 device consumers and retain their legacy WW3D/Direct3D compile closure; the focused GCC attempt reaches `WW3D2/dx8fvf.h` including unavailable excluded-SDK header `d3d8.h`. This happens at compile time even though the bounded headless profile would never acquire a graphics device.

Therefore the current graph simultaneously requires all of the following, which cannot be satisfied by the accepted packet:

1. M20 retail initialization must use the actual W3D module registry and data parse providers.
2. M22-M23 are later domain acceptance and cannot be prerequisites for M20.
3. M20 may replace only physical device edges and may not use a reduced registry, generic parser, proxy success, or alternate lifecycle.
4. Slice 06 acceptance forbids using a later milestone as a code prerequisite.

## Reproducible source evidence

- `GameEngine.cpp` constructs `TheModuleFactory` before `TheThingFactory`, whose retail object load reaches draw-module declarations.
- `ThingTemplate.cpp:601-603` calls `TheModuleFactory->newModuleDataFromINI(...)` and immediately consumes the returned original `ModuleData`.
- `W3DModuleFactory.cpp:62-80` contains the 19 required W3D registrations.
- `data/original-source-classification.tsv` assigns `W3DModuleFactory` and the draw providers to excluded M22-M23 device work.
- A bounded retail run progressed through archive/config initialization to the first `W3DDefaultDraw` module, then failed at the missing W3D `ModuleData` provider.
- A focused attempt to compile the authoritative W3D registry reached the excluded Direct3D header from the original W3D provider closure. Those experimental W3D build edits were removed; no proxy registry is retained.

## Completed local finding retained for the next plan

Before reaching this blocker, the retail gate proved the former Linux archive stub was insufficient. The working tree contains a real bounded BIGF/BIG4 adapter backed by original `ArchiveFile`, `RAMFile`, `StreamingArchiveFile`, and `ArchiveFileSystem` directory dispatch. It compiles and preserves the owned production fixture; it also advances read-only retail initialization into actual object-template parsing. It is useful M20 work, but it does not establish slice or milestone acceptance by itself.

## Required upstream decision

Reconcile the M20/M22-M23 boundary using one of these implementation-ready, identity-preserving directions:

1. Authorize and specify a source refactor that separates the original W3D `ModuleData` parse providers and complete 19-name registry from physical draw/device implementations. M20 would compile the original parse providers and an explicit fail-closed instance-creation edge; M22-M23 would later supply the original runtime draw providers. Identity checks must distinguish that boundary from a generic parser or success/no-op registry.
2. Move the complete W3D provider compilation/port ahead of retail lifecycle acceptance and make M20 depend on it, reordering or combining the affected milestones.
3. Narrow M20's retail gate to the last source-owned stage before object-template W3D module parsing, and move full retail initialization acceptance to the integrated milestone after M22-M23.

Do not resume slice 06 by registering placeholder W3D modules, discarding their parse callbacks, weakening the retail gate, or accepting the base `ModuleFactory` crash.
