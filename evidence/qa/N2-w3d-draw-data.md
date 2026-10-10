# N2 original draw-definition checkpoint

Parent `bbcd6554`; existing N2 plan01/slice03 remains in progress.

The original W3DModelDrawModuleData definition, complete base draw field tables,
default/condition/alias/transition parsing, selector, constructor/destructor and
transfer methods now live in shared W3DModelDrawData.h/.cpp. The original draw
header consumes the same class and its creator forwards to guarded original
parsing. No alternative parsed condition schema or toy selector was introduced.
Renderer-specific methods remain separate and are not linked by this fixture.

Generated original INI -> original module data -> original selector -> owned
first-animation pose executes with original FileSystem, NameKeyGenerator and
ParticleSystemManager template initialization/lookup/retirement. Three abstract
particle presentation methods are fixture seams: render callbacks reject rather
than reporting output. Tests preserve default inheritance, animation replacement,
weighted repeats/None, aliases, ignore flags, visibility lists, weapon/turret
public names, transition IDs, particle template references and final-frame muzzle
queries. Existing shared generated W3D inputs were factored into W3DAssetInput.h;
retail inputs are not used or changed.

createFromINI holds a NameKey transaction outside the candidate owner: parse
failure destroys key-bearing data before key rollback. An unparsed nullptr-INI
creation does not require an unused namespace parent. Native pointer parse tags
use bounded intptr_t admission, preserving source animation indices0..5 and
transition signature widths. Matrix transfer remains48 bytes. The invalid
attach-bone cache vector is initialized before whole-data copies. Intended source
idle-animation distance errors now reject in release too.

Particle-template parsing requires the actual parent and rejects missing named
templates rather than silently treating them as None. Original newTemplate now
guards the pooled candidate through fallible registry insertion. Cold source
template insertion failures preserve the accepted Spark template and retire the
pooled candidate exactly; manager destruction retires all standard registry
backing. Template keys are staged before ownership measurements so inline
literal-to-AsciiString temporaries cannot contaminate a live-count comparison.

Final canonical679 SHA256
`b6b1f24c276a677d0af6ca2532c10201e400953e6c3a93c3648d4dd1537366b4`;
toolchain-inclusive686 SHA256
`20c866dfd83e6bfed8433fa91958074b8aff0df55fcd3b4c2774df11adc62f31`.

| Configuration, normal host | Related tests | Terminal |
| --- | --- | --- |
| GCC | 74/74 PASS, 5.20s | 0 |
| GCC ASan/UBSan/LSan | 74/74 PASS, 30.64s | 0 |
| Clang ASan/UBSan/LSan | 74/74 PASS, 21.67s | 0 |

Exact terminals in all3: cold draw parse104, populated prior-module parse101,
particle-template insertion1. Every failed prefix and successful same-owner retry
is retained, with three full repetitions and initial complete-provider warm-up.
Failed parse preserves prior module identity and standard live backing; NameKey
pool used units and next production cursor are proved before an outer fixture
lease rolls back. Successful retries execute original selection/pose queries.
The original asset, document, motion, core, data, header, module and template
ownership tests remain included in the related matrix. No suppressions, coverage
waivers or timeout increases. Configured431 full suite unrun.

Build: same eleven fixture/audit targets as the prior related69 cohort, plus
original_w3d_draw_data_fixture (real full original archive group, gc-sections).
CTest -V selector:
`^original_(w3d_(draw_data|assets|animation|document)_|data_|core_|header_paths$|graph_headers_|module_data_|template_backing_|runtime_names)`.
Final logs `/tmp/n2-draw-data-<configuration>-closeout-{build,related}.log`.
The earlier74 PASS5.16s/30.67s/21.55s remains in `-final-` logs. Final source
adds original copyright attribution and normalizes four inherited whitespace
lines; the complete related matrix was rerun on that exact source before commit.
Initial supporting73 PASS4.88s/28.41s/20.07s is retained in `-frozen-` logs.
Focused generated-fixture compile failures and the incorrect pool lookup/inline
temporary baseline failures remain in earlier `/tmp/n2-draw-data-gcc-*.log`;
source-verified corrections were made without weakening ownership expectations.

Not accepted: all derived19 draw registrations, whole native ModuleFactory,
weapon/turret/subobject bone-cache publication, actual Drawable consumers,
remaining material/prototype types, full GameEngine startup/loaded simulation,
particle rendering or retail visual parity. Source transfer methods compile but
their condition-specific runtime roundtrip is still pending. No upstream
dependency sources or retail files were modified. N2 and later milestones remain
pending; no completion claim follows from this supporting base-definition path.
