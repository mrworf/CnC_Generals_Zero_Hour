# M22 plan 01 slice 08P0C2D0B2R2: defined source texture-filter admission

## Goal, dependency and boundary

After D0B2A and independently accepted R1, make all generated Linux source
filter/profile/address negative values have defined representations and reject
the complete tuple before any native/source filter-table access or stage
publication. Close historical texture-decision sanitizer categories before B2B.
No bgfx reservation/journal, renderer sampler semantics, private retail input,
Windows/retail declarations, serialized state, allocator or gameplay change.
Authorization is not applicable to generated local controls.

Read-only evidence: unchanged fixture line 426 constructs TxtAddrMode 9 and
line 462 constructs TextureFilterMode 99. GCC CTest 251 reports getter
texturefilter.h:112; Clang CTest 276 also reports setter:114 and profile
comparisons texturefilter.cpp:133/134. Existing enums have bounded unfixed
representable ranges, so reading these synthetic values is undefined before
the intended rejection. Neither executable links native bgfx or reaches A.
Current Linux Apply also publishes min/mag/mip before U/V validation.

## Defined representation and atomic tuple validation

Both GCC and Clang read-only compile controls confirm existing FilterType,
TextureFilterMode and TxtAddrMode underlying types are unsigned with unsigned
size/alignment. Use explicit unsigned underlying representations for these
three enums on Linux only, consistently under the platform macro across every
archive/consumer, not an optional source-body macro. Keep enumerator values,
class member order/size/alignment and Windows declarations unchanged. Fixed
unsigned representation makes all generated raw unsigned values defined;
unsupported values still fail closed, never become accepted source semantics.
Add compile/runtime ABI controls for unsigned types, exact enum/class extent
and untouched non-Linux declaration selection; no implicit-padding comparison.

Before OriginalGpuEdge::required, any table lookup or set_filter_stage_state,
Apply validates stage < 8, all min/mag/mip indices < FILTER_TYPE_COUNT and
both U/V equal repeat or clamp. Convert to the defined raw unsigned domain for
checks. Then retain the accepted source publication order min, mag, mip, U, V.
Profile initialization accepts only bilinear/trilinear/anisotropic and rejects
all other raw values before changing any table entry. Each public default-filter
setter also validates its index before table access or changing any default.
Valid values must retain
the original stage-zero anisotropic downgrade, mip and address semantics.
Do not shift a late invalid into a partially published source state.

The texture-decision witness continues to exercise the real TextureClass and
TextureFilterClass paths, including raw values 9 and 99 now defined, exact
maxima/bound+1 and UINT_MAX for each field/profile/stage. Invalid tuples prove
unchanged Recording event count/content, prepared-state revision/pending sampler
identity/descriptor, public resource count and all filter tables, then a clean
valid retry reproduces expected stage order. No draw/RNG/GPU/frame mutation or
provider creation occurs on rejection. Missing provider plus invalid tuple must
fail at tuple admission first. Reset/two-generation teardown stays exact.

## Surfaces and acceptance

Expected surfaces: original WW3D2 texturefilter.h/.cpp and texture_apply.inc, generated
test_original_texture_decisions.cpp, ledger and adjacent evidence/index.
Verified TextureClass::Apply in texture_apply.inc initializes the texture,
changes LastAccessed and selects pending texture before calling Filter.Apply.
Add a non-mutating Linux-only complete tuple predicate on TextureFilterClass
and invoke it at the start of the real TextureClass::Apply, before Init,
LastAccessed or select_texture; Filter.Apply rechecks the same predicate before
its own table/provider/stage effects. Negative fresh/uninitialized texture and
missing-provider controls prove this order through the public route, not only a
direct helper. No broad source API rewrite or duplicate native
sampler validator. Preserve unrelated renderer diagnostic unstaged.

Focused build: `cmake --build build/<preset> --target original_w3d_texture_decision_tests -j4`.
Focused tests: `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset>
-R '^(original_w3d_texture_decisions|original_w3d_texture_identity|original_w3d_texture_provider_removal|original_w3d_presentation)$'
--output-on-failure` on both native and both sanitizer configurations. Scan the
complete output, not exit codes alone. Exact serial host LSan uses
`ASAN_OPTIONS=detect_leaks=1`, no UBSan override, the corrected presentation
and texture-decision CPU fixtures on both sanitizer configurations.

Require six complete builds and six canonical `-LE 'gpu|lan|retail'` suites
on frozen corrected source, with complete-log sanitizer audits clean for all
R1/R2 categories. Native generated A reservation/resource controls, exact host
Vulkan display/map controls and existing original texture physical controls
prove no unrelated transport regression. Serial host LAN 4/4 all six plus
ledger/header/ABI/diff checks and exact stage review are mandatory. No historical
UBSan category may be suppressed or treated as repaired by successful exits.
Commit one coherent source admission owner:
`delivery: M22 08P0C2D0B2R2 admit defined texture filter values`.

## Planning checkpoint

Implementation-ready plan only, persisted before source changes. B2B remains
closed until R1/R2 independent commits and the clean final matrix complete.
