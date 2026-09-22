# M22 plan 01 slice 06C3C2B3B2: threaded original-process fixture bootstrap

## Outcome and boundary

Resolve the unsanitized Linux physical original source-scene corruption found while extending B3B without changing product allocator behavior. The accepted B3A `dc80dea` baseline fails in its first device generation only with Vulkan validation active. `GameMemory.cpp` globally routes C++ allocations through original DMA; `ScopedCriticalSection` is a no-op until `TheDmaCriticalSection` and `TheMemoryPoolCriticalSection` are installed. Production calls `zh::original_process::initialize_services()` before graphics, but the standalone B3A fixture skipped it. Vulkan's render/validation workers then allocated concurrently through the unlocked original pools. An isolated accepted-B3A build with unchanged GameMemory and a scoped process-services bootstrap passed 30/30 validation-enabled rigid/no-readback first generations and 30/30 full four-generation static scenes, even with the old readback loop. Keep this a fixture-lifecycle correction, not an allocator policy change or readback claim.

## Investigation gate

Initialize process services before fixture asset/device allocations in every threaded original-source bgfx mode, and shut them down only after asset manager, edge and physical device destructors. Assert both DMA/pool critical sections absent before bootstrap, present during the device, absent after teardown. Keep original `GameMemory.cpp`, global new/delete, `STLSpecialAlloc`, explicit DMA and W3D class pools unchanged. Negative control: the isolated accepted-B3A no-services fixture reproduces failure under validation while the same binary with services succeeds. Do not retain the exploratory libc-global or ELF-localization policies: broad production suites proved each incompatible with existing original ownership.

## Positive, negative and recovery

After the allocation policy change, repeat at least 30 fresh-process unsanitized GCC and Clang physical static-source runs with four format/extent/device generations and validation-clean output. Also repeat original B3B mixed diagnostic at default dynamic capacity, Recording positive/failure retry, source-only leak-capable ASan/UBSan controls, and public bgfx direct negative guards. Malformed/incompatible state must still fail closed, not silently render. Zero public/source refs after every generation. B3B itself remains pending until its separate pixel/fault gates pass.

## Validation and commit

Build and run GCC/Clang Debug/Release, full canonical asset-free suites, provider-removal/ABI and dependency ledger. Evidence `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_06c3c2b3b2.md`. One independent commit after root cause and the repeated physical gate are reliable; no checkpoint commit for an unresolved guess.
