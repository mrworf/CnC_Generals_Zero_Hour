# Pinned public bgfx shader compiler for M29

M29 compiles the repository-owned GLSL families into the public bgfx Vulkan
shader container with a pinned upstream `shaderc` source build. The three
revisions and the SHA-256 of each upstream `LICENSE` are in
`bgfx_shaderc.lock`. `bgfx`, `bx` and `bimg` are BSD-2-Clause licensed by
Branimir Karadzic and contributors; the license text remains in each staged
source checkout. The repository's `bgfx_shaderc_glsl.patch` is a narrowly
reviewed source patch under those upstream license terms. No prebuilt
upstream binary or private game asset is committed.

One-time acquisition (requires network, outside configure/build/test):

```sh
tools/renderer/bootstrap_bgfx_shaderc.sh --acquire
```

For an offline machine with the three exact public Git revisions already
available as sibling `bgfx`, `bx`, `bimg` checkouts:

```sh
tools/renderer/bootstrap_bgfx_shaderc.sh --offline-sources /path/to/public/checkouts
```

The supplied checkout path is verified read-only. The script clones it to the
ignored, task-owned `build/bgfx-toolchain/source/` cache, checks pins/licenses,
applies the reviewed patch only there, and builds `build/bgfx-toolchain/bin/shaderc`.
Wrong revisions, different licenses, dirty/unexpected cache source, or a
missing tool fail closed. Configuration and all later shader builds use that
local binary without network access. Keep a copy of the pinned source
checkouts to reproduce the bootstrap without network. `spirv-val` and `glslc`
are ordinary host build prerequisites already used by this project; all four
x86-64 presets build the same 39 shader families.

The patch adds raw GLSL input to upstream shaderc's Vulkan path, preserves
source vertex attribute locations, maps the owned attribute aliases, and
reflects 16-byte integer vector uniform payloads. The build-only lowering
`tools/renderer/prepare_bgfx_glsl.py` maps the owned GLSL descriptor layout
into bgfx's single set-0 stage UBO and separate image/sampler bindings. The
per-shader JSON manifest and `verify_bgfx_shaders.py` check that mapping and
the bgfx/SPIR-V envelopes. The historical SDL_GPU SPIR-V outputs remain
unchanged until M30 switches physical device ownership.
