# Stock renderer assessment

## Scope

Restart from original source under the user's upstream-only dependency policy.
Source/documentation investigation was performed; no fresh stock hardware gate
has passed. bgfx is the preferred candidate, not an unconditional acceptance.

## Executive Summary

The former fork mainly served custom adapter contracts (atomic native submission,
private memory accounting and custom shader transport). Those contracts are
superseded. bgfx can plausibly serve the game using ordinary public submission,
stock shaderc and game-side resource preparation; N1 must prove the remaining
source-specific semantics before wider integration.

## What Is Good

bgfx provides ordered views, rendering targets, texture subresource views and
stencil rendering. Its examples cover reflections and shadow volumes. These
match important original rendering needs without introducing a new scene engine.
SDL remains the window/input provider. Public source references:

- https://bkaradzic.github.io/bgfx/bgfx.html
- https://bkaradzic.github.io/bgfx/tools.html
- https://bkaradzic.github.io/bgfx/examples.html
- https://bkaradzic.github.io/bgfx/overview.html

## What Is Bad Or Risky

Copies precede draws inside a view; blindly translating immediate D3D calls can
reorder effects. shaderc has explicit attribute/varying/uniform conventions; our
former raw-GLSL transport cannot be retained as a compiler requirement. Selective
stencil writes, base-only mip filtering and scene-copy distortion require real
pixel proofs. Local SDK internals are evidence, not public API contracts.

SDL_GPU remains a credible alternative: its depth/stencil pipeline exposes read
and write masks. A missing dedicated rectangle-clear operation alone does not
prove impossibility; an equivalent game-owned draw may work but needs parity
tests. Direct Vulkan would add synchronization/resource ownership to this port
and is not the selected maintenance tradeoff.

- https://wiki.libsdl.org/SDL3/SDL_GPUDepthStencilState
- https://wiki.libsdl.org/SDL3/SDL_BeginGPURenderPass
- https://docs.vulkan.org/guide/latest/synchronization.html

## What Should Change

Rebuild from upstream sources, author supported shaders, and design a focused
game renderer. Run the complete N1 high-risk suite before integrating the world.
Use sampleable scene targets, explicit pass ordering and incremental loading.
Reassess on an actual public-API gap; never expand the dependency locally.

## What I Would Not Change Yet

Do not choose a new game engine or direct Vulkan, remaster assets, or redesign
simulation to fit a renderer. Do not carry over a general transactional GPU API.

## Overall Opinion

Proceed with stock-bgfx qualification. The tests, provenance and no-patch rule
are the maintenance boundary. If N1 fails on a required effect, stop and compare
SDL_GPU against that concrete case before continuing downstream implementation.
