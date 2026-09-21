# M22 slice 04A — original FVF layout and mesh CPU gate

The canonical original `dx8fvf.cpp/.h` now calculates the reached Direct3D
FVF *data layout* on Linux without a D3D SDK or device. Original
`RenderInfoClass` and original `MeshClass::Render` receive an owned W3D mesh:
the authored hidden flag returns before the device boundary, while visible
state fails typed-unavailable. Owned two-UV and multistage layouts check
strides and offsets; unsupported position or nine texture sets fail closed.
This does **not** execute original sort/material/pass scheduling, produce a
recording frame, or prove retail scene rendering. Slices 04B/04C and 05–08
remain required.

The reached original string hash used an unaligned 32-bit read. Replacing it
with an exact-width `memcpy` within the canonical original specialization
preserves its bytes and hash while making its asset/texture lookup defined
on this host; the pre-fix Clang UBSan witness reported a misaligned load in
`WWLib/hashtemplate.h:426`. No proxy manager or asset authority was added.

Validation after the correction:

- GCC and Clang Debug: complete 136/136 sandbox-eligible CTests each; the
  two existing LAN UDP cases passed separately outside the socket sandbox
  for each toolchain, yielding 138/138 each.
- Focused original W3D CPU graph, ABI, compile/link/runtime source identity
  and four-provider removal: 4/4 GCC, with equivalent Clang suite within
  its full run. `dx8fvf.cpp`, `rinfo.cpp`, and `mesh.cpp` are canonical
  linked sources; removed compile/link providers fail the identity control.
- GCC and Clang Debug ASan+UBSan focused CPU graph: 1/1 each outside the
  ptrace-restricted sandbox (LeakSanitizer cannot inspect processes inside).
- Dependency ledger SHA-256 freshness and `git diff --check` pass.

The canonical M20 schema target and M22 full probe remain mutually exclusive
executables. Production still uses M20/M21 schema configuration and no
retail corpus content or original-game symlink contents were changed.
