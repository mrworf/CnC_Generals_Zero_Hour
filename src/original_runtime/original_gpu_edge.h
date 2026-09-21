#pragma once

#include "zh/renderer/contract.h"

#include <unordered_map>

class VertexBufferClass;
class IndexBufferClass;

namespace zh::original_runtime {

// Device-only translation of the bytes owned and populated by original WW3D
// buffers. Never selects geometry, material, shader or pass order.
class OriginalGpuEdge final {
public:
    explicit OriginalGpuEdge(renderer::GpuDevice& device);
    ~OriginalGpuEdge();
    OriginalGpuEdge(const OriginalGpuEdge&) = delete;
    OriginalGpuEdge& operator=(const OriginalGpuEdge&) = delete;

    renderer::BufferHandle bind_vertex(const VertexBufferClass* source);
    renderer::BufferHandle bind_index(const IndexBufferClass* source);
    static OriginalGpuEdge& required();

private:
    renderer::GpuDevice& device_;
    OriginalGpuEdge* previous_;
    std::unordered_map<const VertexBufferClass*, renderer::BufferHandle> vertices_;
    std::unordered_map<const IndexBufferClass*, renderer::BufferHandle> indices_;
};

} // namespace zh::original_runtime
