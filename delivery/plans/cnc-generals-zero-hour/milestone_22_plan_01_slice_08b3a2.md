# M22 plan 01 slice 08B3A2: source volume-geometry CPU closure

Requires 08B3A1.  Compile/extract the smallest original
`W3DVolumetricShadow` source-owned bounded geometry utility that consumes its
source slot family and produces generated volume vertices/indices for 08B3C.
It must validate geometry cardinality, overflow, malformed/foreign input,
resource failure/retry, reset/re-entry, removal, and zero ownership.  It
cannot expose raw Direct3D or private Vulkan APIs, admit any shadow owner,
substitute decals/projected shadows, read retail content, or claim pixels.
