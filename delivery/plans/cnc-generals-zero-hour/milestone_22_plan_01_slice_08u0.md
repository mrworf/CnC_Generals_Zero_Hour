# M22 plan 01 slice 08U0: particle source-frame aggregate

Status: planned evidence-only aggregate after separately accepted 08U0A and
08U0B. Plan transaction parent: `821c0367b01f0425e269c09b105d96bc876a214f`.

## Finite public owner inventory and dependency graph

The correctly provisioned redacted continuation crossed accepted 08T0R3 and
stopped at `W3DDisplay`'s immutable source-frame checkpoint before Recording.
Public generated-only read-only debugger classification: 213 registered
systems, zero current particles; reached type `PARTICLE`, shader `ALPHA`, 208
Drawable-attached stopped and five Object-attached active systems. This is a
fixed category/count, not input metadata. The track, smudge, shadow, terrain,
scene and visual-map providers exist at this boundary. An empty-particle count
does not establish a no-op registry or a valid skip of source update/render.

Public source graph:

1. `W3DDisplay::draw` calls `updateViews`, then native
   `ParticleSystemManager::update`, then source frame/render. Current Linux
   display skips the CPU update and its C2D checkpoint rejects nonzero systems.
2. `ParticleSystemManager::update` stamps the logic frame before traversing
   systems; `ParticleSystem::update` may resolve attachments and shroud/terrain,
   advance wind/delay/lifetime, consume GameClient RNG, create/retire particles,
   priority-evict, spawn attached/slave systems and change linked-list ownership.
   System constructor consumes RNG/creates slave before fallible list insertion;
   `Particle` links both a manager priority list and a system list. Reset removes
   systems/particles and resets IDs/counters but retains templates. Direct
   ParticleSys source has no Audio/FXList dispatch in the reached branch; no
   general claim about unsupported branches or callbacks follows.
3. Native scene queues particles once during terrain traversal and flushes
   them post-water/static-sort before final translucent sorting/present.
   `W3DParticleSystemManager::doParticles` uses up to512 points per system,
   texture lookup, point/streak/volume/smudge/weather branches, source shader
   state and on-screen counters. Current Linux branch is empty-smudge-only and
   rejects any populated registry. The Linux snow provider is not the W3D
   renderer; a nonnormal/snow path cannot be assumed safe.

Graph: accepted T0R3 + S0 import/preload + P0/C2D/D0 frame and scene/shroud/
smudge + B0/B1 Edge/texture + M20 GameClient/manager providers →
[08U0A](milestone_22_plan_01_slice_08u0a.md) reversible bounded CPU candidate →
[08U0B](milestone_22_plan_01_slice_08u0b.md) GPU output and joint frame commit →
08U0 evidence-only aggregate → active [08](milestone_22_plan_01_slice_08.md)
→09. A does not require B; B cannot accept or publish a frame without A's
candidate. The two leaves reflect independently fallible CPU graph and GPU
resource/frame owners, not an added milestone or permission to split further.

## Aggregate acceptance and privacy

Verify A/B commits in current ancestry, their final frozen source/test and
selected-binary hashes, ledger, exact focused/strict/native/physical/common
gates and the joint retry/no-re-advance witness. If B has changed executable
bytes, rely on B's fresh full composition, not an obsolete A suite. No test or
production edit belongs in this evidence-only commit. Record the final accepted
family and explicit unsupported categories; active08 may resume only after U0
closes. The existing scene-once privacy wrapper remains bounded, original roots
read-only, and only fixed categories/counts may enter evidence. Do not modify
the original symlink/content or stage current active08/renderer work.

Commit only aggregate plan/evidence/index state after exact diff and hash audit:
`delivery: M22 08U0 close particle source frame`. One A commit and one B commit
precede it; the aggregate has no new executable gate and must not claim retail
scene or M22 acceptance from a generated particle pass.
