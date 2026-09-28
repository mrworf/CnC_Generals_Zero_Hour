// Bounded Linux source prop-frame ownership. No serialized or class-layout state.
#pragma once
#if defined(ZH_WW3D_CPU_ONLY)
#include <memory>
#include <vector>
class RenderObjClass;
class MeshClass;
namespace ww3d_prop {
class FrameGraph {
    struct State;
    std::unique_ptr<State> state;
    explicit FrameGraph(std::unique_ptr<State>);
public:
    static std::shared_ptr<FrameGraph> capture(const std::vector<RenderObjClass*>&);
    ~FrameGraph();
    FrameGraph(const FrameGraph&)=delete;
    FrameGraph& operator=(const FrameGraph&)=delete;
    void restore() noexcept;
    const std::vector<MeshClass*>& meshes() const noexcept;
};
}
#endif
