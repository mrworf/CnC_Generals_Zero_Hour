# N3/N4 host readiness — physical output unavailable

Date: 2026-10-10 (local). Active packet: `linux-upstream-v2`. N0, N1 and N2
are completed; N3 and N4 are dependency-ready, but both require PRE-03 current
normal-host Vulkan/session evidence before physical delivery and acceptance.

`python3 tools/upstream_bgfx.py verify` passed against pristine official
locked revisions: bgfx `cca91681c953d2de9531197b0f580c866ffaa775`, bx
`d86e4ea9d9da6e832a3ff41398587d82b772c69b`, bimg
`101b5b5fd4670f82cfdec8e98aa1ab9ee93bb2a1`. This preserves the N1
dependency source boundary; it is not a new physical renderer test.

In the normal host execution context, `vulkaninfo --summary` exited 1 and
failed to enumerate any valid GPU. A display-session variable was present,
one DRM render node and NVIDIA device nodes were visible, and the installed
ICD was NVIDIA. The kernel NVIDIA module reports `615.71.09`; installed NVIDIA
user libraries report `615.78.08`. `nvidia-smi` exited 18 with a driver/library
version mismatch. The mismatch is a host state outside this repository; no
vendor code or system driver was changed.

For N4, `pkg-config` found SDL3, FreeType, Fontconfig and FFmpeg codec/format/
resample/util development packages. An ordinary display variable, audio device
nodes and a responding PulseAudio-compatible service were present. These
discovery checks do not establish actual window, input, playback, decoder or
shutdown acceptance. N4 shares the unsatisfied PRE-03 GPU gate; PRE-12's
actual interaction/media evidence remains to be run after readiness.

Unblock PRE-03 by restoring a matching host NVIDIA kernel/user driver and a
supported display session, then demonstrate `vulkaninfo --summary` exiting 0
with an actual physical GPU. Recheck the locked dependency identity and N1
suitability if the driver/configuration changed, then resume N3 readiness and
create its separate governing and slice plans. N4 can resume its own readiness
and planning independently once the same GPU gate is satisfied. Supplied-data
scenario/media acceptance and all N3/N4 physical tests remain pending; no
generated triangle, headless H1 result or CLI presence substitutes for them.
