#include "original_gpu_edge.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8fvf.h"
#include "dx8wrapper.h"
#include "dx8polygonrenderer.h"
#include "shader.h"
#include "meshmdl.h"
#include "zh/renderer/recording_device.h"

#include <stdexcept>
#include <cstring>
#include <vector>

namespace {
using namespace zh::renderer;
#define check(okay) do { if (!(okay)) throw std::runtime_error( \
    "original first GPU edge invariant failed at line " + std::to_string(__LINE__)); } while (false)

class FaultDevice final : public GpuDevice {
public:
    RecordingGpuDevice recorder;
    int reject_create = 0;
    int reject_upload = 0;
    int reject_draw = 0;
    std::vector<BufferHandle> created_buffers;
    BufferHandle create_buffer(const BufferDesc& d, std::string_view l) override
    { if (reject_create && --reject_create == 0) return {}; auto h=recorder.create_buffer(d,l);
      if (h) created_buffers.push_back(h); return h; }
    TextureHandle create_texture(const TextureDesc& d, std::string_view l) override { return recorder.create_texture(d,l); }
    SamplerHandle create_sampler(const SamplerDesc& d, std::string_view l) override { return recorder.create_sampler(d,l); }
    ShaderHandle create_shader(const ShaderDesc& d, std::string_view l) override { return recorder.create_shader(d,l); }
    PipelineHandle create_pipeline(const PipelineKey& d, std::string_view l) override { return recorder.create_pipeline(d,l); }
    ValidationResult upload(const UploadDesc& d, const void* b) override
    { if (reject_upload && --reject_upload == 0) return {false,"injected original upload failure"}; return recorder.upload(d,b); }
    ValidationResult upload_texture(const TextureUploadDesc& d, const void* b) override { return recorder.upload_texture(d,b); }
    ValidationResult begin_pass(const RenderPassDesc& d, std::string_view l) override { return recorder.begin_pass(d,l); }
    ValidationResult draw(const DrawDesc& d) override
    { if (reject_draw && --reject_draw == 0) return {false,"injected original draw failure"}; return recorder.draw(d); }
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

void original_wrapper_indexed_methods()
{
    FaultDevice device;
    auto* vb=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,6));
    auto* ib=NEW_REF(DX8IndexBufferClass,(6));
    {
        VertexBufferClass::WriteLockClass vertices(vb);
        std::memset(vertices.Get_Vertex_Array(),0,vb->FVF_Info().Get_FVF_Size()*6);
        IndexBufferClass::WriteLockClass indices(ib);
        for (unsigned i=0;i<6;++i) indices.Get_Index_Array()[i]=i%3;
    }
    TextureDesc color_desc; color_desc.width=8; color_desc.height=8;
    color_desc.render_target=true; color_desc.sampled=false;
    const auto color=device.create_texture(color_desc,"original draw source-owned fixture target");
    auto depth_desc=color_desc; depth_desc.format=TextureFormat::depth24_stencil8;
    const auto depth=device.create_texture(depth_desc,"original draw source-owned fixture depth");
    RenderPassDesc pass; pass.color_targets[0]=color; pass.color_target_count=1;
    pass.depth_target=depth; pass.width=8; pass.height=8;
    {
        zh::original_runtime::OriginalGpuEdge edge(device);
        ShaderClass unlit;
        unlit.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
        DX8Wrapper::Set_Shader(unlit);
        DX8Wrapper::Set_Material(nullptr);
        DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
        DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
        DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
        DX8Wrapper::Set_Vertex_Buffer(vb);
        DX8Wrapper::Set_Index_Buffer(ib,0);
        check(vb->Engine_Refs()==1 && ib->Engine_Refs()==1);
        MeshModelClass model;
        DX8PolygonRendererClass polygon(3,&model,nullptr,0,3,false,0);
        polygon.Set_Vertex_Index_Range(0,3);
        bool missing_pass=false;
        try { polygon.Render(1); }
        catch (const std::runtime_error& error)
        { missing_pass=std::string(error.what()).find("active render pass")!=std::string::npos; }
        check(missing_pass && device.recorder.snapshot().find("draw pipeline=")==std::string::npos);
        check(static_cast<bool>(device.begin_pass(pass,"original caller-owned indexed pass")));
        polygon.Render(1);
        const auto triangle_record=device.recorder.snapshot();
        check(triangle_record.find("DX8Wrapper::Set_Index_Buffer_Index_Offset")!=std::string::npos &&
            triangle_record.find("index_bits=16 first_index=3 base_vertex=1")!=std::string::npos);
        DX8Wrapper::Set_Draw_Polygon_Low_Bound_Limit(1);
        polygon.Render(1);
        check(device.recorder.snapshot()==triangle_record);
        DX8Wrapper::Set_Draw_Polygon_Low_Bound_Limit(0);
        DX8Wrapper::_Enable_Triangle_Draw(false);
        polygon.Render(1);
        check(device.recorder.snapshot()==triangle_record &&
            !DX8Wrapper::_Is_Triangle_Draw_Enabled());
        DX8Wrapper::_Enable_Triangle_Draw(true);
        DX8Wrapper::Set_Index_Buffer_Index_Offset(0);
        DX8Wrapper::Draw_Strip(0,1,0,3);
        check(device.recorder.snapshot().find("topology="+std::to_string(static_cast<unsigned>(
            PrimitiveTopology::triangle_strip)))!=std::string::npos &&
            device.recorder.snapshot().find("count=3")!=std::string::npos);
        DX8Wrapper::Set_Index_Buffer_Index_Offset(4);
        bool invalid_base=false;
        try { DX8Wrapper::Draw_Triangles(0,1,0,3); }
        catch (const std::runtime_error& error)
        { invalid_base=std::string(error.what()).find("source range")!=std::string::npos; }
        check(invalid_base);
        DX8Wrapper::Set_Index_Buffer_Index_Offset(0);
        bool invalid_index=false;
        try { DX8Wrapper::Draw_Triangles(5,1,0,3); }
        catch (const std::runtime_error& error)
        { invalid_index=std::string(error.what()).find("source range")!=std::string::npos; }
        check(invalid_index);
        device.reject_draw=1;
        bool draw_failure=false;
        try { DX8Wrapper::Draw_Triangles(0,1,0,3); }
        catch (const std::runtime_error& error)
        { draw_failure=std::string(error.what()).find("injected original draw failure")!=std::string::npos; }
        check(draw_failure);
        DX8Wrapper::Draw_Triangles(0,1,0,3);
        DX8Wrapper::Set_Index_Buffer(nullptr,0);
        {
            IndexBufferClass::WriteLockClass indices(ib);
            indices.Get_Index_Array()[2]=5;
        }
        DX8Wrapper::Set_Index_Buffer(ib,0);
        bool invalid_source_index=false;
        try { DX8Wrapper::Draw_Triangles(0,1,0,3); }
        catch (const std::runtime_error& error)
        { invalid_source_index=std::string(error.what()).find("declared vertex range")!=std::string::npos; }
        check(invalid_source_index);
        DX8Wrapper::Set_Index_Buffer(nullptr,0);
        {
            IndexBufferClass::WriteLockClass indices(ib);
            indices.Get_Index_Array()[2]=2;
        }
        DX8Wrapper::Set_Index_Buffer(ib,0);
        DX8Wrapper::Draw_Triangles(0,1,0,3);
        check(static_cast<bool>(device.end_pass()));
        DX8Wrapper::Set_Vertex_Buffer(nullptr);
        DX8Wrapper::Set_Index_Buffer(nullptr,0);
        check(vb->Engine_Refs()==0 && ib->Engine_Refs()==0);
    }
    vb->Release_Ref(); ib->Release_Ref();
    device.destroy(color); device.destroy(depth);
    check(device.recorder.resource_counts().total()==0 &&
        VertexBufferClass::Get_Total_Buffer_Count()==0 &&
        IndexBufferClass::Get_Total_Buffer_Count()==0);
}

