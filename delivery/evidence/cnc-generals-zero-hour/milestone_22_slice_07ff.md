# M22 slice 07FF: integrated generated full-feature Recording gate

`ZH_M22_FULL_FEATURE_PROFILE` transfers the generated original decal owner
into one live generated-map display and records terrain, tracks, decal shadow,
water, original particle dispatch and bounded smudge geometry in source order.
The fixture rejects a malformed map and retains existing source-owner failure,
retry, duplicate, detached-owner, reset, two-generation and zero-resource
controls.

Final-tree acceptance: fresh 206/206 non-GPU/non-LAN suites passed in GCC and
Clang Debug/Release plus both sanitizer builds (broad sanitizer CTest used
`LSAN_OPTIONS=detect_leaks=0` only for ptrace compatibility); host GCC
ASan+UBSan full-feature probe passed with `detect_leaks=1`; host Vulkan passed;
serial LAN passed 4/4 in each of six builds; ledger and diff checks passed.
