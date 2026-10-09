# N2 side/script ownership work-in-progress checkpoint

This checkpoint preserves the outstanding implementation at the user's request
to sort and commit the worktree. It is **not** slice or milestone acceptance.
Parent: `8d705d9d329d5aba2cbc375a70ec931a201605f4`.
Governing work: N2 plan01/slice03, packet `linux-upstream-v2`.

## Scope

The batch contains original SidesList/Scripts headers and implementations,
the actual-source `tests/original/sides_scripts.cpp` fixture, CMake registration
of five test families, and the existing slice-plan continuation. It stages
side/script publication, guards partially acquired trees, and adds transactional
copy/setter ownership. All production changes are in Zero Hour code, not adopted
dependencies. Assets and recovery storage are untouched.

## Validation at the checkpoint

`cmake --build build/original-core-gcc --target original_sides_scripts_fixture -j 2`
completed successfully; Ninja reported no work to do.

Normal-host command:
`ctest --test-dir build/original-core-gcc -R '^original_sides_scripts_' --output-on-failure`
completed in 8.47 seconds with exit code 8:

| Family | Result |
| --- | --- |
| functional | PASS |
| negative | PASS |
| copies | FAIL: complete bounded source census |
| faults | FAIL: every complete side/script prefix and exact terminal |
| setters | FAIL: complete bounded source census |

These are completed failing executions, not timeouts. No current-batch sanitizer
or broader-suite acceptance is claimed. Prior polygon-world evidence applies to
the preceding frozen source, not to this checkpoint.

## Resume requirements

- Diagnose allocation census and exact-terminal invariants across the complete
  parse/copy/constructor/setter family. Warmed original pools can mean zero
  standard allocations; verify that possibility rather than forcing a nonzero
  census or weakening failure/retry coverage. The parse failure's cause remains
  unverified.
- Finish the governing plan's modern name-key rematching, unknown-name fallback,
  nested malformed counts/types/versions, legacy conversions, pending read-state
  rollback/capacity, and accepted-target copy/assignment coverage.
- Validate the frozen batch with GCC and both compiler sanitizer configurations,
  retaining every discovered failure/retry pair and exact terminal; run the
  affected broader matrix and record durable engine-format findings.
- Do not mark N2 or slice03 complete. Enclosing world/object/Dict/weather/name
  publication, native terrain/root and provider ordering remain pending.

No implementation fixes or coverage reductions were made during this worktree
housekeeping transaction.