void original_dynamic_access_owner()
{
    check(VertexBufferClass::Get_Total_Buffer_Count()==0 &&
        IndexBufferClass::Get_Total_Buffer_Count()==0);
    DynamicVBAccessClass::_Deinit(); DynamicIBAccessClass::_Deinit();
    DX8Wrapper::Reset_Source_State();
    FaultDevice device;
    TextureDesc target; target.width=8; target.height=8; target.render_target=true;
    const auto color=device.create_texture(target,"original dynamic upload color");
    target.format=TextureFormat::depth24_stencil8; target.sampled=false;
    const auto depth=device.create_texture(target,"original dynamic upload depth");
    RenderPassDesc pass; pass.color_targets[0]=color; pass.color_target_count=1;
    pass.depth_target=depth; pass.width=8; pass.height=8;
    {
        zh::original_runtime::OriginalGpuEdge edge(device);
        auto original_access=[&](unsigned start,bool fail_upload=false) {
            DynamicVBAccessClass vb(BUFFER_TYPE_DYNAMIC_DX8,dynamic_fvf_type,3);
            DynamicIBAccessClass ib(BUFFER_TYPE_DYNAMIC_DX8,3);
            bool duplicate_vertex=false, duplicate_index=false;
            try { DynamicVBAccessClass second(BUFFER_TYPE_DYNAMIC_DX8,dynamic_fvf_type,3); }
            catch (const std::runtime_error&) { duplicate_vertex=true; }
            try { DynamicIBAccessClass second(BUFFER_TYPE_DYNAMIC_DX8,3); }
            catch (const std::runtime_error&) { duplicate_index=true; }
            check(duplicate_vertex && duplicate_index);
            bool live_vertex_deinit=false, live_index_deinit=false;
            try { DynamicVBAccessClass::_Deinit(); }
            catch (const std::runtime_error&) { live_vertex_deinit=true; }
            try { DynamicIBAccessClass::_Deinit(); }
            catch (const std::runtime_error&) { live_index_deinit=true; }
            check(live_vertex_deinit && live_index_deinit);
            {
                DynamicVBAccessClass::WriteLockClass lock(&vb);
                auto* vertices=lock.Get_Formatted_Vertex_Array();
                for (unsigned i=0;i<3;++i) {
                    vertices[i]={}; vertices[i].x=static_cast<float>(start+i);
                    vertices[i].nx=1.0f; vertices[i].u1=static_cast<float>(i)/2.0f;
                }
                DynamicIBAccessClass::WriteLockClass indices(&ib);
                indices.Get_Index_Array()[0]=0;
                indices.Get_Index_Array()[1]=1;
                indices.Get_Index_Array()[2]=2;
            }
            DX8Wrapper::Set_Vertex_Buffer(vb);
            DX8Wrapper::Set_Index_Buffer(ib,0);
            check(device.recorder.snapshot().find("Set_Vertex_Buffer dynamic offset="+
                std::to_string(start))!=std::string::npos &&
                device.recorder.snapshot().find("Set_Index_Buffer dynamic offset="+
                std::to_string(start))!=std::string::npos);
            check(static_cast<bool>(device.begin_pass(pass,"original dynamic source upload")));
            bool vertex_range=false,index_range=false,base_range=false;
            try { DX8Wrapper::Draw_Triangles(0,1,0,4); }
            catch (const std::runtime_error& error) {
                vertex_range=std::string(error.what()).find("dynamic triangle range")!=std::string::npos;
            }
            try { DX8Wrapper::Draw_Triangles(1,1,0,3); }
            catch (const std::runtime_error& error) {
                index_range=std::string(error.what()).find("dynamic triangle range")!=std::string::npos;
            }
            DX8Wrapper::Set_Index_Buffer_Index_Offset(1);
            try { DX8Wrapper::Draw_Triangles(0,1,0,3); }
            catch (const std::runtime_error& error) {
                base_range=std::string(error.what()).find("dynamic triangle range")!=std::string::npos;
            }
            DX8Wrapper::Set_Index_Buffer_Index_Offset(0);
            check(vertex_range && index_range && base_range &&
                device.created_buffers.empty() == (start==0));
            if (fail_upload) {
                device.reject_upload=1;
                bool failed=false;
                try { DX8Wrapper::Draw_Triangles(0,1,0,3); }
                catch (const std::runtime_error& error) {
                    failed=std::string(error.what()).find("original vertex upload failed")!=std::string::npos;
                }
                check(failed && device.recorder.snapshot().find("draw pipeline=")==std::string::npos);
            }
            bool physical_unavailable=false;
            try { DX8Wrapper::Draw_Triangles(0,1,0,3); }
            catch (const std::runtime_error& error) {
                const std::string reason(error.what());
                physical_unavailable=reason.find("original material")!=std::string::npos ||
                    reason.find("pending source application")!=std::string::npos;
            }
            check(physical_unavailable && device.created_buffers.size()==2);
            const auto vertex_bytes=device.recorder.buffer_bytes(device.created_buffers[0]);
            const auto index_bytes=device.recorder.buffer_bytes(device.created_buffers[1]);
            check(vertex_bytes.size()==5000U*vb.FVF_Info().Get_FVF_Size() &&
                index_bytes.size()==5000U*sizeof(unsigned short));
            auto* selected=reinterpret_cast<const VertexFormatXYZNDUV2*>(vertex_bytes.data());
            auto* selected_index=reinterpret_cast<const unsigned short*>(index_bytes.data());
            check(selected[start].x==static_cast<float>(start) &&
                selected[start+2].x==static_cast<float>(start+2) &&
                selected_index[start]==0 && selected_index[start+2]==2);
            check(static_cast<bool>(device.end_pass()));
        };
        original_access(0);
        original_access(3,true);
        const auto before_wrap=device.recorder.snapshot().size();
        {
            DynamicVBAccessClass wrapped(BUFFER_TYPE_DYNAMIC_DX8,dynamic_fvf_type,4995);
            DynamicIBAccessClass wrapped_indices(BUFFER_TYPE_DYNAMIC_DX8,4995);
            DX8Wrapper::Set_Vertex_Buffer(wrapped);
            DX8Wrapper::Set_Index_Buffer(wrapped_indices,0);
            const auto wrapped_state=device.recorder.snapshot().substr(before_wrap);
            check(wrapped_state.find("Set_Vertex_Buffer dynamic offset=0")!=
                std::string::npos && wrapped_state.find(
                    "Set_Index_Buffer dynamic offset=0")!=std::string::npos);
        }
        bool sorting_draw_rejected=false;
        {
            DynamicVBAccessClass sorting(BUFFER_TYPE_DYNAMIC_SORTING,dynamic_fvf_type,3);
            DynamicIBAccessClass sorting_indices(BUFFER_TYPE_DYNAMIC_SORTING,3);
            {
                DynamicVBAccessClass::WriteLockClass lock(&sorting);
                lock.Get_Formatted_Vertex_Array()[0].x=42;
                DynamicIBAccessClass::WriteLockClass indices(&sorting_indices);
                indices.Get_Index_Array()[0]=1;
            }
            try { DX8Wrapper::Set_Vertex_Buffer(sorting); }
            catch (const std::runtime_error& error) {
                sorting_draw_rejected=std::string(error.what()).find("sorting renderer")!=std::string::npos;
            }
        }
        check(sorting_draw_rejected);
        DX8Wrapper::Set_Vertex_Buffer(nullptr);
        DX8Wrapper::Set_Index_Buffer(nullptr,0);
        check(device.recorder.snapshot().find("draw pipeline=")==std::string::npos);
    }
    DynamicVBAccessClass::_Deinit(); DynamicIBAccessClass::_Deinit();
    {
        DynamicVBAccessClass grown(BUFFER_TYPE_DYNAMIC_DX8,dynamic_fvf_type,5001);
        DynamicIBAccessClass grown_indices(BUFFER_TYPE_DYNAMIC_DX8,5001);
        check(DynamicVBAccessClass::Get_Default_Vertex_Count()==5001 &&
            DynamicIBAccessClass::Get_Default_Index_Count()==5001);
    }
    DynamicVBAccessClass::_Reset(true); DynamicIBAccessClass::_Reset(true);
    DynamicVBAccessClass::_Deinit(); DynamicIBAccessClass::_Deinit();
    DX8Wrapper::Reset_Source_State();
    device.destroy(color); device.destroy(depth);
    check(device.recorder.resource_counts().total()==0 &&
        VertexBufferClass::Get_Total_Buffer_Count()==0 &&
        IndexBufferClass::Get_Total_Buffer_Count()==0);
}

