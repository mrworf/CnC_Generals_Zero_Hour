# M22 plan 01 slice 08P0C2B: exact shroud stage binding

## Goal, dependency and boundary

After C2A's exact tree program, restore native `W3DShaderManager::setShroudTex`
stage/resource/transform semantics on the CPU shipping route. No tree draw,
C1 advancement, scene preparation, decal, physical factory or retail admission
is included. This generated local renderer operation has no authorization gate.

## Source binding and lifecycle

Resolve the exact active terrain/map/shroud texture, positive finite cell size,
texture dimensions and draw origin. For native stage 1, preserve camera-space
position coordinate selection, COUNT2 transform, TEXTURE/CURRENT MODULATE color
and SELECTARG2 alpha. Construct the canonical inverse-view × origin offset ×
scale transform, with one-cell border offset and source row/matrix orientation.
The tree vertex program uses its own unswayed world position and c32/c33;
preserve subsequent tree-loop UV-index/transform-disable state in source order
and do not double-apply the camera-space transform. Do not bind an unshrouded
pass or replace the exact source texture with a placeholder.

Readiness must be side-effect free. Preflight every owner/resource and matrix
before delayed state mutation; preserve the existing asserting/query semantics
outside any narrow readiness peek. On provider/resource failure preserve prior
accepted source stage/transform/resource identity; binding failure unwinds
owned references and state, and retry succeeds without reset. Source owner
removal and active-generation mismatch reject before partial binding. A new
public capability owner, if independently needed, requires a plan-only split
before implementation. No tree registry, RNG or effects may change.

## Surfaces, tests and acceptance

Expected surfaces: canonical shader manager CPU branch/types, original shroud
access/readiness if required, existing DX8 delayed state/transform transport,
source identity ledger and generated binding witness. Native Windows behavior
and serialized/native class layout remain unchanged.

Generated positives exercise nontrivial camera/inverse view, shifted origin,
unequal cell dimensions, actual shroud texture/generation and the tree-program
constant/stage sequence. Assert exact color/alpha operations and matrices,
not only texture presence. Negatives cover null/stale/removed terrain or
shroud, singular/nonfinite matrix, invalid dimensions, missing/stale texture,
binding failure, unchanged prior state and clean retry across two generations.
Identify/persist exact focused commands before production. Run GCC/Clang and
sanitizer focused witnesses, six complete builds/canonical nonretail suites
(`-LE 'gpu|lan|retail'`, sanitizer `ASAN_OPTIONS=detect_leaks=0`), exact serial
host LSan (`ASAN_OPTIONS=detect_leaks=1`, no UBSan override), established physical
Vulkan, serial LAN 4/4 all six, ledger and diff on final source. Commit one slice:
`delivery: M22 08P0C2B bind exact tree shroud stage`.
