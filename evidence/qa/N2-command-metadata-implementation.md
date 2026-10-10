# N2 logical command metadata — verified component checkpoint

Parent992b8b44; initially clean tree. Existing plan01/slice03 was refined before
production changes. Only game-owned source, tests and supporting artifacts change.
No upstream libraries, retail assets or recovery storage are touched.

Actual ControlBar metadata parsing/lookup now has explicit logical init/reset,
guarded partial initialization, pooled override candidate ownership, terminal
override-chain admission and correct interior intrusive-node retirement. Source
interactive init/reset share these operations; physical scheme/widget behavior
is not accepted by logical fixtures. Actual native client/root integration remains
pending, as does full milestone acceptance.

Retained iteration evidence:

- GCC81993 build terminal1: fixture attempted protected creator APIs and passed a
  const inspection pointer to mutable source admission. Corrected with a derived
  inspection-only probe and actual nonconst lookup before parsing overrides.
- GCC35915 retry build passed, focused4 terminal8/0.11s: generated MOVE is not in
  the original command table. Source token census verified ATTACK_MOVE; corrected
  fixture data, not command parsing or gameplay.
- GCC50125 canonical-token focused4 PASS0.13s terminal0.
- Clang sanitizer40016 focused4 PASS0.80s terminal0 on canonical-token fixture.
- GCC sanitizer20181 initial focused4 PASS1.79s terminal0.
- These intermediate passes predate the final repeated override-chain test and
  CommandSet terminal-admission correction. Final frozen evidence is still pending.
- Final focused4: GCC53935 PASS0.13s, Clang87967 PASS0.77s, GCC sanitizer57730
  PASS1.78s, all terminal0. Source frozen on canonical653
  `05bd13977ecb84328f8387e3762f977bb1820ffdb2d0a0cae7fe97e80b18ffa9`,
  inclusive660 `8c6a60804b257653d5099ca19dffcd058c8b4074418f0218f6ad1f3e5b85494c`.
  379 configured tests. Frozen configured builds/related189 PASS: GCC93912
  182.53s, Clang sanitizer22417 480.34s, GCC sanitizer97376 790.94s; all terminal0.
  No production edits occurred during these broader runs.

Logs /tmp/n2-command-metadata-<configuration>-{build,focused}.log; GCC retry and
canonical variants retain their own prefixes. Existing60s family bounds and
unsuppressed ASan/UBSan/LSan remain unchanged. No full379 or N2 acceptance claimed.

Remaining source contracts include nonempty science/image/template parents,
actual client/root composition, real reset/shutdown ordering and GameMain entry.
Durable verified findings and the next coupled cohort contract are in
docs/original-engine-formats.md and the same plan01/slice03.
