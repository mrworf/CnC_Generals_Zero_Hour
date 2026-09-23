# M22 plan 01 slice 08B3: aggregate volumetric-shadow closure

Requires the ordered 08B3A, 08B3B, and 08B3C prerequisites.  The original
volumetric manager cannot be added directly to the CPU full-draw target: its
source uses the legacy buffer-slot allocator, shadow-geometry construction,
raw D3D8/D3DX calls, and a two-pass stencil-volume/composite protocol.  This
aggregate records their composed mask-bit-2 closure only after 08B3C proves a
bounded source owner.  It may not substitute a decal, a no-op, a flag-only
route, raw Direct3D, private Vulkan APIs, pixels, or retail reachability.
