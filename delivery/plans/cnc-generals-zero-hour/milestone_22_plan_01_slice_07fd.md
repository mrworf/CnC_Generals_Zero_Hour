# M22 plan 01 slice 07FD: active original shadow route

## Outcome and dependency

Requires 07FD0 and the accepted original scene mesh route. Implement the
smallest source-requested shadow type: bounded `SHADOW_DECAL` requests, which
`W3DDebrisDraw` and `W3DModelDraw` submit through
`W3DShadowManager::addShadow`. `W3DDefaultDraw` is the contrasting source
volume caller, not a decal caller. Preserve volume and projection-derived
routes as typed pending. The disabled-shadow owner is not an accepting
substitute once a source drawable requests the admitted decal.

## Boundaries and acceptance

Use owned source objects/map state to cover the enabled manager selection,
per-object add/remove, source pass ordering, reset and rollback. Unsupported
shadow types or state must fail before a successful frame; missing manager,
stale terrain and injected resource/pass failure must recover without leaked
shadow owners. Keep physical pixels, retail input and unrelated effects out
of this child.
