# M22 slice 08P0B3A: exact GameClient RNG preview

The existing six-word GameClient random stream now has a bounded
copy/preview/commit interface. Integer and real preview calls run the same
source `randomValue` algorithm against caller-owned words; they do not touch
the published client seed. Successful work can commit those six words with
a no-throw assignment, while failed work discards the local copy. The
owner excludes intervening client draws between copy and commit. The
GameLogic and audio streams, and `InitRandom` seeding, are unchanged. This
is the prerequisite for 08P0B3B's fallible geometry/Recording upload path;
no tree type, physical factory, scene, or GPU resource is published here.

The original-data codec witness seeds an equivalent fresh generation and
checks native tree random-scale/then-sway draw order, byte-identical
published state during preview, commit continuation, discarded-preview
retry matching the fresh generation, and unchanged GameLogic/audio results.
It also characterizes source equal-bound behavior: the integer call consumes
one word while the real call consumes none. GCC/Clang debug and sanitizer
focused codec/identity tests pass 2/2 each; strict host LSan passes 2/2 on
both sanitizer toolchains.

Final-source gates: six complete GCC/Clang Debug, Release and ASan+UBSan
builds and six canonical nonretail suites passed 267/267 each (`-LE
gpu|lan|retail`, sanitizer suites with `ASAN_OPTIONS=detect_leaks=0`).
Physical Vulkan display/bootstrap/map passed 3/3 GCC; Clang display/map
passed 2/2. Serial host LAN passed 4/4 in all six configurations. Two
initial LAN runs collided on the harness's fixed UDP port because they
were launched across configurations concurrently; globally serial reruns
passed and are the recorded gate. The original dependency ledger and
`git diff --check` passed. The unrelated renderer diagnostic remains
unstaged.
