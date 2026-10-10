# Architecture review — headless foundation qualification

## Scope

REC-001/REC-DEC-001, HQ-01–HQ-07 and their direct effects on R03/R05/R07/R08/R09.
Source inspection at c5e1fcb3 plus explicitly unaccepted dirty work. The new specification,
journey and nonvisual UX inputs are reviewed by exact hashes in the stage result. No
production source changed. This is an in-chat skill review, not independent-agent verification.

## Executive Summary

The contract is sufficient to define a finite original-runtime foundation demonstration.
It reaches actual engine startup/load/update and an original scripted/scheduled state change,
with independent semantic witnesses, repeatable original-state observations and full owner
retirement. No architecture conflict with the approved stock SDL/bgfx direction is established.
PASS is definition readiness within this amendment, not evidence that H1 executes today.

## What Is Good

- Good: source root/update remains the execution authority. GameEngine.cpp::update orders
  original client/audio/message work before GameLogic; GameLogic.cpp::update retains original
  scripts, terrain, sleepy modules, AI/build/partition and end-of-frame destruction. Keeping
  these paths prevents an attractive but invalid replacement simulator.
- Good: generated DeletionUpdate is a real original scheduled behavior. Its constructor uses
  original RNG/duration state and setWakeFrame; update schedules source object destruction.
  GameLogic processes the destroy list before incrementing the frame. Frame-entry/exit evidence
  makes the required oracle precise without accepting two compilers making the same mistake.
- Good: nonuniform terrain detects a false logical-provider success. The source graph already
  documents why the base zero-height terrain implementation is insufficient. Drawless actors
  avoid a misleading claim that a model needs physical rendering to exist logically.
- Good: source-call/provider manifests make required dependencies explicit. Weapon.cpp's
  projectile query demonstrates why client/model code cannot be declared irrelevant globally.
  H1 omits that behavior only in a declared fixture while preserving final combat fidelity.
- Good: real supplied-data initialization is separate from generated deterministic execution.
  The current indexing mask and link-only probe are not promoted into startup acceptance.

## What Is Bad Or Risky

- Risky: implementation can continue following the old expanding slice after this definition
  is accepted. Existing delivery plans/state still require their own controlled update; this
  review cannot silently rewrite their boundaries. Evidence: REC-RUN-001's plan growth and
  conflicting Full509/Full475 resumption references.
- Risky: a native root can appear linked while required services are absent. The explicit
  EXCLUDE_FROM_ALL root probe never calls init, and the concrete native root is pending.
  HQ-01/04 prohibit counting that as execution or using successful empty providers.
- Risky: fixture registration can accidentally stand in for production creators. Current
  ownership fixtures include test module adapters; they are useful components, not H1's
  allowed gameplay factory. H1 must register original DeletionUpdate, not a test subclass
  with synthetic update behavior.
- Risky: GPU/display-independent execution still needs logical time, client state and view
  queries. NativeHeadlessDisplay::advanceLogicalFrame guards actual source globals and
  freeze state; bypassing those guards would change semantics instead of fixing integration.
- Risky: a passing small scenario can be mistaken for all-provider or game parity. HQ-04/07
  and UX-HQ-05 explicitly forbid this inference. Broad requirements remain in the baseline.

## What Should Change

The new contract supplies the formerly missing acceptance definition. Subsequent delivery
must implement/verify its native root, H1 and private initialization path, and reconcile the
existing planning records with independently closeable outcomes. These are already required
R07 obligations, not new feature scope or an architecture rewrite.

Choose source-equivalent provider mechanisms and documented CLI/fixture encoding during
implementation. Do not require universal simulation-tick rollback where the source/product
contract only requires failure plus owner retirement and corrected startup. Preserve existing
operation-level rollback guarantees and sanitizer/failure-pair requirements.

## What I Would Not Change Yet

Do not replace bgfx, fork dependencies, rewrite accepted core/data foundations, delete CPU
model/particle/audio work, redesign original UI, reorder N0–N7, or port every physical provider
before the declared drawless qualification. No evidence here justifies those changes.
Do not adopt a complete D3D8 compatibility layer merely to simplify one native factory.

## Overall Opinion

No unresolved material architectural choice remains in this bounded contract. H1's size,
seed, fixed workload and frame-level oracle are engineering qualification choices under
existing authority. Source-defined behavior and later game scope remain binding. Missing
runtime execution is implementation evidence to obtain, not a reason to invent another
Product Owner decision. The package must prominently retain its amendment-only scope.

Validation: source/call inspection, requirement consistency, exact input hashes, internal
links and structured result checks. No build, sanitizer, GPU, retail or gameplay run occurred.
