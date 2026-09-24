# M22 plan 01 slice 08F4A: map-owned terrain shroud notification

## Outcome and source route

Define the missing CPU `BaseHeightMapRenderObjClass::notifyShroudChanged`
method for an exact live map-owned terrain object. Native implementation
notifies an owned prop buffer only when present. The bounded CPU map has no
prop-buffer producer, so the source-equivalent result is an owner-checked
no-op, not a prop implementation. This method is independently callable and
is required by later 08F4B display cell dispatch.

## Scope and state

Require the published terrain alias, original GPU edge, primary scene,
loaded map and initialized shroud, with no prop buffer. Reject pre-init,
foreign/removed provider, detached or inconsistent scene/map owner, and a
forced non-null prop buffer without dereferencing it. Repeat and two fresh
generated generations must add no aliases or Recording resources. No user
authorization applies to this internal generated owner method; the explicit
test profile and exact source owners are its admission boundary.

Do not implement prop allocation/notification, display shroud dispatch,
08F construction selector, retail traversal, or pixels. The accepted 08D,
08F0 and production defaults remain unchanged.

## Validation and commit

Extend a generated map/shroud owner probe for positive repeat, negative
pre-init/provider removal/detach/malformed prop, recovery and teardown.
Check source identity, ledger and provider removal. Run full six configured
builds and canonical non-GPU/non-LAN/non-retail suites, canonical sanitizer
`detect_leaks=0`, strict host LSan both compilers, physical Vulkan controls,
serial LAN in all six, and `git diff --check`. One 08F4A production/test/evidence
commit after acceptance; 08F4B may then implement the display caller.
