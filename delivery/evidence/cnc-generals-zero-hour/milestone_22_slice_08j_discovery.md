# M22 slice 08J prerequisite discovery

Parent state `f1b101c` accepts the generated multi-tile terrain aggregate.
The subsequent redacted continuation admitted the explicit scene transaction,
retained the accepted configuration/setup state and clean teardown, then
stopped before game logic at the fixed category
`pre-map active-water update lifecycle unavailable`. No sanitizer category
was reported. No private path, identifier, filename, byte, hash, image,
command stream, or raw output is retained.

The adjacent call graph shows one coherent missing source owner. The accepted
reset deliberately detaches a resource-ready active water object. The native
update-owned phase reaches terrain-visual update before map load; the current
Linux branch permits that exact state only through a generated selector. The
later visual-map transaction likewise reattaches the same owner only through
the generated construction selector, then relies on the already accepted
disabled-grid and map-override owners. Failure rollback already tracks whether
the transaction added the water scene link.

08J therefore replaces both selector-specific decisions with one exact
detached-pre-map lifecycle contract and validates update, ordered reattachment,
rollback and retry together. The slice-08 trial selector, owner-guard edit,
test registration and temporary classifier were removed before this plan-only
checkpoint. Retail scene admission remains pending in slice 08.
