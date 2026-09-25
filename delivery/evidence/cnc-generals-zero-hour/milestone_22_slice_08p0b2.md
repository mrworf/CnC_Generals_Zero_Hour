# M22 slice 08P0B2: bounded source tree atlas

The CPU-only `W3DTreeAtlasSource` is an unpublished candidate. It reads
tree TGA input through the exact published file-system provider, checking
`Art/Terrain/` before `Art/Textures/`. Checked raw/RLE 24/32-bit decode,
TGA orientation, source half-tile and square-tile decisions produce BGRA
tile bytes. Case-insensitive duplicate names reuse the first tile block.
The source widest-first grid packs 64-pixel tree cells, flips tile-row
placement and pixel rows into the atlas, starts at 512 pixels, and enforces
512 decoded tiles and a 2048-pixel edge. Native tree source comments mention
four-pixel borders, but the executable tree packing and copy path has no
extra border gap; this candidate follows the executable path. No GPU texture
is created, and the tree factory and 08P0A registry remain unchanged.

The complete decode/pack/pixel transaction stays local until success.
Missing, malformed, unsupported and truncated files; removed providers;
injected read/tile/pack/pixel failures; and a six-class 600-tile overflow
leave an earlier candidate intact. Reset releases its CPU pixel allocation;
same-owner retry and two display generations succeed. Generated TGA fixtures
assert exact 2x2 and 1x1 source slots, case-insensitive alias, 24-bit RLE
fallback, 32-bit orientation, half-tile, BGRA pixels, 500-tile/2048-pixel
success and 600-tile rejection. Candidate pixel storage is empty before
display teardown; final device residuals are zero. The generated source
input remains read-only and unchanged.

Final-source gates: six complete GCC/Clang Debug, Release and ASan+UBSan
builds and six canonical nonretail suites passed 267/267 each (`-LE
gpu|lan|retail`, sanitizer suites with `ASAN_OPTIONS=detect_leaks=0`).
Focused strict host LSan passed 2/2 on GCC and 2/2 on Clang for the atlas
witness and terrain registry. Physical Vulkan display/bootstrap/map passed
3/3 GCC; Clang display/map passed 2/2. Serial host LAN passed 4/4 in all
six configurations. The original dependency ledger and `git diff --check`
passed. The unrelated renderer diagnostic remains unstaged.
