# Milestone prerequisite manifest

Product: cnc-generals-zero-hour. Packet: delivery/milestones/cnc-generals-zero-hour/linux-upstream-v2/status.yaml.
Source transaction: d0483ca911b029460fb46d878fe260e90ec8d071.
Audited packet: linux-upstream-v2 initial compilation; revision recorded in handoff.
Planning transaction: workflow/delivery-planning/handoff.yaml.
Audit: 2026-10-07. Intended order N0→N1→N2→N3→N4→N5→N6→N7.

| ID | Description | Consumers | Classification | Status/provider | Class | Verification/commands executed | Files changed | Blocking | Evidence |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| PRE-01 | Approved baseline, recovery and local toolchain | N0,N1,N2 | Already satisfied | Verified/repository and host | AUTO | git bundle verify; original-subtree diff; gcc/clang/cmake/ninja/python version checks | N0 artifacts | Yes | FACT: recovery evidence; GCC16.2.1, Clang22.1.8, CMake4.4.3, Ninja1.13.2, Python3.14.7 |
| PRE-02 | Official upstream acquisition | N1 | Already satisfied | Network access verified; pins/build owned by N1 | AUTO | git ls-remote official bgfx HEAD succeeded outside sandbox | None | Yes | FACT: official HEAD cca91681c953d2de9531197b0f580c866ffaa775 observed; this alone is not a build |
| PRE-03 | Vulkan hardware and validation | N1,N3,N4 | Already satisfied | Verified/host | AUTO | vulkaninfo --summary outside sandbox | None | Yes | FACT: RTX4070, NVIDIA615.71.09, Vulkan1.4.351 device, Khronos layer1.4.357 |
| PRE-04 | User-supplied retail data and representative scenarios | N2,N3,N4,N5 | Requires user action | Existing link preserved; completeness verified by N2 diagnostics, owner user | USER_ACTION | Link identity checked without reading target; content validation not run | None | At retail boundary only | FACT: link preserved; INFERENCE: usable licensed data based on user statements, verify in N2 |
| PRE-05 | Development packages/public headers | N1,N2 | Already satisfied | Verified/host; clean-machine instructions owned by consumers | AUTO | pkg-config versions for SDL3, FreeType, Fontconfig,zlib,FFmpeg,X11,GL,Wayland,Vulkan | None | Yes | FACT: all queries succeeded; no OS installs performed |
| PRE-06 | Accepted renderer and actual original process | N3,N4 | Provided by an earlier milestone | Pending/N1,N2 | AUTO | Inspect accepted statuses and evidence before entry; not run yet | None | Yes | FACT: explicit graph providers |
| PRE-07 | Integrated world, UI and media | N5 | Provided by an earlier milestone | Pending/N3,N4 | AUTO | Inspect accepted statuses and source-runtime evidence; not run yet | None | Yes | FACT: explicit graph providers |
| PRE-08 | Playable deterministic process and local UDP facility | N6 | Provided by an earlier milestone | N5 provides process; N6 owns nonprivileged UDP probe | AUTO | Source/runtime acceptance then local socket probe; not run | None | Yes | INFERENCE: local host sockets usable outside sandbox; N6 verifies before integration |
| PRE-09 | Clean distribution build environments | N7 | Requires user action | External host/container availability verified at N7; owner user/maintainer | USER_ACTION | command -v docker succeeded; daemon/images not assumed available | None | At N7 only | FACT: CLI present; INFERENCE: clean Ubuntu/Fedora environments can be acquired, verify then |

N1 owns stock source locking/bootstrap and executable GPU fixtures; N2 owns original
source build/bootstrap and asset validators. These are deliverables, not hidden
preconditions requiring already-written code. N7 owns packaging/upgrade tooling.

The sandbox cannot resolve GitHub or load the host NVIDIA ICD. The identical
read-only checks succeeded outside it through the execution approval mechanism.
Do not misclassify sandbox driver visibility as a missing host driver.

Delivery evidence update (N2 slice02): the read-only data probe reached
stage4/mask63 and complete before/after SHA256/metadata snapshots were unchanged.
`evidence/qa/N2-original-data.md` establishes archive admission, basic startup
content families and populated actual text ownership. PRE04's actual startup
and representative scenario completeness remains pending at slice03/later
consumers; this partial evidence does not remove those prerequisite gates.

No new M0 is required: the user explicitly specified N0's recoverable-reset
outcome. Later milestone outputs have named providers. Ordinary consumers share
the documented build conventions but headless N2 does not require N1 acceptance.
