# M22 plan 01 slice 08S0B: native preload and temporary Drawable lifetime

Status: planned; production waits for accepted 08S0A.
Plan transaction parent: `89df8bb309ad8cd4f6d409cdc27d645f11d9f782`.

## Outcome, dependencies and boundary

Depends on 08S0A and accepted 08L2R1 symmetric binding, 08N0 modeled ownership,
08P0C4 tree factory/removal, 08Q0 recolor/cache, 08R0 prop/reset/frame and
display/bootstrap/borrowed-file/texture owners. [S0](milestone_22_plan_01_slice_08s0.md)
records the exact graph and common final-source contract. The original
GameLogic/GameClient preload stage traverses actual modules and prepared asset
descriptors without reaching a pending display stub or leaking its temporary
Drawables. Missing optional preload retains native best-effort behavior; required
scene model/texture admission remains strict at its accepted consumer.

No replacement preload list/parser, factory kind, render frame owner, pre-upload,
audio/UI/media behavior, recolor semantics, class fields/virtuals/layout or
serialization changes. Windows successful behavior and ordinary minimal/headless
LinuxDisplay recording semantics remain unchanged. No retail selector/input is
added; generated witnesses use the established full-probe/Python process/map
owner. Retail permissions are not needed for implementation.

Expected surfaces: CPU W3DDisplay preload methods, owner-local GameClient
temporary preload cleanup, existing full-probe generated dispatch only where
needed, new owned probe/wrapper and CMake/ledger/identity coverage. Stage shared
engine/ledger paths by exact B hunks, never the preserved active08 trial.

## Source order, provider and lifetime contract

Display model preload preserves literal native model+`.w3d` naming and existing
asset-manager cache check/file resolution. It uses A's scoped strong import;
missing source load=false remains optional void preload with no new fallback.
Malformed/unsupported/provider/ownership/allocation rejection stays explicit
and fully rolled back. Empty authored model states retain native caller skip.
Do not reinterpret already-qualified names or modify established templates.

Texture preload uses the exact default regular texture descriptor request and
releases its returned caller ref once, including exceptions. No lazy Init,
pixel loading, upload, LastAccessed, sampler/state/stage/frame mutation occurs.
Cache hits keep exact descriptor identity/defaults and refs. Matching display,
manager/singleton, file/provider and idle generation are admitted before work;
null/stale/foreign/removed/busy owners reject without side effects. Source loader
missing behavior is not permission to ignore a later required draw failure.

Keep GameClient's native order: loaded Drawable modules; selected preload
temporary Drawables; debris models; optional control-bar scheme image requests;
particle descriptors; authored fixed texture requests. Keep time/preloadEverything/
KINDOF_PRELOAD decisions, condition-state order and source no-transition-preload
semantics. Do not bypass normal module callbacks or call a private toy path.
Preflight idle removal/lifecycle conditions before temporary creation. After
newDrawable returns, a scoped owner destroys exactly that temporary on any
module/preload/import failure and on success; construction rollback stays with
accepted factory ownership. Preflight ensures cleanup cannot throw/mask the
primary error; no destructor-based rejection or sibling reset. Existing live
Drawable/module/scene/shadow/tree/prop/registry/ref identities remain unchanged.

Per-request import is atomic; earlier successful independent preload requests
may stay accepted. No invented all-or-nothing GameClient/map preload batch.
Debris list is cleared only at its native completed-loop point; retry is
idempotent via accepted cache identities. Imports do not advance animation/RNG/
FX/audio or create frame/present commands. Display teardown uses accepted source
order; missing provider/reset/re-entry does not dereference stale ownership.

## Generated witnesses and acceptance commands

Register `original_w3d_asset_preload` through a new owned wrapper/probe on the
existing full-probe process/map/asset-producer lifecycle. Cover fresh-map
GameLogic start continuation with preload enabled, loaded-state and temporary
preload Drawables, time/preloadEverything selection, debris/particle/optional
scheme/default descriptor order without introducing a UI draw claim. Prove
existing models/cache hits, qualified names, empty/missing optional requests,
native lazy texture identity/defaults, and no GPU/stage/access/frame consumption.

Inject every import and temporary constructor/module callback/preload/destruction
admission boundary; assert exact registry/scene/module/shadow/tree/prop/ref/native
baseline immediately and deterministic same-owner retry. Keep siblings intact.
Cover provider omission/foreign/removed/stale generation, active/pending frame
rejection before temporary creation, reset/recreation, two generations and zero
teardown. Existing active08 dirty replacement/failure/retry assertions remain
separate trial coverage, not folded into B. Module/instance kinds not already
accepted remain closed, including unsupported conditional preload templates.

Focused build:
`cmake --build build/<preset> --target zh_original_w3d_full_probe original_w3d_cpu_graph_tests original_w3d_asset_import_tests original_w3d_prop_owner_tests -j4`.
Exact fourteen-control focus:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -R '^(original_w3d_asset_preload|original_w3d_asset_import|original_w3d_asset_import_identity|original_w3d_asset_import_provider_removal|original_w3d_generated_construction|original_w3d_borrowed_file_owner|original_w3d_modeled_volume_ready|original_w3d_shadow_decal_route|original_w3d_shadow_volume_route|original_w3d_prop_owner|original_w3d_tree_module|original_w3d_full_draw_scenario|original_w3d_full_draw_identity|original_w3d_full_draw_provider_removal)$' --output-on-failure -j1 -V`.
Verify all14 IDs after B registration. Four focused configurations and strict
same14 both sanitizers follow S0, no strict UBSan override.

New generated physical mode on GCC/Clang debug and both sanitizers:
`VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation python3 tools/run_validation_clean.py python3 tests/original_rendering/test_w3d_asset_preload.py --source-root . --executable build/<preset>/zh_original_w3d_full_probe --asset-producer build/<preset>/original_w3d_cpu_graph_tests --gpu`.
This is B's implementation-owned wrapper extension, not a claimed existing run.
After exact descriptor preload, drive those same identities through accepted
source mesh/terrain draw; compare source payload/commands/pixels/recreation and
preserve required-missing rejection. Preload itself claims no pixel upload.
Run six complete builds/canonicals, established native Vulkan/minimal/LAN,
ledger/owned/staged/privacy audits. Only then commit B and close S0 separately.

## Preconditions, readiness and commit

M22-S0-01 accepted providers, M22-S0-02 accepted A are entry gates;
M22-S0-03 B wiring/new witness is its output. PRE-002 committed tools/generators;
PRE-012/PRE-016 fresh host physical/strict/LAN facilities at acceptance;
PRE-008 read-only retail is later active08 input, not a B prerequisite.
Source/index and existing ten focus registrations are inspected; four new IDs
are A/B registration outputs. No new tool/service/credential or M0.
Acceptance requires source preload continuation, every temp/import fault and
retry/teardown/control plus the complete final-source matrix clean.
Exact commit: `delivery: M22 08S0B own native asset preload lifecycle`.
Only B-owned source/test/registration/ledger/plan/evidence/index hunks; preserve
active08/renderer unstaged paths. S0 aggregate adds no executable change/rerun.
