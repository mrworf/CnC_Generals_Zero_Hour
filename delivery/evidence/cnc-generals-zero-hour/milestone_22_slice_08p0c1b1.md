# M22 slice 08P0C1B1: tree collision and push-aside

The CPU-only full-draw terrain owner now handles the production mobile-unit
movement callback through `Object::setPosition` and `LinuxGameClient`'s
explicit full-draw capability. The minimal/headless engine retains its no-op
callback and no terrain draw dependency. The collision path follows native
immobile filtering, box/cylinder radius selection, clamped 100x100 area
partition, 3D collision sphere, repeated-pusher frame window and direction
selection. It rejects missing providers, zero radius and unsupported crusher
toppling before publication. Crusher/toppling and external FX remain C1B2/C1B3.

Push state is copied into a candidate owner and published only after validation.
The visible-frame path advances outward/inward motion only when unpaused,
rebuilds raised-Z vertex displacement and the source darkening slot, and
publishes state/Recording buffers together after fallible geometry/upload
work. Rejection preserves accepted tree/partition identity, frame, source
buffers, resource counts and GameClient RNG; removal and relocation leave no
stale partition entry. No native class layout was changed.

The generated two-generation witness uses test-owned mobile and immobile
objects, a raised-vertex W3D asset, and the real `Object::setPosition`
forwarding path. It covers cylinder/box collision, edge bucket, repeated
pusher, pause, outward/inward return, relocation/removal, zero radius, null
unit, zero inward frames, injected collision publication, geometry and
Recording upload failures, and clean retry with exact vertex/darkening bytes.
Clang's initial focused abort was traced to an outdated generated asset
producer executable; rebuilding that dependency made the same witness pass.
Temporary fixed-category diagnostics were removed before final-source gates.

Final-source validation: focused generated terrain witness passed 1/1 in
GCC/Clang Debug and in both ASan+UBSan configurations (sanitizer runs isolated
with `ASAN_OPTIONS=detect_leaks=0`). Six complete GCC/Clang Debug, Release
and sanitized builds passed. Their six canonical nonretail suites passed
267/267 each (`-LE gpu|lan|retail`); sanitized suites ran serially.
Strict host LSan with exactly `ASAN_OPTIONS=detect_leaks=1` and no UBSan
override passed display-owner/terrain controls 2/2 on each compiler under
host escalation. Physical Vulkan validation passed display/bootstrap/map
3/3 on GCC and display/map 2/2 on Clang under host escalation. Serial host
LAN passed 4/4 in all six configurations. The original dependency ledger
and `git diff --check` passed. The unrelated renderer diagnostic remains
unstaged.
