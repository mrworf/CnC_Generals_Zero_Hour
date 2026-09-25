# M22 slice 08P0B3B: atomic tree geometry and Recording resources

The CPU-only tree owner now composes accepted model references and bounded
atlas pixels into source XYZNDUV1 vertex/index wrappers and one procedural
Recording atlas. Per-instance random scale and sway are previewed from the
exact six-word GameClient stream. A candidate builds and uploads all geometry
and atlas resources before type/instance publication and no-throw RNG commit;
every negative path discards the preview and retains accepted source pointers,
resource counts and registry identity. The source edge also rolls back its
texture mapping if handle-ref bookkeeping allocation fails after insertion.
The physical `W3DTreeDraw` factory remains closed pending 08P0C.

The generated textured W3D/TGA witness verifies native vertex positions,
index winding, UV atlas transform, initial sway/darkening/base-height slots,
and byte equality between source wrappers and Recording uploads. It checks
the source 29997/29998 vertex and 59991/59994 index boundary pairs, missing
UVs, missing providers, duplicate IDs, 64 types and 4000 instances, resource
faults from geometry through texture publication/registry, Recording creation
and upload faults for both buffers and atlas, accepted-owner preservation,
remaining-type atlas rebuild after removal, relocation and final pre-teardown
zero residual. Failed admissions leave the next successful scale/sway sequence
equal to a saved fresh-equivalent client generation. Two device generations
and retry are covered. The existing fault hooks cannot deterministically
force the internal `std::map` handle-ref allocation in `publish_texture`;
the code-path rollback is audited, while adjacent texture-create/upload/
publish injections prove no half-published source or Recording residual.
The expanded deterministic 64-type/4000-instance generated witness takes
about 55 seconds across two normal process generations; under sanitizers a
single generation exceeds the shared runner's 30-second default. Only this
script opts into a 120-second generated-probe timeout, with all assertions
and workload retained. An initial strict LSan run and the first sanitizer
suite runs hit that obsolete timeout; final-source reruns supersede them.

Final-source validation: six complete GCC/Clang Debug, Release and
ASan+UBSan builds and six canonical nonretail suites passed 267/267 each
(`-LE gpu|lan|retail`, sanitizer suites with
`ASAN_OPTIONS=detect_leaks=0`). Post-audit focused GCC/Clang generated
display-owner and terrain witnesses passed 2/2 each. Strict host LSan with
`ASAN_OPTIONS=detect_leaks=1` passed 2/2 on both sanitizer toolchains;
the generated terrain witness took 219.25 seconds across two GCC process
generations and 185.60 seconds across two Clang generations. Physical
Vulkan display/bootstrap/map passed 3/3 GCC and display/map passed 2/2
Clang. Serial host LAN passed 4/4 in all six configurations. The original
dependency ledger and `git diff --check` passed. The unrelated renderer
diagnostic remains unstaged.
