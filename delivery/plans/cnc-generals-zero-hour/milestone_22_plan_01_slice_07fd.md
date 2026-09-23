# M22 plan 01 slice 07FD: active original shadow route

## Outcome and dependency

Requires 07F9 and the accepted original scene mesh route. Implement each
source shadow type reached by the selected scene-family contract, beginning
from `W3DShadowManager::addShadow` and the original drawable/scene ownership
path through its map-aware recording pass. The disabled-shadow owner is not
an accepting substitute once a source drawable requests a shadow.

## Boundaries and acceptance

Use owned source objects/map state to cover the enabled manager selection,
per-object add/remove, source pass ordering, reset and rollback. Unsupported
shadow types or state must fail before a successful frame; missing manager,
stale terrain and injected resource/pass failure must recover without leaked
shadow owners. Keep physical pixels, retail input and unrelated effects out
of this child.
