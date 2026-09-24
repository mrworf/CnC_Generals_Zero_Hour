# M22 plan 01 slice 08G2: deterministic audio event playback state

## Outcome and source boundary

Close the second source-initialization defect exposed by the redacted retail
Recording probe. Original object-template parsing copies an `AudioEventRTS`
value before playback has begun. Every ordinary constructor initializes the
event's other runtime state but leaves `m_portionToPlayNext` indeterminate;
the source copy constructor then reads that invalid enum. Establish the same
deterministic initial playback portion already used by the class reset path.

## Bounded implementation and non-scope

Initialize `m_portionToPlayNext` to `PP_Attack` in every ordinary constructor
that can create an event before copy or assignment. Preserve the copy and
assignment operators, explicit setter, attack/sound/decay transition order,
event lookup and physical audio behavior. Do not synthesize an audio backend,
change authored event selection, ignore missing required audio resources, or
add a retail-only exception.

## Tests, dependency and commit

Requires accepted slice 08G1 so the redacted source-init route reaches this
boundary without an earlier undefined shift. Extend generated original-owner
coverage to construct each overload, copy and assign it before playback, and
require `PP_Attack`; retain explicit portion transitions and malformed/event
provider negatives. Run focused GCC/Clang tests, both strict host sanitizer
gates, all six canonical non-GPU/non-LAN/non-retail suites, physical Vulkan
controls, serial LAN, dependency-ledger validation and `git diff --check`.
Commit one independently reviewable implementation slice:
`delivery: M22 08G2 initialize audio playback portion`.

Retail roots remain read-only and are not needed for implementation
validation. A later redacted probe may record only fixed aggregate stage and
sanitizer categories. Slice 08G2 does not admit the retail scene selector,
repair the source-scene borrow/unwind transaction, or claim Recording output.
