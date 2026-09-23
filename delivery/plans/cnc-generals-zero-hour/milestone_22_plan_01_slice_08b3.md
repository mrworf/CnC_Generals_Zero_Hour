# M22 plan 01 slice 08B3: source volumetric-shadow prerequisite

Requires 08B2. The original volumetric manager is not linked in the CPU
full-draw target, so this slice must establish its bounded source owner,
update/draw/reverse teardown and Recording proof before any retail route may
admit shadow volumes. It may not substitute a decal, no-op, or flag-only
route.
