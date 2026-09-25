# M22 slice 08N0A: modeled volume-shadow publication

The CPU-only original modeled draw path now binds `DrawableInfo` before draw
module construction, scene-links and binds the render object before requesting
a replacement volume shadow, and defers the initial request until the draw
module is published in `onObjectCreated`. The existing shadow-manager guard
still rejects a foreign, detached or duplicate render owner. Constructor
failure releases its unpublished module's render/shadow/track state locally;
replacement failure releases the just-created caster and scene object, and
normal removal releases the caster before detaching the render object. A
volume model acquisition with no render object fails instead of silently
publishing an unshadowed state. Non-volume missing-model behavior is unchanged.

The project-owned generated bridge fixture now includes a second valid rigid
model state and an enabled volume shadow. Four positive mission/skirmish/retry
attempts exercise initial publication and real model replacement. The weak
W3D-owner snapshot, only present in the full W3D executable, proves exact
model, scene, `DrawableInfo` user-data and bounded-caster identity. Its
pre-teardown count shows one newly admitted caster and preservation of the
prior caster on success; 23 negative/fault attempts leave zero attempted
casters and preserve the prior identity before Recording teardown. These
include scene-add, post-scene, shadow admission, post-shadow and two
replacement boundaries. The admission-list allocation path deletes an
unpublished caster if insertion fails. The older borrowed-file missing-model,
bridge-map, shadow-source and decal controls remain independent and green.

All six GCC/Clang Debug, Release and ASan+UBSan complete builds and canonical
nonretail suites passed 266/266 each (`-LE gpu|lan|retail`; sanitizer suites
used `ASAN_OPTIONS=detect_leaks=0`). Both focused strict host LSan groups
passed 10/10, including the new constructor/replacement witness. Physical
Vulkan display/factory controls passed 2/2 with host escalation; serial host
LAN passed 4/4 in all six configurations. The original dependency ledger and
`git diff --check` passed on final source.

This slice does not claim volume geometry readiness or a rendered shadow
frame; that remains 08N0B. No retail source, selector, path, logical name,
bytes, hash or raw output is retained. The original symlink/content is
untouched, and the unrelated renderer diagnostic remains unstaged.
