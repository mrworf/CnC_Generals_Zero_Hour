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
} // namespace

int main()
{
    first_original_buffer_bytes();
    injected_device_failures();
}
