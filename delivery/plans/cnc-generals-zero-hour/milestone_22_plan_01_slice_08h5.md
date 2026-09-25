# M22 plan 01 slice 08H5: authored terrain consumer aggregate

## Outcome and dependency order

Requires slices 08H5A through 08H5D in order. Couple static base/primary
terrain vertices, transactional extra-blend inventory, the bounded road-base
alpha material and source-ordered dynamic extra-blend submission through one
generated authored terrain scene.

## Acceptance

The aggregate fixture exercises multiple base/edge classes, primary and extra
blend records, custom edge selection and cliff state through the same original
height-map owner used by scenario construction. Assert exact base-before-extra
Recording categories, failure rollback at every child boundary and clean
two-generation re-entry. No retail input is used in this prerequisite.

Run focused GCC/Clang, strict host LSan, all six canonical nonretail suites,
physical Vulkan, serial LAN, dependency ledgers and `git diff --check`. One
aggregate commit records only coupling/evidence changes:
`delivery: M22 08H5 close authored terrain consumers`.

## Result

Complete. The generated authored scene composes the accepted static
base/primary vertices, exact extra-blend inventory, road-base alpha material
and source-ordered dynamic extra submission through one original height-map
owner. Multi-class base/edge atlases, primary/custom/extra blend records and
cliff-authored topology reach ordered base-before-extra Recording draws with
child-boundary rollback and two-generation cleanup. See the
[08H5 evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08h5.md).
