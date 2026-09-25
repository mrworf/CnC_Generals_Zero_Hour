# M22 slice 08L3A0: borrowed W3D file-factory owner

The CPU-only original `W3DDisplay` now constructs and owns the canonical
`W3DFileSystem` before its scene and asset-manager bootstrap. It publishes
`TheW3DFileSystem` only after all display owners are ready, and on normal or
failed teardown restores the file factory that preceded the display. The
generated original draw window borrows the display's exact factory identity;
standalone draw windows own a temporary factory and restore their prior
owner. A missing or foreign active factory rejects generated construction.

The asset-free generated source test emits one project-owned W3D model and
maps a modeled `LogicFixture` through the native `W3DModelDraw` path. It
witnesses `active=1 modeled=1` immediately before the generated boundary
reset, distinguishes a removed model as `modeled=0`, and proves a clean retry.
It also injects before factory creation and after global publication, then
checks zero post-teardown graphics owners and Recording resources. Separate
missing-factory and foreign-factory injections reject without a dangling
owner and retry cleanly. Two independent generations load the model, and
generated source input remains byte-for-byte unchanged. No retail archive or
original symlink was opened by this focused witness.

The accepted 08L2 generated-construction route remains intact. Bridge and
pathfinder attachment are still the separate 08L3A owner; the map attempt
transaction remains 08L3B.

Final-state validation:

- Six complete GCC/Clang Debug, Release and ASan+UBSan builds passed.
- All six asset-free canonical suites passed 262/262 each. Sanitizer broad
  suites use `ASAN_OPTIONS=detect_leaks=0`.
- Strict host LeakSanitizer passed the generated scene, generated
  construction and borrowed file-owner routes 3/3 in GCC and Clang.
- The unchanged physical Vulkan display-owner and factory controls passed
  2/2 with the validation layer. Serial host LAN passed 4/4 in all six
  configurations.
- The standalone full W3D draw scenario passed in GCC Debug and Clang
  Release without its optional retail archive. The source dependency ledger
  and `git diff --check` pass.