void source_camera_clear_edge()
{
    RecordingGpuDevice device;
    TextureDesc texture;
    texture.width=48; texture.height=32; texture.render_target=true; texture.sampled=false;
    const auto color=device.create_texture(texture,"source color");
    texture.format=TextureFormat::depth24_stencil8;
    const auto depth=device.create_texture(texture,"source depth-stencil");
    const auto vertex=device.create_shader({ShaderStage::vertex,"source.vert",0,0},"source vertex");
    const auto fragment=device.create_shader({ShaderStage::fragment,"source.frag",0,0},"source fragment");
    PipelineDesc pipeline_desc; pipeline_desc.vertex_shader=vertex; pipeline_desc.fragment_shader=fragment;
    const auto pipeline=device.create_pipeline(PipelineKey(pipeline_desc),"source pipeline");
    const auto vertices=device.create_buffer({36,BufferUsage::vertex,true},"source vertices");
    DrawDesc draw; draw.pipeline=pipeline; draw.vertex_buffer=vertices; draw.vertex_or_index_count=3;
    {
        zh::original_runtime::OriginalGpuEdge edge(device);
        bool inactive=false;
        try { edge.clear_source_viewport(true,true,true,{0,0,0,1},1,0); }
        catch (const std::runtime_error&) { inactive=true; }
        check(inactive);
        edge.bind_frame_targets(color,depth,48,32);
        edge.begin_source_frame(true,true,0.1F,0.2F,0.3F,1);
        check(device.draw(draw));
        edge.set_source_viewport(8,4,24,16,0,1);
        const auto before=device.snapshot();
        bool invalid=false;
        try { edge.clear_source_viewport(true,true,true,{0,0,0,1},1,256); }
        catch (const std::runtime_error&) { invalid=true; }
        check(invalid && device.snapshot()==before);
        edge.clear_source_viewport(true,true,true,{0.25F,0.5F,0.75F,1},0.5F,9);
        check(device.draw(draw));
        edge.end_source_frame(false);
        const auto stream=device.snapshot();
        const auto first=stream.find("draw pipeline=");
        const auto clear=stream.find("clear_viewport rect=8,4,24,16");
        const auto second=stream.find("draw pipeline=",first+1);
        check(first!=std::string::npos && clear>first && second>clear);
        check(stream.find("flags=CDS color=0.250000,0.500000,0.750000,1.000000 depth=0.500000 stencil=9")!=
            std::string::npos);
        bool ended=false;
        try { edge.clear_source_viewport(true,false,false,{0,0,0,1},1,0); }
        catch (const std::runtime_error&) { ended=true; }
        check(ended);
        edge.bind_frame_targets(color,depth,48,32);
        edge.begin_source_frame(false,false,0,0,0,1);
        bool no_viewport=false;
        try { edge.clear_source_viewport(true,false,false,{0,0,0,1},1,0); }
        catch (const std::runtime_error&) { no_viewport=true; }
        check(no_viewport);
        edge.abort_source_frame();
    }
    device.destroy(vertices); device.destroy(pipeline); device.destroy(fragment); device.destroy(vertex);
    device.destroy(depth); device.destroy(color);
    check(device.resource_counts().total()==0);
}
} // namespace

int main()
{
    first_original_buffer_bytes();
    injected_device_failures();
    canonical_fvf_layouts();
    original_wrapper_indexed_methods();
    original_dynamic_access_owner();
    source_camera_clear_edge();
}
