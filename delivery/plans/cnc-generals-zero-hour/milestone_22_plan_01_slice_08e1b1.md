# M22 08E1B1: generated mapped-image provider lifecycle

Provide the smallest source-compatible generated `INI::MappedImage` →
`ImageCollection` provider path used by the later SinglePlayer load layout.
Generated descriptors prove named case-insensitive lookup, dimensions and
descriptor identity; duplicate declarations deliberately retain the original
owner and reparse its fields.  The original provider is descriptor-only:
unknown texture formats and zero extents retain no texture payload and remain
for the later consumer to validate, rather than inventing a decoder here.

Missing input, malformed fields, absent-provider rejection (the original
parser returns before consuming the block, so a supplied definition fails
closed), partial malformed owner cleanup/retry, two generations, provider
removal, and zero ownership are required.  “Foreign/stale owner” is not
representable at this global
collection boundary: the source has one published collection pointer; a
non-null replacement is consumed as the active owner and real ownership
typing belongs to the later load-screen owner slice.  No layout, mouse,
video, audio, retail asset, draw or pixel claim.  Depends on 08E1A; required
by 08E1B.
