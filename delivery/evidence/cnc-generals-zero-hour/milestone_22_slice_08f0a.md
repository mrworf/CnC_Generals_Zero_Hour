# M22 slice 08F0A evidence: update-owned two-phase lifecycle

## Source decision

The first original new-game call only records the pending start.  The second
phase is dispatched by `GameEngine::update`, which updates the client and
message stream before `GameLogic::update`.  The Linux probe now preserves that
order and avoids a duplicate base update on the start tick.

## Focused and final-tree evidence

- The generated full-draw witness proves accepted update-owned construction,
  missing-model distinction, missing-rider rejection, and a fresh second
  process generation.  The complementary M21/M24 replay fixture confirms that
  its established direct two-phase transaction is not admitted to this M22
  branch.
- After the condition was narrowed to the explicit M22 draw profile, all six
  freshly rebuilt non-GPU/non-LAN final-tree suites passed: GCC Debug
  **260/260** (210.68s), GCC Release **260/260** (121.18s), Clang Debug
  **260/260** (197.26s), Clang Release **260/260** (75.30s), GCC ASan/UBSan
  **260/260** (581.61s), and Clang ASan/UBSan **260/260** (459.90s).
- Strict `detect_leaks=1` source focus passed **9/9** in GCC (41.17s) and
  Clang (30.01s).  This includes setup/re-entry, parser aggregate, and
  full-draw identity/provider controls.
- Physical public-bgfx Vulkan validation with the Khronos layer passed
  **9/9**.  Serial local-socket LAN passed **4/4** in each of the six
  configurations.  Direct and registered dependency-ledger checks and
  `git diff --check` pass.

## Boundaries

No retail route, retail content, identifiers, paths, bytes, hashes, images, or
Recording-fidelity claim was used.  The selector transition remains pending
08F0; parser/provider and scenario construction remain 08F1 and 08F.
