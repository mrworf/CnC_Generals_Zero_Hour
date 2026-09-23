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

The original Bink and W3D implementations remain source evidence only until a
separate public decoder/renderer edge is planned. No retail bytes, private
media names, raw Direct3D, proprietary Bink dependency, or pixel claim is in
scope. Depends on 08E1A; required by E1B.
