# M22 plan 01 slice 08R0B: exact source shroud material pass

Status: planned; 08R0A accepted, ready for implementation.
Plan transaction parent: `b4fc25b137e94accc1abebd4f0f14bc992c64670`.

## Outcome and boundary

Depends on [08R0A](milestone_22_plan_01_slice_08r0a.md), accepted 05B2B2B1,
08P0C2A0/A (reflection/program ABI), 08P0C2B0 (source-stage transaction),
08P0C2B (exact shroud pixels/content/binding), and 08P0C2D0 (device transactions).
Own exact ST_SHROUD_TEXTURE install/uninstall and public CPU/GPU lowering only.
No prop lifecycle, scene scheduling, frame/present, factory, retail admission or
unrelated shader/shadow mode. [R0](milestone_22_plan_01_slice_08r0.md) supplies the
complete audit and common final-source validation contract.

## Source semantics and transaction

Native Install binds resident current shroud on the selected stage and selects
ST_SHROUD_TEXTURE, source prelit material, EQUAL depth, CameraSpacePosition and
COUNT2 texture transformation. Ordinary build uses multiplicative sprite blend;
DEBUG/INTERNAL fogOn chooses alpha sprite exactly, not guessed release behavior.
Use source-selected exact stage combiners, filter/address/sampler tuple and alpha/
depth semantics, with explicit program ABI rather than generic shader substitution.
Texture identity/content epoch/device generation must match accepted shroud.
Preserve the authored ObjectShroud/CELL_CLEAR numeric comparison at callers.

Build transform from bounded local view inverse, native origin offset and texture
extent scale in authored order, transpose-equivalent at transport. Reject singular,
nonfinite or nonrepresentable intermediate values before refs/access/maps/device
mutation. Preserve direct quantized pixel semantics already accepted by C2B.
Use existing source-stage transaction/retirement ownership, selected-stage-only
publication; no lazy load/init/LastAccessed mutation before tuple admission.
Failure leaves exact prior stage texture/refs/maps/filter/transform/program/state,
content pixels/epoch and unrelated pending shader/material/stages unchanged.
Successful UnInstall follows native reset: texture null, LESSEQUAL depth, ordinary
stage index and transform disabled. It cannot erase unrelated accepted state.
Every direct mutation obeys existing live-attempt poison-before-provider rule.

Generated camera-space coordinates required for this exact shader family are
bounded explicit lowering; unrelated generic FVF/coordinate rejections remain
unchanged. Prove constant/reflection/stage-output-index ABI and exact physical UV
sampling. No new public general matrix/shader API or Windows behavior change.

## Generated witnesses and exact commands

Register `original_w3d_prop_shroud` and generated `--gpu` mode; input contains no
prop owner/frame dependency. Cover release multiplicative and debug-alpha native
choices, exact EQUAL depth, program/state/stage identity, shifted origin, nontrivial
view inverse, COUNT2/camera mapping, content quantization/borders and physical
nonuniform pixels. Cover all install/apply/program/upload/commit faults, wrong/
stale/foreign/unready texture/provider, invalid tuple, singular/nonfinite/bound+1
matrix, stage index/output/transform mismatch, uninstall/reset and retry. Assert
unselected maps/material/shader state and accepted shroud pixels unchanged.

Build focus:
`cmake --build build/<preset> --target original_w3d_prop_shroud_tests original_w3d_tree_program_tests original_w3d_stage_transaction_tests original_w3d_source_reference_tests zh_original_w3d_full_probe -j4`.
Exact eight-control focus:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -R '^(original_w3d_prop_shroud|original_w3d_shroud_data|original_w3d_terrain_shroud_projection|original_w3d_tree_program|original_w3d_stage_transaction|original_w3d_source_reference|original_w3d_texture_decisions|original_w3d_material_abi_isolation)$' --output-on-failure -j1`.
Physical generated command on all four focused configurations:
`python3 tools/run_validation_clean.py build/<preset>/original_w3d_prop_shroud_tests --gpu`.
Run focus/strict/six-build/canonical/established Vulkan/LAN/ledger/diff exactly as
R0 specifies; record actual eight-ID selection and clean category audit.

## Readiness and commit

M22-R0-01 accepted providers; M22-R0-03 B implementation/registration is its output,
not an entry gate. A acceptance gates B entry; physical host availability remains
fresh PRE-012/016 verification at acceptance. No new tool, credential or M0.
Commit only B-owned helper/program/state/tests/registration/ledger and evidence/
status: `delivery: M22 08R0B lower exact shroud material pass`.
Keep A accepted and active08/renderer edits unstaged; checkpoint any new owner.
