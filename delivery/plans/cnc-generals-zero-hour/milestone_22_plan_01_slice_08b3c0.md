# M22 plan 01 slice 08B3C0: original XYZ volume public-edge dispatch

Requires 08B3A and 08B3B. Add the narrow `OriginalGpuEdge` bridge that binds
accepted source XYZ vertex/index buffers to the accepted public
increment/decrement/composite protocol while a caller-owned original frame is
active. It owns only public shaders, pipelines, and transient bindings; it
does not admit a `Shadow`/`W3DVolumetricShadow` owner, source scheduling,
retail input, resize/presentation route, raw Direct3D/private Vulkan API, or
pixel assertion.

Generated Recording and physical Vulkan witnesses must prove the exact
three-pass descriptor and draw order; reject inactive/missing/stale or foreign
source handles, an unsupported source layout, non-D24S8/capability targets,
and injected shader/pipeline/buffer-upload/draw failures. All failures roll
back to a retryable state. Two generation recreation, source-provider removal,
and zero public resources are required before 08B3C may introduce the bounded
original `SHADOW_VOLUME` owner.
