# M22 08E1B3: generated video buffer and stream lifecycle aggregate

The original `SinglePlayerLoadScreen` opens its video through the generic
`VideoPlayer`/`VideoStream` registry, then allocates a `VideoBuffer`, and uses
the concrete Bink/W3D implementations for decode and texture upload.  Those
last two implementations depend respectively on proprietary Bink and legacy
W3D/Direct3D texture APIs, so they cannot be silently compiled or substituted
as one purported original route.

The dependency-safe closure is ordered as follows:

1. **08E1B3A** compiles and exercises the portable original
   `VideoPlayer`/`VideoStream` registry, close/reset and provider publication
   with a generated headless stream.  It has no frame payload or buffer.
2. **08E1B3B** provides a project-owned portable `VideoBuffer` allocation
   contract on the original base metadata, including format/extent/pitch and
   failure rollback.  It has no decoder or pixels.
3. **08E1B3C** composes generated decoded frames with the accepted buffer and
   original registry, preserving open/ready/decompress/render/advance/close
   order without Bink files or a W3D texture path.
4. This aggregate proves the two-generation composition required by the
   later SinglePlayer owner.

## Aggregate delivery contract

`original_video_stream_buffer_aggregate` is the dedicated coupled gate.  It
runs the accepted B3A original `VideoPlayer` registry witness, B3B portable
original-base `VideoBuffer` witness, and B3C live generated
`VideoStream`→`VideoBuffer` transaction in that dependency order.  Each
witness independently rejects its bounded invalid transition/failure inputs
and publishes its two-generation zero-ownership terminal marker; the coupled
gate rejects a missing executable, non-zero exit, or missing terminal marker.
The B3C witness is the live transaction: it uses the original registry and
base buffer together for `open → ready/decompress → render → next → close`,
then performs failure rollback and retry.  B3A and B3B additionally make the
original source close/free and provider-removal edges independently visible.

No aggregate production factory, layout owner, decoder, texture upload, or
retail route is added.  The aggregate therefore preserves default fail-closed
behavior and carries the B3A–B3C rejection, reset/re-entry, provider-removal,
and zero-ownership guarantees without claiming proprietary decode or pixels.

The original Bink and W3D implementations remain source evidence only until a
separate public decoder/renderer edge is planned. No retail bytes, private
media names, raw Direct3D, proprietary Bink dependency, or pixel claim is in
scope. Depends on 08E1A; required by E1B.
