# M22 slice 08P0B1: exact source tree model references

The CPU-only `W3DTreeModelSource` is an unpublished candidate, separate from
the accepted 08P0A type/instance registry. It resolves a model through the
exact published display/WW3D asset manager, owns the returned render ref,
and accepts a direct mesh or a source HLOD first-child mesh. HLOD child
acquisition adds its own ref; the parent HLOD ref is released before the
candidate retains the child. Candidate reset/destruction releases that mesh
ref once. Empty/non-mesh HLOD child, absent asset/provider, live-candidate
re-acquisition, malformed bounds and injected root/child reference failures
cannot publish a candidate or change accepted terrain/tree state. The
physical `W3DTreeDraw` factory remains closed; texture, atlas, Recording
resource and frame admission remain 08P0B2/B3/C.

The generated display-owner packet contains direct and HLOD meshes plus
empty and nested non-mesh HLODs. Its witness checks reference count,
numeric first-child bone offset, source vertex sphere and HLOD shadow
bounds, root/child failure unwind, provider removal and retry across two
display generations. The source packet and original input remain unchanged.

Final-source gates: six complete GCC/Clang Debug, Release and ASan+UBSan
builds and six canonical nonretail suites passed 267/267 each (`-LE
gpu|lan|retail`, sanitizer suites with `ASAN_OPTIONS=detect_leaks=0`).
Focused strict host LSan passed 3/3 on GCC and 3/3 on Clang, including the
model witness, terrain registry and generated scene boundary. Physical
Vulkan display/bootstrap/map passed 3/3 GCC; Clang display/map passed 2/2.
Serial host LAN passed 4/4 in all six configurations. The original
dependency ledger and `git diff --check` passed. Sandbox-only UDP socket
and LSan ptrace denials were superseded by passing host runs. The unrelated
renderer diagnostic remains unstaged.
