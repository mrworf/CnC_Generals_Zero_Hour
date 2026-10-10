# N2 original pristine-query checkpoint

Parent `3fb53069`; same N2 plan01/slice03, not milestone acceptance.

Generated original draw INI -> original selected condition -> owned first/final
model binding -> original pristine map/weapon/turret tables -> original bone and
position lookup executes without renderer objects. Definitions remain unchanged.
W3DModelQueries.cpp shares the original weapon/turret derivation and lookup
bodies with the legacy draw wrappers, which retain their phase/FPU checks.

Native preparation stages the complete original condition candidate under an
asset-cache snapshot and outer NameKey transaction. Late allocation failure
retires the candidate before asset and namespace rollback; successful return
publishes the whole owned query with a nonthrowing move. Cold and populated
cohorts verify retained identity, cache counts, exact standard backing, pooled
name used units, and production cursor before outer fixture rollback. Every
failure/retry pair is retained, with three full repetitions after complete warm-up.

Source behavior checked: first/final animation, nonzero named-bone sentinel,
unadorned/contiguous01..99 discovery, gap termination, full-name-before-suffix
subobject lookup across all LODs then additional models, original unadorned and
numbered weapon barrels, previous-FX reuse for extra muzzle flashes, separate
launch transforms, turret indices/positions, invalid-key sentinel reset, missing
model/wrong hierarchy/nonfinite scale rejection. Subobjects use their actual
attachment bone, removing the original global-index-into-LOD0 undefined shortcut.
Numbered formatting uses original owned AsciiString rather than fixed256-byte
unchecked buffers. Proxies remain metadata, not invented subobjects.

Frozen canonical680 SHA256
`8addc2134fb51ef6cd9bbde3faf3153d803874125fd5cb0a31b1201ef47c4d56`;
toolchain-inclusive687 SHA256
`ad6bec10cef3378d9205361a77f90e72cd85b7031820e34d7eeb63fd681b2258`.

Final normal-host related77 matrix:

| Configuration | Result | Terminal |
| --- | --- | --- |
| GCC | 77/77 PASS, 5.70s | 0 |
| GCC ASan/UBSan/LSan | 77/77 PASS, 34.13s | 0 |
| Clang ASan/UBSan/LSan | 77/77 PASS, 24.22s | 0 |

Exact query allocation terminals in all3: cold207, populated52. The preceding
draw-parse/template and original asset/document/motion/core/data/header/module
ownership cohorts remain included. Configured434 full suite unrun; no leak
suppressions, coverage waivers or timeout changes. No live jobs remain.

Build targets and verbose CTest selector match the preceding draw-definition
checkpoint. Final logs `/tmp/n2-pristine-<configuration>-closeout-{build,related}.log`.
Initial77 PASS5.73s/34.05s/24.32s remains in `-frozen-` logs; inherited trailing
whitespace in the extracted source was normalized, then the matrix rerun on
the exact final source. No executable behavior changed in that closeout.
Focused8 PASS1.15s and expanded numbered8 PASS1.17s are retained in `-reviewed-`
and `-numbered-` logs. The first fixture compile rejected two mixed-type auto
declarations; both were corrected together, with no production behavior change.
That compile attempt remains in `/tmp/n2-pristine-gcc-focused-build.log`.

Not accepted: actual Drawable dispatch and logic-phase/FPU integration, current
client pose/attachment queries and nested transforms, derived19 draw
registrations, whole native ModuleFactory, remaining material/prototype types,
GameEngine startup/loaded simulation, particle rendering or retail visual parity.
No adopted dependency sources or retail files were changed. N2 remains in progress.

Post-checkpoint source review distinguished the serialized HLodDef constructor
from the separate in-memory LOD-array constructor: only the latter flattens its
children. File-loaded nesting must remain intact. This resolves the speculative
flattening prerequisite without changing production source or acceptance claims;
anchors and remaining recursive-transform boundary are in original-engine-formats.md.
