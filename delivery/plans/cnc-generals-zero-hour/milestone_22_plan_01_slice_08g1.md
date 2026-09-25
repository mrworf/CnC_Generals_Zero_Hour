# M22 plan 01 slice 08G1: zero-based veterancy flag safety

## Outcome and source boundary

Close the first source-initialization defect exposed by the redacted retail
Recording probe before scenario construction. `VeterancyLevel` is a
zero-based enum (`REGULAR` through `HEROIC`), but the three inline flag
helpers currently shift by `level - 1`. An authored exclusion of the first
level therefore evaluates a negative shift before the retail scene can reach
map construction. This is a source correctness prerequisite, not retail
content support or scene admission.

## Bounded implementation and non-scope

Change `getVeterancyLevelFlag`, `setVeterancyLevelFlag`, and
`clearVeterancyLevelFlag` to use the zero-based enum value as the bit index.
Reject or otherwise fail safely for values outside the declared level range
before shifting; do not reinterpret `ALL`/`NONE`, change the enum, alter INI
token syntax, or add a retail-specific bypass. Preserve all unrelated combat,
experience and object-template behavior.

## Tests, dependency and commit

Requires accepted slice 08F. Add generated positive and negative coverage for
all four valid levels, independent set/get/clear behavior, `ALL`/`NONE`, and
invalid values without undefined shifts. Exercise the original INI flag
parser with generated tokens, including exclusion of the first level, and
verify malformed input still rejects. Run focused GCC/Clang tests, both strict
host sanitizer gates, all six canonical non-GPU/non-LAN/non-retail suites,
physical Vulkan controls, serial LAN, dependency-ledger validation and
`git diff --check`. Commit one independently reviewable implementation slice:
`delivery: M22 08G1 make veterancy flags zero based`.

Retail roots remain read-only and are not needed for implementation
validation. A later redacted probe may record only fixed aggregate stage and
sanitizer categories. Slice 08G1 does not admit the retail scene selector,
initialize audio events, repair scene-owner unwind, or claim Recording output.

## Result

Complete. The three original helpers now map the four valid zero-based enum
values to bits zero through three. The named invalid enum value returns false
or preserves the supplied mask before any shift. Generated unit coverage
checks every bit and invalid operation, while the generated scenario fixture
drives mixed first-through-last modifiers through the original INI parser.
No selector, retail provider or audio behavior changed. See the
[08G1 evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08g1.md).
