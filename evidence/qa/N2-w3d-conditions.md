# N2 original model-condition data checkpoint

Parent: `2f6a09b0`. Same N2 plan01/slice03, still in progress.

The original `W3DAnimationInfo`, `ModelConditionInfo`, transition signatures and
renderer-independent lifecycle/mode/public-bone methods now live in shared
`Common/W3DModelConditionData.h` / `System/W3DModelConditionData.cpp`. The
original W3DModelDraw header consumes those same definitions; no parallel parsed
condition schema was introduced. Animation mode storage is an Int with unchanged
source values (default ONCE=2), removing the nested renderer enum dependency.
The supported NO_RETAIN_ANIM_HANDLES metadata lifecycle is unchanged.

`ModelConditionInfo::prepareNativeBinding` admits an owned model/first-animation
pose using the original deterministic query rules: first animation, frame0 or
frames-1 for flag4, hierarchy scale outside abs(scale-1)>.01, and unconditional
external-root scale. Model, clip and pose prepare in one offside asset-cache
transaction; missing/incompatible inputs and late pose allocation failures
preserve all accepted cache backing. No original bone cache is marked valid by
this supporting query boundary. Legitimate no-model/null states remain distinct
from missing required assets; an empty authored first clip rejects.

Generated tests use original FileSystem and original condition/flag/matcher
types, not a separate selector implementation. They check alias matching,
intersection/extraneous-bit ordering, clone cache address isolation, actual
night/snow alias matching, public-bone normalization, first/final pose frames,
both scale factors and epsilon, missing clips, incompatible named skeletons,
cache retirement, and coupled cold/populated-cache ownership failure/retry.
The populated-cache case retains an actual Static.Mesh definition/document/model
identity, rather than only an explicit NULL entry.

Frozen canonical675 SHA256
`0049583d583569d8629afdbbe8bedd3cfbce3e1e52bfbeb0e5fec497cd519256`;
toolchain-inclusive682 SHA256
`569c3176da391ee9e0e2a875abdd1e3ce20f286ee905953819dc8324f1243023`.

| Configuration, normal host | Related tests | Terminal |
| --- | --- | --- |
| GCC | 69/69 PASS, 4.54s | 0 |
| GCC ASan/UBSan/LSan | 69/69 PASS, 26.03s | 0 |
| Clang ASan/UBSan/LSan | 69/69 PASS, 18.58s | 0 |

No suppressions or timeout increases. Exact condition terminals cold172 and
seeded175 in all configurations; all prefixes and successful same-owner retries
retained, three full repetitions. Failure checks original bone-cache flags,
asset-cache counts/accepted identity, exact standard live backing, descriptors
and NativeDataFile pooled used units. Whole-owner destruction is checked too.
The prior asset/motion/document failure/retry families remain in related69.

Build targets: original_w3d_assets_fixture, original_w3d_animation_fixture,
original_w3d_document_fixture, original_data_fixture, original_data_audit,
original_core_fixture, original_graph_headers_fixture, original_module_data_fixture,
original_template_backing_fixture, original_runtime_fixture. CTest -V selector:
`^original_(w3d_(assets|animation|document)_|data_|core_|header_paths$|graph_headers_|module_data_|template_backing_|runtime_names)`.
Logs: `/tmp/n2-condition-data-<configuration>-frozen-{build,related}.log`;
final whitespace-normalized source uses `-final-{build,related}.log`.
Initial matrix PASS4.59s/26.60s/19.19s is retained; staged diff-check then
detected inherited trailing whitespace in the extracted original code. It was
removed, hashes refreshed and all builds/related69 rerun on final source.
Focused GCC12 and then coupled13 runs passed before final frozen69.

Configured426 full suite unrun. This does not compile/execute the entire legacy
W3DModelDraw rendering translation unit, register all draw definitions, parse
real draw INI, populate weapon/turret/subobject caches or execute actual Drawable
query consumers. Those are next integration work, followed by native root/full
startup/loaded simulation. N2/slice03 and all later milestones remain pending.
Retail inputs and upstream dependencies were not modified.
