#include "original_gpu_edge.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8fvf.h"
#include "zh/renderer/recording_device.h"

#include <stdexcept>
#include <cstring>

namespace {
using namespace zh::renderer;
void check(bool okay) { if (!okay) throw std::runtime_error("original first GPU edge invariant failed"); }

class FaultDevice final : public GpuDevice {
public:
    RecordingGpuDevice recorder;
    int reject_create = 0;
    int reject_upload = 0;
    BufferHandle create_buffer(const BufferDesc& d, std::string_view l) override
    { if (reject_create && --reject_create == 0) return {}; return recorder.create_buffer(d,l); }
    TextureHandle create_texture(const TextureDesc& d, std::string_view l) override { return recorder.create_texture(d,l); }
    SamplerHandle create_sampler(const SamplerDesc& d, std::string_view l) override { return recorder.create_sampler(d,l); }
    ShaderHandle create_shader(const ShaderDesc& d, std::string_view l) override { return recorder.create_shader(d,l); }
    PipelineHandle create_pipeline(const PipelineKey& d, std::string_view l) override { return recorder.create_pipeline(d,l); }
    ValidationResult upload(const UploadDesc& d, const void* b) override
    { if (reject_upload && --reject_upload == 0) return {false,"injected original upload failure"}; return recorder.upload(d,b); }
    ValidationResult upload_texture(const TextureUploadDesc& d, const void* b) override { return recorder.upload_texture(d,b); }
    ValidationResult begin_pass(const RenderPassDesc& d, std::string_view l) override { return recorder.begin_pass(d,l); }
    ValidationResult draw(const DrawDesc& d) override { return recorder.draw(d); }
    ValidationResult end_pass() override { return recorder.end_pass(); }
    ValidationResult present(TextureHandle h) override { return recorder.present(h); }
    void destroy(BufferHandle h) override { recorder.destroy(h); }
    void destroy(TextureHandle h) override { recorder.destroy(h); }
    void destroy(SamplerHandle h) override { recorder.destroy(h); }
    void destroy(ShaderHandle h) override { recorder.destroy(h); }
    void destroy(PipelineHandle h) override { recorder.destroy(h); }
    const std::string& last_error() const noexcept override { return recorder.last_error(); }
    bool pass_active() const noexcept override { return recorder.pass_active(); }
    void record_marker(std::string_view marker) override { recorder.record_marker(marker); }
};

void first_original_buffer_bytes()
{
    auto *vb = NEW_REF(DX8VertexBufferClass, (DX8_FVF_XYZ, 3));
    auto *ib = NEW_REF(DX8IndexBufferClass, (3));
    {
        VertexBufferClass::WriteLockClass lock(vb);
        auto *bytes = static_cast<unsigned char*>(lock.Get_Vertex_Array());
        for (unsigned i=0; i<3*vb->FVF_Info().Get_FVF_Size(); ++i) bytes[i] = static_cast<unsigned char>(i);
        IndexBufferClass::WriteLockClass indices(ib);
        indices.Get_Index_Array()[0]=2;
        indices.Get_Index_Array()[1]=1;
        indices.Get_Index_Array()[2]=0;
    }
    FaultDevice device;
    bool missing_session=false;
    try { (void)zh::original_runtime::OriginalGpuEdge::required(); }
    catch (const std::runtime_error&) { missing_session=true; }
    check(missing_session);
    {
        zh::original_runtime::OriginalGpuEdge edge(device);
        bool nested=false;
        try { zh::original_runtime::OriginalGpuEdge other(device); }
        catch (const std::runtime_error&) { nested=true; }
        check(nested);
        auto vertex = edge.bind_vertex(vb);
        auto index = edge.bind_index(ib);
        check(device.recorder.resource_counts().buffers == 2);
        check(edge.bind_vertex(vb) == vertex && edge.bind_index(ib) == index);
        const auto vertex_bytes = device.recorder.buffer_bytes(vertex);
        const auto index_bytes = device.recorder.buffer_bytes(index);
        check(vertex_bytes.size() == 3*vb->FVF_Info().Get_FVF_Size());
        check(std::memcmp(vertex_bytes.data(), vb->Get_CPU_Vertex_Buffer(), vertex_bytes.size()) == 0);
        check(index_bytes.size() == 3*sizeof(unsigned short));
        check(std::memcmp(index_bytes.data(), ib->Get_CPU_Index_Buffer(), index_bytes.size()) == 0);
        auto *sorting = NEW_REF(SortingVertexBufferClass, (3));
        bool unsupported=false;
        try { (void)edge.bind_vertex(sorting); } catch (const std::runtime_error&) { unsupported=true; }
        check(unsupported && device.recorder.resource_counts().buffers == 2);
        sorting->Release_Ref();
        vb->Release_Ref();
        ib->Release_Ref();
        // The scoped bridge holds the original source references until device teardown.
        check(edge.bind_vertex(vb) == vertex && edge.bind_index(ib) == index);
    }
    check(device.recorder.resource_counts().total() == 0);
    check(VertexBufferClass::Get_Total_Buffer_Count() == 0);
    check(IndexBufferClass::Get_Total_Buffer_Count() == 0);
}

void injected_device_failures()
{
    auto *vb = NEW_REF(DX8VertexBufferClass, (DX8_FVF_XYZ, 3));
    auto *ib = NEW_REF(DX8IndexBufferClass, (3));
    for (int fail=1; fail<=2; ++fail) {
        FaultDevice device;
        device.reject_create=fail;
        bool rejected=false;
        {
            zh::original_runtime::OriginalGpuEdge edge(device);
            try { edge.bind_vertex(vb); edge.bind_index(ib); }
            catch (const std::runtime_error&) { rejected=true; }
        }
        check(rejected && device.recorder.resource_counts().total() == 0);
    }
    for (int fail=1; fail<=2; ++fail) {
        FaultDevice device;
        device.reject_upload=fail;
        bool rejected=false;
        {
            zh::original_runtime::OriginalGpuEdge edge(device);
            try { edge.bind_vertex(vb); edge.bind_index(ib); }
            catch (const std::runtime_error&) { rejected=true; }
        }
        check(rejected && device.recorder.resource_counts().total() == 0);
    }
    vb->Release_Ref();
    ib->Release_Ref();
    check(VertexBufferClass::Get_Total_Buffer_Count() == 0 && IndexBufferClass::Get_Total_Buffer_Count() == 0);
}

void canonical_fvf_layouts()
{
    using zh::original_runtime::OriginalGpuEdge;
    const auto normal_uv = OriginalGpuEdge::layout_for_fvf(DX8_FVF_XYZNUV1);
    const FVFInfoClass original_normal(DX8_FVF_XYZNUV1);
    check(normal_uv.stride == original_normal.Get_FVF_Size() && normal_uv.attribute_count == 3);
    check(normal_uv.attributes[1].location == 1 && normal_uv.attributes[1].offset == original_normal.Get_Normal_Offset());
    check(normal_uv.attributes[2].location == 4 && normal_uv.attributes[2].offset == original_normal.Get_Tex_Offset(0));
    const auto diffuse_uv = OriginalGpuEdge::layout_for_fvf(DX8_FVF_XYZDUV2);
    const FVFInfoClass original_diffuse(DX8_FVF_XYZDUV2);
    check(diffuse_uv.stride == original_diffuse.Get_FVF_Size() && diffuse_uv.attribute_count == 4);
    check(diffuse_uv.attributes[1].location == 2 && diffuse_uv.attributes[1].offset == original_diffuse.Get_Diffuse_Offset());
    check(diffuse_uv.attributes[3].location == 5 && diffuse_uv.attributes[3].offset == original_diffuse.Get_Tex_Offset(1));
    const auto no_uv = OriginalGpuEdge::layout_for_fvf(DX8_FVF_XYZN);
    check(no_uv.attribute_count == 2 && no_uv.stride == FVFInfoClass(DX8_FVF_XYZN).Get_FVF_Size());
    // The authored FVFInfoClass reports overlapping offsets for the mixed
    // tangent coordinate family; never reinterpret it as a safe packed layout.
    for (unsigned invalid : {0x004U, 0x1002U, 0x902U, 0x10002U,
             static_cast<unsigned>(DX8_FVF_XYZNDUV1TG3)}) {
        bool rejected = false;
        try { (void)OriginalGpuEdge::layout_for_fvf(invalid); }
        catch (const std::runtime_error&) { rejected = true; }
        check(rejected);
    }
    FaultDevice device;
    auto vertex = device.create_shader({ShaderStage::vertex,"layout-vertex"},"layout vertex");
    auto fragment = device.create_shader({ShaderStage::fragment,"layout-fragment"},"layout fragment");
    PipelineDesc pipeline; pipeline.vertex_shader=vertex; pipeline.fragment_shader=fragment;
    pipeline.vertex_layout=VertexLayout::original_fvf; pipeline.original_fvf=normal_uv;
    auto first=device.create_pipeline(PipelineKey(pipeline),"canonical original FVF");
    check(first && device.create_pipeline(PipelineKey(pipeline),"repeat") == first);
    auto different=pipeline; different.original_fvf=diffuse_uv;
    auto second=device.create_pipeline(PipelineKey(different),"distinct original FVF");
    check(second && second != first);
    different.original_fvf.attributes[1].offset=0;
    check(!device.create_pipeline(PipelineKey(different),"overlap"));
    different=pipeline; different.original_fvf.stride=8;
    check(!device.create_pipeline(PipelineKey(different),"short stride"));
    different=pipeline; different.original_fvf.attributes[1].location=0;
    check(!device.create_pipeline(PipelineKey(different),"duplicate semantic"));
    different=pipeline; different.original_fvf.attributes[2].format=static_cast<VertexElementFormat>(255);
    check(!device.create_pipeline(PipelineKey(different),"unsupported element"));
    different=pipeline; different.original_fvf.attribute_count=17;
    check(!device.create_pipeline(PipelineKey(different),"attribute limit"));
    const auto vb=device.create_buffer({normal_uv.stride*3U,BufferUsage::vertex,true},"FVF source bytes");
    const auto ib=device.create_buffer({6,BufferUsage::index,true},"FVF source indices");
    const unsigned short indices[3]={0,1,2};
    check(static_cast<bool>(device.upload({ib,6,0,6},indices)));
    TextureDesc color_desc; color_desc.width=4; color_desc.height=4; color_desc.render_target=true;
    auto color=device.create_texture(color_desc,"FVF target");
    auto depth_desc=color_desc; depth_desc.format=TextureFormat::depth24_stencil8; depth_desc.sampled=false;
    auto depth=device.create_texture(depth_desc,"FVF depth");
    RenderPassDesc pass; pass.color_targets[0]=color; pass.color_target_count=1;
    pass.depth_target=depth; pass.width=4; pass.height=4;
    check(static_cast<bool>(device.begin_pass(pass,"original FVF indexed")));
    DrawDesc draw; draw.pipeline=first; draw.vertex_buffer=vb; draw.index_buffer=ib;
    draw.index_element_size=IndexElementSize::uint16; draw.vertex_or_index_count=3;
    check(static_cast<bool>(device.draw(draw)));
    draw.base_vertex=1;
    check(!device.draw(draw) && device.last_error().find("vertex") != std::string::npos);
    draw.base_vertex=-1;
    check(!device.draw(draw) && device.last_error().find("vertex") != std::string::npos);
    draw.base_vertex=0;
    check(static_cast<bool>(device.end_pass()));
    device.destroy(color); device.destroy(depth); device.destroy(vb); device.destroy(ib);
    device.destroy(first); device.destroy(second); device.destroy(vertex); device.destroy(fragment);
    check(device.recorder.resource_counts().total() == 0);
}
} // namespace

int main()
{
    first_original_buffer_bytes();
    injected_device_failures();
    canonical_fvf_layouts();
}
