# M22 plan 01 slice 08B3A: aggregate CPU source prerequisite

Requires 08B3A1 and 08B3A2.  The source buffer-slot allocator and the bounded
volume-geometry extraction are separately reviewable prerequisites for the
later public stencil contract.  This aggregate records their composed CPU
closure.  It does not admit `SHADOW_VOLUME`, raw Direct3D, a private Vulkan
interface, projected/decal substitution, retail reachability, or pixels.
