# M22 08E1C0B2A: generated map-preview resource closure

Use original `getMapPreviewImage` with a generated map-preview file and the
accepted C0B1 MapCache plus mapped-image provider. Prove source path
normalization, source file copy to isolated XDG map-preview storage, mapped
image publication/reuse, missing file and absent-resource rejection, copy
failure/retry, two generations, and provider/removal evidence. Do not decode
or inspect preview pixels, create a load-screen/layout owner, or use retail
map bytes.

`positionStartSpots` and `updateMapStartSpots` are deferred to C0B2B: they
mutate source GameWindow/Gadget state and require GameInfo/GameText consumer
ownership on top of this preview resource path.

Acceptance: focused GCC/Clang/sanitizer probes, six portable full suites,
strict host LSan, physical Vulkan, serial LAN, provider identity/removal,
evidence and one isolated commit.
