# M22 slice 08P0C1B2A: tree crusher topple state

The full-draw terrain CPU owner now stages a mobile crusher's native
partition collision into a copied tree instance, with finite nonzero
direction, minimum angular speed, acceleration, fog pause/reveal, bounce and
DOWN. The new per-instance `Matrix3D` is explicitly identity-initialized;
the original class layout is unchanged. A visible frame applies the candidate
transform to source raised vertices and publishes tree state, geometry and
Recording buffers only after validation and upload. Missing player,
partition or shroud, invalid motion and injected state/geometry/upload
failures preserve accepted state and resources. No DOWN sink/deletion or
external FX dispatch is admitted; those owners remain 08P0C1B2B/B3.

The generated witness uses the shipping `Object::setPosition` callback for
crusher initiation. It checks paused and hidden accelerated frames, visible
reentry, exact raised vertex X/Z bytes, fog freeze/reveal, bounce reversal,
zero-bounce DOWN, ineligible immobile unit, nonfinite/zero parameter and
zero-length direction rejection, failed-frame rollback and clean retry,
removal and live-FALLING reset. A test-only Display notification sink bridges
the generated logical/visual map-size mismatch only while the real
PartitionManager performs reveal/undo and cover/undo. Exact cell/status
notification pairs are asserted; the real Display is restored before each
topple frame, and every PartitionManager shroud cell is compared to its
pre-test baseline after reconstruction. The first generation's historical
fog residue caused the second generation's topple retry to freeze; the
fixture now reconstructs that baseline without changing production reset.
Both generations retain the immediate pre-teardown no-draw and zero-Recording
residual assertions.

The complete two-generation witness takes about 57–58 seconds natively,
257.93 seconds in isolated GCC ASan+UBSan and 218.53 seconds in isolated
Clang ASan+UBSan. Its script alone now bounds each generated subprocess at
240 seconds and reports a redacted timeout failure; all prior assertions
and the 64-type/4000-instance workload remain. Temporary diagnostic markers
were removed before final-source gates.

Final-source focused GCC/Clang Debug passed 1/1 each (57.42s and 58.49s);
isolated GCC/Clang ASan+UBSan passed 1/1 each with
`ASAN_OPTIONS=detect_leaks=0` (257.93s and 218.53s). Six complete
GCC/Clang Debug, Release and ASan+UBSan builds passed. All six canonical
nonretail suites passed 267/267 each with `-LE gpu|lan|retail`; sanitizer
suites ran serially, and the expanded witness passed within them in
257.60s GCC and 215.98s Clang. Strict host LSan used exactly
`ASAN_OPTIONS=detect_leaks=1`, no UBSan override, and passed display-owner
and generated-terrain controls 2/2 on both compilers under host escalation.
Host-escalated physical Vulkan passed GCC display/bootstrap/map 3/3 and
Clang display/map 2/2 with `VK_LAYER_KHRONOS_validation` and no
`Validation Error` or `VUID-` category; host NVIDIA driver 615.71.09.
Serial host LAN passed 4/4 in all six configurations. The two owned
dependency-ledger rows pass the checker, `git diff --check` is clean, and
the unrelated renderer diagnostic remains unstaged.
