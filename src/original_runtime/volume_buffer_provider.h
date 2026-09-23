#pragma once

class VertexBufferClass;
class IndexBufferClass;

// Read-only ownership query implemented beside the source buffer manager.
// It avoids importing legacy min/max macro headers into the public GPU edge.
bool zh_w3d_volume_provider_owns(const VertexBufferClass *vertex, const IndexBufferClass *index);
