# N2 fixed-step native logical display timing

Parent `b79590c4`; same N2 plan01/slice03. Supporting native timing/consumer
checkpoint, not full GameClient updates, concrete providers or N2 acceptance.

Original source: W3DDisplay.cpp:1667,1753..1805 and ww3d.cpp:1180
(under GameEngineDevice/Source/W3DDevice/GameClient and
Libraries/Source/WWVegas/WW3D2 respectively). A game-owned value clock separates
fixed-step accumulated time from published animation time, preserves last-frame
suppression, freeze precedence, fast-time deferred publication and unsigned32
wrap. No wall-clock-derived simulation time or bgfx clock is introduced.

NativeHeadlessDisplay owns the clock per lifetime. Its logical entry reads actual
ScriptEngine, GameLogic, GameClient and primary ScriptedView state, requires
matching display/tactical publications and rejects worker or logic-update calls
before clock mutation. Source ScriptedView's configured step is shared. Actual
GameClient composed-headless code invokes the entry before ordinary View updates;
fast-camera-only ticks skip that normal update. Physical draw APIs still reject.

Generated actual Display/View/client/Drawable consumers prove source33ms steps,
zero standard allocation demand, current pose advancement, repeated-frame
suppression, real initialized ScriptEngine freeze and camera-movement freeze,
same-frame unfreeze suppression, configured17ms replacement View, absent/wrong
parent rejection, worker/phase rejection and corrected same-owner retry. Direct
View deletion uses its original owner-withdrawal path. Display reset preserves
time, while a fresh display starts0. Fast deferred time is a clock-state test:
the retained native ScriptEngine's debugger predicates return false. No fake
production override or debug UI is substituted.

The real ScriptEngine constructor/init/reset/destructor cohort is warmed with
the same generated namespace and owned parents before ownership measurements.
Current model replacement sweeps retain each failure/retry and independent
constructor/pool checks. The broader matrix includes actual native client
construction, registration, camera/View ownership and complete composed-service
reset/fault paths; this does not assert that the missing full update route ran.

## Validation

Initial fixture compilation attempted protected Display::deleteViews. It was
corrected to legitimate View destruction/owner withdrawal, without opening a
production cleanup API or weakening ownership. Focused1 PASS2.22s.

Frozen canonical716 SHA256
`f21b8a01d0f9a3a9954b5bade1c39fbe25af06c5af67194cfdaeddee58e24b53`;
inclusive723 SHA256
`9e58ade532e70bf6e4fe5a67c99944b46f123cc0a2524f391a02a3f96715f65a`.
Hash procedure relative to9e5c0e4f matches N2-animation-clock.md.
Frozen related120 PASS GCC110.66s/GCCsan443.52s/Clangsan283.05s; all processes
exited0, no live jobs remain. All218 prefix/composed-client manifest lines match
across configurations. Timed current replacement cold173/seeded176, advancement/
reads0, source Drawable construction standard1/module-pool2 and composed-client
terminal205 retain all prefixes/retries and independent pooled retirement.
Full configured457 unrun; this checkpoint is not slice03 or N2 acceptance.
All ASan/UBSan/LSan enabled, independent60s family timeouts unchanged.
Logs `/tmp/n2-display-time-<configuration>-frozen-{build,related}.log`.
Build the N2-animation-clock.md targets plus original_native_client_fixture and
original_native_font_fixture. Verbose CTest selector:

```text
^original_(draw_provider_data_|drawable_queries_|w3d_(draw_data|assets|animation|document)_|data_|core_|header_paths$|graph_headers_|module_data_|template_backing_|runtime_(names|floating-point)|native_client_|native_font_composition)
```

Pending: complete actual GameClient update with loaded simulation/terrain/UI;
source pause/event/selection/transitions and concrete19 providers; bounding
primitives/all-LOD bounds; condition transfer and remaining materials/prototypes;
whole native startup/full suite. Logic pause is read by production, but its full
audio/input/UI setter cohort is not introduced into this focused fixture.
No physical/GPU/retail execution or parity claim. Assets, dependencies and
recovery storage remain untouched.
