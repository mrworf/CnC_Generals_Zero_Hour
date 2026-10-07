# Upstream-only Zero Hour Linux port

The user authorized a fresh start from
`0a05454d8574207440a5fb15241b98ad0b435590`. The active specification is
`docs/zero-hour-linux-port-plan.md`; packet `linux-upstream-v2` replaces every
earlier milestone and renderer contract. The ignored `.port-recovery/` directory
is historical recovery storage, never an active source or build dependency.
Do not automatically import old implementation or acceptance claims.

## Non-negotiable boundaries

- Adopted frameworks, libraries, and their tools must remain upstream-clean.
  Use supported public APIs and documented build configuration. Never patch
  vendor sources, copy private backend internals, or depend on private headers.
- Zero Hour source may be redesigned. Legacy Visual C++/MSVC compatibility and
  internal Windows ABI preservation are not requirements. Serialized formats,
  gameplay, and intended audiovisual behavior remain requirements.
- Retail asset links and their targets are read-only. Users provide these files.
  Runtime decoding and disposable local caches are allowed; never distribute
  proprietary assets or derivatives. Do not print private selectors or roots.
- Work in this chat; do not launch agents. Do not reset, clean, or modify the
  recovery archive as part of ordinary implementation.
- bgfx is a candidate until N1 passes on a pristine build. A required patch or
  visual compromise fails that gate and requires architecture reassessment.

## Delivery discipline

Write a governing and coherent slice plan before production changes. Follow
the dependency graph; commit independently useful, verified behavior slices.
Use proportionate focused tests during iteration, then the required broader
matrix on frozen source. Do not repeat assert/fix/full-build loops: inspect the
surrounding lifecycle and parallel paths, then batch coupled corrections.
A timeout means incomplete execution, not a diagnosis or permission to weaken
coverage. Split independent sweeps while retaining each failure/retry pair.

Consult `docs/original-engine-formats.md` before investigating engine formats or
ownership. Record verified source findings, evidence links, and pending limits.
Historical observations are not current acceptance.

The original engine replaces ordinary global allocation with memory pools.
Inspect allocation and deallocation together, including temporary/nothrow paths,
and validate new original-runtime ownership with GCC and Clang sanitizers.
Native handles can represent multiple acquired references; release ownership
units, not distinct numerical handles. Guard constructor acquisitions before
fallible helpers. Avoid callbacks into simulation from render/audio threads.

Preserve packed attributes, source-selected mip levels, filters and transforms.
Descriptor counts alone do not prove initialized backing. Check original filter
tables and source rendering behavior before writing a visual oracle. Do not
restore old shader block layouts when explicit stock shader inputs suffice.
Use defined raw representations for malformed-input tests; do not introduce UB
while trying to test rejection. Preserve fixed-width file/network encodings.

Validate real draw cardinality and upload volume, ordered clears/copies, repeated
scene lifetimes, and resource growth. Do not infer whole-game parity from a
triangle, mocked renderer, or isolated buffer test. Physical GPU evidence must
name the dependency revision and configuration actually executed.
