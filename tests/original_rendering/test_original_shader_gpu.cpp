#include "original_gpu_edge.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8fvf.h"
#include "dx8wrapper.h"
#include "shader.h"
#include "texture.h"
#include "TARGA.H"
#include "ffactory.h"
#include "wwfile.h"
#include "ww3d.h"
#include "vertmaterial.h"
#include "light.h"
#include "lightenvironment.h"
#include "zh/platform/sdl_gpu_device.h"

#include <SDL3/SDL.h>

#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>

namespace {
void check(bool condition, const std::string& message)
{
    if (!condition) throw std::runtime_error(message);
}

struct SourceVertex { float position[3]; std::uint32_t diffuse; float uv[2]; };
static_assert(sizeof(SourceVertex)==24);
struct SourceVertex2 { float position[3]; std::uint32_t diffuse; float uv0[2],uv1[2]; };
static_assert(sizeof(SourceVertex2)==32);
struct LitVertex { float position[3],normal[3]; };
struct LitVertex1 { float position[3],normal[3],uv[2]; };
struct LitVertex2 { float position[3],normal[3],uv0[2],uv1[2]; };
static_assert(sizeof(LitVertex)==24 && sizeof(LitVertex1)==32 && sizeof(LitVertex2)==40);

class OwnedFile final : public FileClass {
public:
    OwnedFile(std::string name,std::vector<unsigned char> bytes)
        :name_(std::move(name)),bytes_(std::move(bytes)) {}
    const char* File_Name() const override { return name_.c_str(); }
    const char* Set_Name(const char* name) override { name_=name; return name_.c_str(); }
    int Create() override { return 0; }
    int Delete() override { return 0; }
    bool Is_Available(int=0) override { return !bytes_.empty(); }
    bool Is_Open() const override { return open_; }
    int Open(const char* name,int rights=READ) override { Set_Name(name); return Open(rights); }
    int Open(int rights=READ) override { open_=!bytes_.empty() && rights==READ; pos_=0; return open_; }
    int Read(void* out,int count) override {
        if (!open_ || count<0) return 0;
        const size_t n=std::min(static_cast<size_t>(count),bytes_.size()-pos_);
        std::memcpy(out,bytes_.data()+pos_,n); pos_+=n; return static_cast<int>(n);
    }
    int Seek(int offset,int mode=SEEK_CUR) override {
        const auto base=mode==SEEK_SET ? 0LL : mode==SEEK_END ?
            static_cast<long long>(bytes_.size()) : static_cast<long long>(pos_);
        const auto next=base+offset;
        if (next<0 || next>static_cast<long long>(bytes_.size())) throw std::runtime_error("owned fixture seek");
        pos_=static_cast<size_t>(next); return static_cast<int>(pos_);
    }
    int Size() override { return static_cast<int>(bytes_.size()); }
    int Write(const void*,int) override { return 0; }
    void Close() override { open_=false; }
private:
    std::string name_; std::vector<unsigned char> bytes_; size_t pos_=0; bool open_=false;
};
class OwnedFactory final : public FileFactoryClass {
public:
    std::map<std::string,std::vector<unsigned char>> files;
    int owners=0;
    FileClass* Get_File(const char* name) override { ++owners; return new OwnedFile(name,files[name]); }
    void Return_File(FileClass* file) override { --owners; delete file; }
};
class FactoryScope final {
public:
    explicit FactoryScope(OwnedFactory& factory):previous_(_TheFileFactory) { _TheFileFactory=&factory; }
    ~FactoryScope() { _TheFileFactory=previous_; }
private:
    FileFactoryClass* previous_;
};
std::vector<unsigned char> owned_targa(std::array<unsigned char,4> bgra)
{
    TGAHeader header{};
    header.ImageType=TGA_TRUECOLOR; header.Width=2; header.Height=2;
    header.PixelDepth=32; header.ImageDescriptor=0x28;
    std::vector<unsigned char> bytes(sizeof(header)+16+14,0);
    std::memcpy(bytes.data(),&header,sizeof(header));
    for (unsigned pixel=0;pixel<4;++pixel)
        std::memcpy(bytes.data()+sizeof(header)+pixel*4,bgra.data(),4);
    return bytes;
}

void original_unlit_source_pixels(unsigned generation,unsigned width,unsigned height)
{
    using namespace zh::renderer;
    SdlGpuOptions options;
    options.shader_root=ZH_GPU_SHADER_DIR;
    options.debug=true;
    SdlGpuDevice device(options);
    check(device.capabilities().backend=="vulkan","original shader selected non-Vulkan backend");
    TextureDesc color_desc;
    color_desc.width=width; color_desc.height=height;
    color_desc.format=TextureFormat::rgba8; color_desc.render_target=true;
    color_desc.sampled=false;
    const auto color=device.create_texture(color_desc,"original shader diagnostic target");
    TextureDesc depth_desc=color_desc;
    depth_desc.format=TextureFormat::depth24_stencil8;
    const auto depth=device.create_texture(depth_desc,"original shader diagnostic depth");
    check(color && depth,device.last_error());
    {
        zh::original_runtime::OriginalGpuEdge edge(device);
        ShaderClass shader;
        shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
        shader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
        DX8Wrapper::Set_Shader(shader);
        DX8Wrapper::Set_Material(nullptr);
        DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
        DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
        DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
        DX8Wrapper::Apply_Render_State_Changes();
        const auto state=edge.prepare_applied_state(DX8_FVF_XYZDUV1);
        check(state.texture_mask==0,"original no-texture shader invented a source stage");
        // Retail normal/UV1 and already accepted source fixture normal/UV2
        // require exact input-location variants. Their lit pixels belong to
        // original category issuance in 06, not this device-only probe.
        for (const auto& [fvf,variant] : {std::pair{DX8_FVF_XYZN,"n0"},
                                         std::pair{DX8_FVF_XYZNUV1,"n1"},
                                         std::pair{DX8_FVF_XYZNUV2,"n2"}}) {
            const std::string vertex_name="renderer/original_applied_"+std::string(variant)+".vert";
            const auto normal_shader=device.create_shader(
                {ShaderStage::vertex,vertex_name,1,0},
                "original normal FVF exact-input validation");
            const auto normal_fragment=device.create_shader(
                {ShaderStage::fragment,"renderer/original_applied_0.frag",1,0},
                "original shader input validation fragment");
            PipelineDesc normal_pipeline;
            normal_pipeline.vertex_shader=normal_shader;
            normal_pipeline.fragment_shader=normal_fragment;
            normal_pipeline.vertex_layout=VertexLayout::original_fvf;
            normal_pipeline.original_fvf=edge.layout_for_fvf(fvf);
            const auto normal_handle=device.create_pipeline(PipelineKey(normal_pipeline),
                "original source FVF input-only Vulkan probe");
            check(normal_shader && normal_fragment && normal_handle,device.last_error());
            device.destroy(normal_handle);
            device.destroy(normal_shader);
            device.destroy(normal_fragment);
        }
        auto* vb=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,3));
        auto* ib=NEW_REF(DX8IndexBufferClass,(3));
        const std::array<SourceVertex,3> source_vertices{{
            {{-0.8F,-0.8F,0.5F},0xff40c080U,{0.0F,0.0F}},
            {{0.8F,-0.8F,0.5F},0xff40c080U,{1.0F,0.0F}},
            {{0.0F,0.8F,0.5F},0xff40c080U,{0.5F,1.0F}},
        }};
        {
            VertexBufferClass::WriteLockClass vertices(vb);
            check(vb->FVF_Info().Get_FVF_Size()==sizeof(SourceVertex),
                "original FVFInfoClass disagrees with source upload bytes");
            std::memcpy(vertices.Get_Vertex_Array(),source_vertices.data(),sizeof(source_vertices));
            IndexBufferClass::WriteLockClass indices(ib);
            indices.Get_Index_Array()[0]=0;
            indices.Get_Index_Array()[1]=1;
            indices.Get_Index_Array()[2]=2;
        }
        const auto vertex=edge.bind_vertex(vb);
        const auto index=edge.bind_index(ib);
        vb->Release_Ref(); ib->Release_Ref();
        RenderPassDesc pass;
        pass.color_targets[0]=color; pass.color_target_count=1;
        pass.depth_target=depth; pass.width=width; pass.height=height;
        check(device.begin_pass(pass,"original source shader physical lowering probe"),device.last_error());
        edge.validate_prepared_state(state);
        DrawDesc draw;
        draw.pipeline=state.pipeline;
        draw.vertex_buffer=vertex; draw.index_buffer=index;
        draw.vertex_or_index_count=3;
        draw.index_element_size=IndexElementSize::uint16;
        draw.vertex_bindings=state.vertex_bindings;
        draw.fragment_bindings=state.fragment_bindings;
        check(device.draw(draw),device.last_error());
        check(device.end_pass(),device.last_error());
        const auto pixels=device.readback_rgba(color);
        check(pixels.size()==static_cast<std::size_t>(width)*height*4,device.last_error());
        const auto center=(height/2*width+width/2)*4;
        const auto corner=0U;
        check(std::abs(static_cast<int>(pixels[center])-64)<=5 &&
              std::abs(static_cast<int>(pixels[center+1])-192)<=5 &&
              std::abs(static_cast<int>(pixels[center+2])-128)<=5 &&
              pixels[center+3]>=250,
              "original ARGB diffuse and unlit source combiner pixel semantics differ");
        check(pixels[center]!=pixels[corner] || pixels[center+1]!=pixels[corner+1],
            "original source shader did not shade diagnostic triangle");
        std::cout<<"original-applied-unlit-pixels=pass generation="<<generation
                 <<" center-rgb="<<static_cast<unsigned>(pixels[center])<<":"
                 <<static_cast<unsigned>(pixels[center+1])<<":"
                 <<static_cast<unsigned>(pixels[center+2])<<'\n';
        OwnedFactory files;
        files.files["owned-zero.tga"]=owned_targa({16,128,240,128});
        files.files["owned-one.tga"]=owned_targa({96,64,128,255});
        FactoryScope factory(files);
        WW3D::Set_Thumbnail_Enabled(false);
        WW3D::Set_Texture_Reduction(0,1);
        WW3D::Enable_Texturing(true);
        TextureClass source_zero("source-zero","owned-zero.tga",MIP_LEVELS_ALL,
            WW3D_FORMAT_UNKNOWN,true,true);
        TextureClass source_one("source-one","owned-one.tga",MIP_LEVELS_ALL,
            WW3D_FORMAT_UNKNOWN,true,true);
        source_zero.Apply(0);
        check(!source_zero.Is_Missing_Texture() && files.owners==0,
            "original source texture decoder did not load owned pixels");
        shader.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
        DX8Wrapper::Set_Shader(shader);
        DX8Wrapper::Apply_Render_State_Changes();
        const auto one_state=edge.prepare_applied_state(DX8_FVF_XYZDUV1);
        check(one_state.texture_mask==1 &&
              one_state.fragment_bindings.textures[0]==edge.texture_handle(&source_zero),
              "original source texture stage zero was not selected");
        const auto shade=[&](const zh::original_runtime::OriginalGpuEdge::PhysicalState& prepared,
                             BufferHandle selected_vertex,const char* label) {
            check(device.begin_pass(pass,label),device.last_error());
            edge.validate_prepared_state(prepared);
            draw.pipeline=prepared.pipeline;
            draw.vertex_buffer=selected_vertex;
            draw.vertex_bindings=prepared.vertex_bindings;
            draw.fragment_bindings=prepared.fragment_bindings;
            check(device.draw(draw),device.last_error());
            check(device.end_pass(),device.last_error());
            const auto output=device.readback_rgba(color);
            check(output.size()==static_cast<std::size_t>(width)*height*4,device.last_error());
            return std::array<int,4>{output[center],output[center+1],output[center+2],output[center+3]};
        };
        Matrix4x4 world_shift(true);
        world_shift[0].W=0.7f;
        DX8Wrapper::Set_Transform(D3DTS_WORLD,world_shift);
        const auto shifted_state=edge.prepare_applied_state(DX8_FVF_XYZDUV1);
        const auto shifted=shade(shifted_state,vertex,"original source world-translation probe");
        check(shifted[0]==5 && shifted[1]==5 && shifted[2]==10,
            "original nonidentity Matrix4x4 world transform did not preserve native DX8 transpose");
        DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
        const auto restored_one=edge.prepare_applied_state(DX8_FVF_XYZDUV1);
        const auto textured=shade(restored_one,vertex,"original source one-stage shader physical probe");
        check(std::abs(textured[0]-60)<=5 && std::abs(textured[1]-96)<=5 &&
              std::abs(textured[2]-8)<=5 && std::abs(textured[3]-128)<=5,
              "original decoded BGRA stage-zero modulate pixel differs from authored source");
        std::cout<<"original-applied-one-stage-pixels=pass generation="<<generation
                 <<" center-rgb="<<textured[0]<<":"<<textured[1]<<":"<<textured[2]<<'\n';
        source_one.Apply(1);
        check(!source_one.Is_Missing_Texture() && files.owners==0,
            "original second texture decoder did not load owned pixels");
        shader.Set_Post_Detail_Color_Func(ShaderClass::DETAILCOLOR_ADD);
        DX8Wrapper::Set_Shader(shader);
        DX8Wrapper::Apply_Render_State_Changes();
        const auto two_state=edge.prepare_applied_state(DX8_FVF_XYZDUV2);
        check(two_state.texture_mask==3 && two_state.fragment_bindings.texture_count==2 &&
              two_state.fragment_bindings.textures[0]==edge.texture_handle(&source_zero) &&
              two_state.fragment_bindings.textures[1]==edge.texture_handle(&source_one),
              "original second source texture stage was not bound in source order");
        auto* two_vb=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV2,3));
        const std::array<SourceVertex2,3> two_vertices{{
            {{-0.8F,-0.8F,0.5F},0xff40c080U,{0.0F,0.0F},{0.0F,0.0F}},
            {{0.8F,-0.8F,0.5F},0xff40c080U,{1.0F,0.0F},{1.0F,0.0F}},
            {{0.0F,0.8F,0.5F},0xff40c080U,{0.5F,1.0F},{0.5F,1.0F}},
        }};
        {
            VertexBufferClass::WriteLockClass vertices(two_vb);
            check(two_vb->FVF_Info().Get_FVF_Size()==sizeof(SourceVertex2),
                "original two-UV FVF byte stride differs from source");
            std::memcpy(vertices.Get_Vertex_Array(),two_vertices.data(),sizeof(two_vertices));
        }
        const auto two_vertex=edge.bind_vertex(two_vb);
        two_vb->Release_Ref();
        const auto detailed=shade(two_state,two_vertex,"original source two-stage shader physical probe");
        check(std::abs(detailed[0]-188)<=5 && std::abs(detailed[1]-160)<=5 &&
              std::abs(detailed[2]-104)<=5 && std::abs(detailed[3]-128)<=5,
              "original decoded two-stage additive source pixel differs from authored shader");
        std::cout<<"original-applied-two-stage-pixels=pass generation="<<generation
                 <<" center-rgb="<<detailed[0]<<":"<<detailed[1]<<":"<<detailed[2]<<'\n';
        shader.Set_Post_Detail_Color_Func(ShaderClass::DETAILCOLOR_DISABLE);
        ShaderClass::Invalidate();
        DX8Wrapper::Set_Shader(shader);
        DX8Wrapper::Apply_Render_State_Changes();
        unsigned interpreted=0;
        for (unsigned op : {D3DTOP_DISABLE,D3DTOP_SELECTARG1,D3DTOP_SELECTARG2,
                            D3DTOP_MODULATE,D3DTOP_ADD}) {
            DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_COLOROP,op);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_ALPHAOP,op);
            for (unsigned arg : {D3DTA_DIFFUSE,D3DTA_CURRENT,D3DTA_TEXTURE}) {
                for (unsigned field : {D3DTSS_COLORARG1,D3DTSS_COLORARG2,
                                       D3DTSS_ALPHAARG1,D3DTSS_ALPHAARG2})
                    DX8Wrapper::Set_DX8_Texture_Stage_State(0,field,arg);
                const auto requested=edge.prepare_applied_state(DX8_FVF_XYZDUV1);
                const auto observed=shade(requested,vertex,"original authored combiner operator/argument probe");
                const std::array<int,4> diffuse{{64,192,128,255}};
                const std::array<int,4> texture{{240,128,16,128}};
                const auto& selected=arg==D3DTA_TEXTURE?texture:diffuse;
                for (unsigned channel=0;channel<4;++channel) {
                    int expected=diffuse[channel];
                    if (op==D3DTOP_SELECTARG1 || op==D3DTOP_SELECTARG2) expected=selected[channel];
                    else if (op==D3DTOP_MODULATE) expected=(selected[channel]*selected[channel]+127)/255;
                    else if (op==D3DTOP_ADD) expected=std::min(255,2*selected[channel]);
                    check(std::abs(observed[channel]-expected)<=5,
                        "original combiner source op/arg pixel differs from bounded GPU interpreter");
                }
                ++interpreted;
            }
        }
        DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE);
        const auto color_texture=edge.prepare_applied_state(DX8_FVF_XYZDUV1);
        const auto independent_color=shade(color_texture,vertex,"original independent source color probe");
        check(std::abs(independent_color[0]-240)<=5 && std::abs(independent_color[1]-128)<=5 &&
              std::abs(independent_color[2]-16)<=5 && independent_color[3]>=250,
              "original independent color/alpha stage produced incorrect source texture color");
        DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_COLORARG1,D3DTA_DIFFUSE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
        const auto alpha_texture=edge.prepare_applied_state(DX8_FVF_XYZDUV1);
        const auto independent_alpha=shade(alpha_texture,vertex,"original independent source alpha probe");
        check(std::abs(independent_alpha[0]-64)<=5 && std::abs(independent_alpha[1]-192)<=5 &&
              std::abs(independent_alpha[2]-128)<=5 && std::abs(independent_alpha[3]-128)<=5,
              "original independent color/alpha stage produced incorrect source texture alpha");
        std::cout<<"original-applied-combiner-pixels=pass generation="<<generation
                 <<" operation-argument-cases="<<interpreted<<" independent-channels=2\n";
        ShaderClass::Invalidate();
        shader.Set_Alpha_Test(ShaderClass::ALPHATEST_ENABLE);
        shader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_SRC_ALPHA);
        shader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA);
        DX8Wrapper::Set_Shader(shader);
        DX8Wrapper::Apply_Render_State_Changes();
        source_zero.Apply(0);
        const auto alpha_mapped=zh::original_runtime::OriginalGpuEdge::map_applied_state(DX8_FVF_XYZDUV1);
        check(alpha_mapped.alpha_test && alpha_mapped.pipeline.blend.enabled &&
              alpha_mapped.alpha_reference==96.0f/255.0f,
              "original authored alpha-test/blend state was not selected");
        const auto alpha_state=edge.prepare_applied_state(DX8_FVF_XYZDUV1);
        const auto blended=shade(alpha_state,vertex,"original authored alpha/blend physical probe");
        check(std::abs(blended[0]-33)<=5 && std::abs(blended[1]-51)<=5 &&
              std::abs(blended[2]-9)<=5 && std::abs(blended[3]-191)<=5,
              "original alpha-tested source blending pixel differs from authored state");
        files.files["owned-reject.tga"]=owned_targa({16,128,240,32});
        TextureClass rejected_alpha("source-rejected","owned-reject.tga",MIP_LEVELS_ALL,
            WW3D_FORMAT_UNKNOWN,true,true);
        rejected_alpha.Apply(0);
        check(!rejected_alpha.Is_Missing_Texture() && files.owners==0,
            "original rejected-alpha texture was not decoded from owned source");
        const auto rejected_state=edge.prepare_applied_state(DX8_FVF_XYZDUV1);
        const auto discarded=shade(rejected_state,vertex,"original authored alpha reject physical probe");
        check(std::abs(discarded[0]-5)<=2 && std::abs(discarded[1]-5)<=2 &&
              std::abs(discarded[2]-10)<=2 && discarded[3]>=250,
              "original alpha test did not discard the source-rejected fragment");
        std::cout<<"original-applied-alpha-blend=pass generation="<<generation
                 <<" accepted="<<blended[0]<<":"<<blended[1]<<":"<<blended[2]
                 <<" rejected=clear\n";
        shader.Set_Alpha_Test(ShaderClass::ALPHATEST_DISABLE);
        shader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_ONE);
        shader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ZERO);
        shader.Set_Fog_Func(ShaderClass::FOG_ENABLE);
        DX8Wrapper::Set_Fog(true,Vector3(1.0f,0.0f,0.0f),0.0f,1.0f);
        DX8Wrapper::Set_Shader(shader);
        DX8Wrapper::Apply_Render_State_Changes();
        source_zero.Apply(0);
        const auto fog_mapped=zh::original_runtime::OriginalGpuEdge::map_applied_state(DX8_FVF_XYZDUV1);
        check(fog_mapped.pipeline.fog_enabled && fog_mapped.fog_color[0]==1.0f &&
              fog_mapped.fog_start==0.0f && fog_mapped.fog_end==1.0f,
              "original source fog selection did not reach physical mapping");
        const auto fog_state=edge.prepare_applied_state(DX8_FVF_XYZDUV1);
        const auto fogged=shade(fog_state,vertex,"original authored fog physical probe");
        check(std::abs(fogged[0]-158)<=5 && std::abs(fogged[1]-48)<=5 &&
              std::abs(fogged[2]-4)<=5 && std::abs(fogged[3]-128)<=5,
              "original view-space fog source pixel differs from authored state");
        std::cout<<"original-applied-fog-pixels=pass generation="<<generation
                 <<" center-rgb="<<fogged[0]<<":"<<fogged[1]<<":"<<fogged[2]<<'\n';
        shader.Set_Fog_Func(ShaderClass::FOG_DISABLE);
        DX8Wrapper::Set_Fog(false,Vector3(0.0f,0.0f,0.0f),0.0f,1.0f);
        DX8Wrapper::Set_Shader(shader);
        DX8Wrapper::Apply_Render_State_Changes();
        auto checker=owned_targa({0,0,255,255});
        for (unsigned pixel : {1U,3U}) {
            const std::array<unsigned char,4> blue{{255,0,0,255}};
            std::memcpy(checker.data()+sizeof(TGAHeader)+pixel*4,blue.data(),4);
        }
        files.files["owned-uv.tga"]=checker;
        TextureClass uv_texture("source-uv","owned-uv.tga",MIP_LEVELS_ALL,
            WW3D_FORMAT_UNKNOWN,true,true);
        uv_texture.Get_Filter().Set_Min_Filter(TextureFilterClass::FILTER_TYPE_NONE);
        uv_texture.Get_Filter().Set_Mag_Filter(TextureFilterClass::FILTER_TYPE_NONE);
        uv_texture.Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        uv_texture.Apply(0);
        auto* uv_vb=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,3));
        auto uv_vertices=source_vertices;
        for (auto& v:uv_vertices) { v.uv[0]=0.25f; v.uv[1]=0.25f; }
        {
            VertexBufferClass::WriteLockClass vertices(uv_vb);
            std::memcpy(vertices.Get_Vertex_Array(),uv_vertices.data(),sizeof(uv_vertices));
        }
        const auto uv_vertex=edge.bind_vertex(uv_vb);
        uv_vb->Release_Ref();
        const auto uv_original=edge.prepare_applied_state(DX8_FVF_XYZDUV1);
        const auto left=shade(uv_original,uv_vertex,"original untransformed UV source probe");
        Matrix4x4 uv_shift(true);
        uv_shift[0].W=0.5f;
        DX8Wrapper::Set_Transform(D3DTS_TEXTURE0,uv_shift);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
        const auto uv_transformed=edge.prepare_applied_state(DX8_FVF_XYZDUV1);
        const auto right=shade(uv_transformed,uv_vertex,"original transformed UV source probe");
        check(std::abs(left[0]-64)<=5 && left[2]<=5 &&
              right[0]<=5 && std::abs(right[2]-128)<=5,
              "original source-issued UV transform did not select authored texture texels");
        std::cout<<"original-applied-uv-transform=pass generation="<<generation
                 <<" left="<<left[0]<<":"<<left[2]
                 <<" right="<<right[0]<<":"<<right[2]<<'\n';
    }
    device.destroy(depth); device.destroy(color);
    check(device.wait_idle(),device.last_error());
}

void original_lit_source_pixels(unsigned generation)
{
    using namespace zh::renderer;
    SdlGpuOptions options; options.shader_root=ZH_GPU_SHADER_DIR; options.debug=true;
    SdlGpuDevice device(options);
    check(device.capabilities().backend=="vulkan","original lit shader selected non-Vulkan backend");
    TextureDesc target; target.width=160; target.height=120;
    target.format=TextureFormat::rgba8; target.render_target=true;
    const auto color=device.create_texture(target,"source lit target");
    target.format=TextureFormat::depth24_stencil8;
    const auto depth=device.create_texture(target,"source lit depth");
    check(color && depth,device.last_error());
    LightEnvironmentClass environment;
    environment.Reset(Vector3(0,0,0),Vector3(0.1F,0.2F,0.3F));
    environment.Pre_Render_Update(Matrix3D(true));
    {
        zh::original_runtime::OriginalGpuEdge edge(device);
        ShaderClass shader;
        shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
        shader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
        auto* material=NEW_REF(VertexMaterialClass,());
        material->Set_Lighting(true);
        material->Set_Diffuse(Vector3(0.5F,0.4F,0.3F));
        material->Set_Ambient(Vector3(0.2F,0.3F,0.4F));
        material->Set_Emissive(Vector3(0.01F,0.02F,0.03F));
        material->Set_Opacity(0.75F);
        DX8Wrapper::Set_Material(material);
        material->Release_Ref();
        DX8Wrapper::Set_Shader(shader);
        DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
        DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
        DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
        DX8Wrapper::Set_Light_Environment(&environment);
        DX8Wrapper::Apply_Render_State_Changes();
        auto* vb=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZN,3));
        auto* ib=NEW_REF(DX8IndexBufferClass,(3));
        const std::array<LitVertex,3> source_vertices{{
            {{-0.8F,-0.8F,0.5F},{0,0,1}},
            {{0.8F,-0.8F,0.5F},{0,0,1}},
            {{0.0F,0.8F,0.5F},{0,0,1}},
        }};
        {
            VertexBufferClass::WriteLockClass lock(vb);
            check(vb->FVF_Info().Get_FVF_Size()==sizeof(LitVertex),"original lit FVF N0 stride differs");
            std::memcpy(lock.Get_Vertex_Array(),source_vertices.data(),sizeof(source_vertices));
            IndexBufferClass::WriteLockClass indices(ib);
            for (unsigned i=0;i<3;++i) indices.Get_Index_Array()[i]=i;
        }
        const auto vertex=edge.bind_vertex(vb),index=edge.bind_index(ib);
        vb->Release_Ref(); ib->Release_Ref();
        RenderPassDesc pass;
        pass.color_targets[0]=color; pass.color_target_count=1;
        pass.depth_target=depth; pass.width=160; pass.height=120;
        const auto draw_lit=[&](const char* label,BufferHandle selected_vertex,unsigned selected_fvf) {
            const auto state=edge.prepare_applied_state(selected_fvf);
            check(device.begin_pass(pass,label),device.last_error());
            DrawDesc draw; draw.pipeline=state.pipeline;
            draw.vertex_buffer=selected_vertex; draw.index_buffer=index;
            draw.vertex_or_index_count=3; draw.index_element_size=IndexElementSize::uint16;
            draw.vertex_bindings=state.vertex_bindings;
            draw.fragment_bindings=state.fragment_bindings;
            edge.validate_prepared_state(state);
            check(device.draw(draw),device.last_error());
            check(device.end_pass(),device.last_error());
            const auto pixels=device.readback_rgba(color);
            check(pixels.size()==160U*120U*4U,device.last_error());
            const auto i=(60U*160U+80U)*4U;
            return std::array<int,4>{pixels[i],pixels[i+1],pixels[i+2],pixels[i+3]};
        };
        const auto ambient=draw_lit("source original ambient-only lit physical probe",vertex,DX8_FVF_XYZN);
        check(std::abs(ambient[0]-8)<=5 && std::abs(ambient[1]-20)<=5 &&
              std::abs(ambient[2]-38)<=5 && std::abs(ambient[3]-191)<=5,
              "original ambient/emissive/material source pixel differs");
        LightClass directional(LightClass::DIRECTIONAL);
        Matrix3D directional_transform(true);
        directional_transform.Rotate_X(WWMATH_PI);
        directional.Set_Transform(directional_transform);
        directional.Set_Diffuse(Vector3(0.4F,0.5F,0.6F));
        environment.Add_Light(directional);
        environment.Pre_Render_Update(Matrix3D(true));
        DX8Wrapper::Set_Light_Environment(&environment);
        const auto lit=draw_lit("source original directional lit physical probe",vertex,DX8_FVF_XYZN);
        const auto issued=DX8Wrapper::Snapshot_Source_State();
        check(issued.light_enabled[0] && issued.lights[0].Type==D3DLIGHT_DIRECTIONAL &&
              issued.lights[0].Direction.z<0.0F,
              "original rotated directional source did not select the light");
        const unsigned packed=issued.render.at(D3DRS_AMBIENT);
        const std::array<float,3> global{{static_cast<float>((packed>>16)&255)/255.0F,
            static_cast<float>((packed>>8)&255)/255.0F,
            static_cast<float>(packed&255)/255.0F}};
        const auto& source_light=issued.lights[0];
        const std::array<float,3> expected{{
            255.0F*(0.01F+0.2F*global[0]+0.5F*source_light.Diffuse.r),
            255.0F*(0.02F+0.3F*global[1]+0.4F*source_light.Diffuse.g),
            255.0F*(0.03F+0.4F*global[2]+0.3F*source_light.Diffuse.b),
        }};
        check(std::abs(lit[0]-expected[0])<=6 && std::abs(lit[1]-expected[1])<=6 &&
              std::abs(lit[2]-expected[2])<=6 && std::abs(lit[3]-191)<=5,
              "original directional diffuse/material source pixel differs");
        LightClass point(LightClass::POINT);
        point.Set_Position(Vector3(0,0,1));
        point.Set_Diffuse(Vector3(0.6F,0.4F,0.2F));
        point.Set_Ambient(Vector3(0.02F,0.03F,0.04F));
        point.Set_Near_Attenuation_Range(0,2);
        point.Set_Far_Attenuation_Range(2,10);
        environment.Reset(Vector3(0,0,0),Vector3(0.1F,0.2F,0.3F));
        environment.Add_Light(point);
        environment.Pre_Render_Update(Matrix3D(true));
        DX8Wrapper::Set_Light_Environment(&environment);
        auto* point_vb=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZN,3));
        auto point_vertices=source_vertices;
        for (auto& selected:point_vertices) {
            selected.position[0]*=0.125F;
            selected.position[1]*=0.125F;
        }
        {
            VertexBufferClass::WriteLockClass lock(point_vb);
            std::memcpy(lock.Get_Vertex_Array(),point_vertices.data(),sizeof(point_vertices));
        }
        const auto point_vertex=edge.bind_vertex(point_vb);
        point_vb->Release_Ref();
        const auto point_pixel=draw_lit("source original point lit physical probe",point_vertex,DX8_FVF_XYZN);
        const auto point_state=DX8Wrapper::Snapshot_Source_State();
        const auto& point_light=point_state.lights[0];
        check(point_light.Type==D3DLIGHT_POINT && point_state.light_enabled[0],
              "original point light was not source-selected");
        const float point_distance=std::fabs(point_light.Position.z-0.5F);
        const float attenuation=1.0F/(point_light.Attenuation0+
            point_light.Attenuation1*point_distance+
            point_light.Attenuation2*point_distance*point_distance);
        const auto global_point=point_state.render.at(D3DRS_AMBIENT);
        const std::array<float,3> expected_point{{
            255.0F*(0.01F+0.2F*(float((global_point>>16)&255)/255.0F+
                point_light.Ambient.r*attenuation)+0.5F*point_light.Diffuse.r*attenuation),
            255.0F*(0.02F+0.3F*(float((global_point>>8)&255)/255.0F+
                point_light.Ambient.g*attenuation)+0.4F*point_light.Diffuse.g*attenuation),
            255.0F*(0.03F+0.4F*(float(global_point&255)/255.0F+
                point_light.Ambient.b*attenuation)+0.3F*point_light.Diffuse.b*attenuation),
        }};
        for (unsigned channel=0;channel<3;++channel)
            check(std::abs(point_pixel[channel]-expected_point[channel])<=6,
                  "original point attenuation/ambient/diffuse pixel differs");
        D3DLIGHT8 out_of_range=point_light;
        out_of_range.Range=0.1F;
        DX8Wrapper::Set_Light(0,&out_of_range);
        const auto clipped_point=draw_lit("source out-of-range point light physical probe",
            point_vertex,DX8_FVF_XYZN);
        const std::array<float,3> clipped_expected{{
            255.0F*(0.01F+0.2F*float((global_point>>16)&255)/255.0F),
            255.0F*(0.02F+0.3F*float((global_point>>8)&255)/255.0F),
            255.0F*(0.03F+0.4F*float(global_point&255)/255.0F),
        }};
        for (unsigned channel=0;channel<3;++channel)
            check(std::abs(clipped_point[channel]-clipped_expected[channel])<=5,
                  "original source point range failed to suppress diffuse and ambient terms");
        environment.Reset(Vector3(0,0,0),Vector3(0.02F,0.03F,0.04F));
        for (unsigned light_index=0;light_index<4;++light_index) {
            LightClass selected(LightClass::DIRECTIONAL);
            selected.Set_Transform(directional_transform);
            selected.Set_Diffuse(Vector3(0.10F,0.05F,0.025F));
            environment.Add_Light(selected);
        }
        environment.Pre_Render_Update(Matrix3D(true));
        DX8Wrapper::Set_Light_Environment(&environment);
        const auto four=draw_lit("source four-light bound physical probe",vertex,DX8_FVF_XYZN);
        const auto four_state=DX8Wrapper::Snapshot_Source_State();
        const unsigned four_ambient=four_state.render.at(D3DRS_AMBIENT);
        const std::array<float,3> four_expected{{
            255.0F*(0.01F+0.2F*float((four_ambient>>16)&255)/255.0F),
            255.0F*(0.02F+0.3F*float((four_ambient>>8)&255)/255.0F),
            255.0F*(0.03F+0.4F*float(four_ambient&255)/255.0F),
        }};
        check(std::all_of(four_state.light_enabled.begin(),four_state.light_enabled.end(),
            [](bool enabled){return enabled;}),"original four-light slots not all source enabled");
        for (unsigned channel=0;channel<3;++channel) {
            const auto diffuse=[](const D3DLIGHT8& light,unsigned component) {
                return component==0?light.Diffuse.r:component==1?light.Diffuse.g:light.Diffuse.b;
            };
            float expected_four=four_expected[channel];
            const float material_diffuse=channel==0?0.5F:channel==1?0.4F:0.3F;
            for (const auto& light:four_state.lights)
                expected_four+=255.0F*material_diffuse*diffuse(light,channel);
            check(std::abs(four[channel]-expected_four)<=6,
                  "original four-slot bounded directional pixel differs");
        }
        environment.Reset(Vector3(0,0,0),Vector3(0.05F,0.1F,0.15F));
        environment.Add_Light(directional);
        environment.Pre_Render_Update(Matrix3D(true));
        DX8Wrapper::Set_Light_Environment(&environment);
        auto* diagonal_vb=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZN,3));
        auto diagonal_vertices=source_vertices;
        for (auto& selected:diagonal_vertices) {
            selected.normal[0]=0.70710678F;
            selected.normal[2]=0.70710678F;
        }
        {
            VertexBufferClass::WriteLockClass lock(diagonal_vb);
            std::memcpy(lock.Get_Vertex_Array(),diagonal_vertices.data(),sizeof(diagonal_vertices));
        }
        const auto diagonal_vertex=edge.bind_vertex(diagonal_vb);
        diagonal_vb->Release_Ref();
        Matrix4x4 nonuniform(true);
        nonuniform[0].X=2.0F;
        DX8Wrapper::Set_Transform(D3DTS_WORLD,nonuniform);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_NORMALIZENORMALS,FALSE);
        const auto unnormalized=draw_lit("source inverse-transpose unnormalized normal probe",
            diagonal_vertex,DX8_FVF_XYZN);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_NORMALIZENORMALS,TRUE);
        const auto normalized=draw_lit("source normalized inverse-transpose normal probe",
            diagonal_vertex,DX8_FVF_XYZN);
        const float incidence_raw=0.70710678F;
        const float incidence_normalized=incidence_raw/std::sqrt(0.25F*0.5F+0.5F);
        const auto normal_source=DX8Wrapper::Snapshot_Source_State();
        const unsigned normal_ambient=normal_source.render.at(D3DRS_AMBIENT);
        const float expected_raw=255.0F*(0.01F+0.2F*float((normal_ambient>>16)&255)/255.0F+
            0.5F*normal_source.lights[0].Diffuse.r*incidence_raw);
        const float expected_normalized=255.0F*(0.01F+0.2F*float((normal_ambient>>16)&255)/255.0F+
            0.5F*normal_source.lights[0].Diffuse.r*incidence_normalized);
        check(std::abs(unnormalized[0]-expected_raw)<=6 &&
              std::abs(normalized[0]-expected_normalized)<=6 &&
              normalized[0]>unnormalized[0]+6,
              "original nonuniform world inverse-transpose or authored normal normalization differs");
        DX8Wrapper::Set_DX8_Render_State(D3DRS_NORMALIZENORMALS,FALSE);
        DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
        Matrix3D specular_transform(true);
        specular_transform.Rotate_X(WWMATH_PI);
        specular_transform.Rotate_Y(-0.9272952F);
        directional.Set_Transform(specular_transform);
        environment.Reset(Vector3(0,0,0),Vector3(0.05F,0.1F,0.15F));
        environment.Add_Light(directional);
        environment.Pre_Render_Update(Matrix3D(true));
        DX8Wrapper::Set_Light_Environment(&environment);
        auto* specular_vb=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZN,3));
        auto specular_vertices=source_vertices;
        for (auto& selected:specular_vertices) {
            selected.normal[0]=1.0F;
            selected.normal[2]=0.0F;
        }
        {
            VertexBufferClass::WriteLockClass lock(specular_vb);
            std::memcpy(lock.Get_Vertex_Array(),specular_vertices.data(),sizeof(specular_vertices));
        }
        const auto specular_vertex=edge.bind_vertex(specular_vb);
        specular_vb->Release_Ref();
        DX8Wrapper::Set_DX8_Render_State(D3DRS_LOCALVIEWER,FALSE);
        material->Set_Specular(Vector3(0.3F,0.2F,0.1F));
        material->Set_Shininess(4.0F);
        DX8Wrapper::Set_Material(material);
        DX8Wrapper::Apply_Render_State_Changes();
        const auto specular_off=draw_lit("source authored specular disabled physical probe",
            specular_vertex,DX8_FVF_XYZN);
        shader.Set_Secondary_Gradient(ShaderClass::SECONDARY_GRADIENT_ENABLE);
        DX8Wrapper::Set_Shader(shader);
        DX8Wrapper::Apply_Render_State_Changes();
        const auto specular_on=draw_lit("source authored specular enabled physical probe",
            specular_vertex,DX8_FVF_XYZN);
        const auto selected_specular=DX8Wrapper::Snapshot_Source_State();
        check(selected_specular.render.at(D3DRS_SPECULARENABLE)==1 &&
            selected_specular.render.at(D3DRS_SPECULARMATERIALSOURCE)==D3DMCS_MATERIAL &&
            selected_specular.lights[0].Specular.r==1.0F,
            "original specular producer did not select source material/light terms");
        const float incoming_x=-selected_specular.lights[0].Direction.x;
        const float incoming_z=-selected_specular.lights[0].Direction.z;
        const float half_x=incoming_x/std::sqrt(incoming_x*incoming_x+
            (incoming_z-1.0F)*(incoming_z-1.0F));
        const float expected_specular=255.0F*selected_specular.material.Specular.r*
            selected_specular.lights[0].Specular.r*std::pow(std::max(0.0F,half_x),
                selected_specular.material.Power);
        check(std::abs(float(specular_on[0]-specular_off[0])-expected_specular)<=6 &&
              specular_on[0]>specular_off[0]+20 &&
              specular_on[3]==specular_off[3],
              "original source specular material/power/local-viewer switch differs");
        material->Set_Ambient_Color_Source(VertexMaterialClass::COLOR1);
        material->Set_Diffuse_Color_Source(VertexMaterialClass::COLOR2);
        material->Set_Emissive_Color_Source(VertexMaterialClass::COLOR1);
        DX8Wrapper::Set_Material(material);
        DX8Wrapper::Apply_Render_State_Changes();
        DX8Wrapper::Set_DX8_Render_State(D3DRS_SPECULARMATERIALSOURCE,D3DMCS_COLOR2);
        const auto absent_vertex_colors=draw_lit("original absent COLOR1/COLOR2 material fallback",
            specular_vertex,DX8_FVF_XYZN);
        for (unsigned channel=0;channel<4;++channel)
            check(std::abs(absent_vertex_colors[channel]-specular_on[channel])<=3,
                  "normal-only source FVF failed documented vertex color material fallback");
        material->Set_Ambient_Color_Source(VertexMaterialClass::MATERIAL);
        material->Set_Diffuse_Color_Source(VertexMaterialClass::MATERIAL);
        material->Set_Emissive_Color_Source(VertexMaterialClass::MATERIAL);
        DX8Wrapper::Set_Material(material);
        DX8Wrapper::Apply_Render_State_Changes();
        DX8Wrapper::Set_DX8_Render_State(D3DRS_SPECULARMATERIALSOURCE,D3DMCS_MATERIAL);
        shader.Set_Secondary_Gradient(ShaderClass::SECONDARY_GRADIENT_DISABLE);
        DX8Wrapper::Set_Shader(shader);
        DX8Wrapper::Set_DX8_Render_State(D3DRS_LOCALVIEWER,TRUE);
        DX8Wrapper::Apply_Render_State_Changes();
        directional.Set_Transform(directional_transform);
        environment.Reset(Vector3(0,0,0),Vector3(0.05F,0.1F,0.15F));
        environment.Add_Light(directional);
        environment.Pre_Render_Update(Matrix3D(true));
        DX8Wrapper::Set_Light_Environment(&environment);
        const auto source_untextured=draw_lit("source retail-normal N0 lit stage-zero disabled",
            vertex,DX8_FVF_XYZN);
        OwnedFactory files;
        files.files["lit-zero.tga"]=owned_targa({16,128,240,128});
        files.files["lit-one.tga"]=owned_targa({96,64,128,255});
        FactoryScope factory(files);
        WW3D::Set_Thumbnail_Enabled(false);
        WW3D::Set_Texture_Reduction(0,1);
        WW3D::Enable_Texturing(true);
        TextureClass texture_zero("lit-zero","lit-zero.tga",MIP_LEVELS_ALL,
            WW3D_FORMAT_UNKNOWN,true,true);
        TextureClass texture_one("lit-one","lit-one.tga",MIP_LEVELS_ALL,
            WW3D_FORMAT_UNKNOWN,true,true);
        texture_zero.Apply(0);
        check(!texture_zero.Is_Missing_Texture() && files.owners==0,
            "original lit stage zero did not decode owned source pixels");
        shader.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
        DX8Wrapper::Set_Shader(shader);
        DX8Wrapper::Apply_Render_State_Changes();
        auto* one_vb=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZNUV1,3));
        auto* two_vb=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZNUV2,3));
        std::array<LitVertex1,3> one_vertices{};
        std::array<LitVertex2,3> two_vertices{};
        for (unsigned v=0;v<3;++v) {
            std::memcpy(one_vertices[v].position,source_vertices[v].position,sizeof(source_vertices[v].position));
            std::memcpy(one_vertices[v].normal,source_vertices[v].normal,sizeof(source_vertices[v].normal));
            std::memcpy(two_vertices[v].position,source_vertices[v].position,sizeof(source_vertices[v].position));
            std::memcpy(two_vertices[v].normal,source_vertices[v].normal,sizeof(source_vertices[v].normal));
            one_vertices[v].uv[0]=two_vertices[v].uv0[0]=two_vertices[v].uv1[0]=0.25F;
            one_vertices[v].uv[1]=two_vertices[v].uv0[1]=two_vertices[v].uv1[1]=0.25F;
        }
        {
            VertexBufferClass::WriteLockClass lock(one_vb);
            check(one_vb->FVF_Info().Get_FVF_Size()==sizeof(LitVertex1),
                "original normal+UV1 FVF stride differs");
            std::memcpy(lock.Get_Vertex_Array(),one_vertices.data(),sizeof(one_vertices));
        }
        {
            VertexBufferClass::WriteLockClass lock(two_vb);
            check(two_vb->FVF_Info().Get_FVF_Size()==sizeof(LitVertex2),
                "original normal+UV2 FVF stride differs");
            std::memcpy(lock.Get_Vertex_Array(),two_vertices.data(),sizeof(two_vertices));
        }
        const auto vertex_one=edge.bind_vertex(one_vb),vertex_two=edge.bind_vertex(two_vb);
        one_vb->Release_Ref(); two_vb->Release_Ref();
        bool missing_source_uv=false;
        try { (void)edge.prepare_applied_state(DX8_FVF_XYZN); }
        catch (const std::runtime_error& error) {
            missing_source_uv=std::string(error.what()).find("source UV")!=std::string::npos;
        }
        check(missing_source_uv,"original N0 missing UV did not fail closed for texture stage");
        const auto one_pixel=draw_lit("source retail normal+UV1 lit one-stage physical probe",
            vertex_one,DX8_FVF_XYZNUV1);
        check(std::abs(one_pixel[0]-source_untextured[0]*240.0F/255.0F)<=6 &&
              std::abs(one_pixel[1]-source_untextured[1]*128.0F/255.0F)<=6 &&
              std::abs(one_pixel[2]-source_untextured[2]*16.0F/255.0F)<=6 &&
              std::abs(one_pixel[3]-source_untextured[3]*128.0F/255.0F)<=6,
              "original normal+UV1 lit stage-one source pixel differs");
        texture_one.Apply(1);
        check(!texture_one.Is_Missing_Texture() && files.owners==0,
            "original lit stage one did not decode owned source pixels");
        shader.Set_Post_Detail_Color_Func(ShaderClass::DETAILCOLOR_ADD);
        DX8Wrapper::Set_Shader(shader);
        DX8Wrapper::Apply_Render_State_Changes();
        const auto two_pixel=draw_lit("source normal+UV2 lit two-stage physical probe",
            vertex_two,DX8_FVF_XYZNUV2);
        check(std::abs(two_pixel[0]-std::min(255,one_pixel[0]+128))<=6 &&
              std::abs(two_pixel[1]-std::min(255,one_pixel[1]+64))<=6 &&
              std::abs(two_pixel[2]-std::min(255,one_pixel[2]+96))<=6 &&
              std::abs(two_pixel[3]-one_pixel[3])<=6,
              "original normal+UV2 lit two-stage source pixel differs");
        shader.Set_Post_Detail_Color_Func(ShaderClass::DETAILCOLOR_DISABLE);
        shader.Set_Fog_Func(ShaderClass::FOG_ENABLE);
        DX8Wrapper::Set_Fog(true,Vector3(1.0F,0,0),0.0F,1.0F);
        DX8Wrapper::Set_Shader(shader);
        DX8Wrapper::Apply_Render_State_Changes();
        const auto lit_fog=draw_lit("original lit view-fogged normal+UV1 source pixel",
            vertex_one,DX8_FVF_XYZNUV1);
        check(std::abs(lit_fog[0]-(one_pixel[0]+255)*0.5F)<=6 &&
              std::abs(lit_fog[1]-one_pixel[1]*0.5F)<=6 &&
              std::abs(lit_fog[2]-one_pixel[2]*0.5F)<=6 &&
              std::abs(lit_fog[3]-one_pixel[3])<=3,
              "original lit source view-space fog/opacity differs");
        shader.Set_Fog_Func(ShaderClass::FOG_DISABLE);
        DX8Wrapper::Set_Fog(false,Vector3(0,0,0),0.0F,1.0F);
        shader.Set_Alpha_Test(ShaderClass::ALPHATEST_ENABLE);
        shader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_SRC_ALPHA);
        shader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA);
        DX8Wrapper::Set_Shader(shader);
        DX8Wrapper::Apply_Render_State_Changes();
        const auto lit_blended=draw_lit("original lit source alpha-test/blend pixel",
            vertex_one,DX8_FVF_XYZNUV1);
        const float coverage=float(one_pixel[3])/255.0F;
        check(std::abs(lit_blended[0]-(coverage*one_pixel[0]+(1.0F-coverage)*5.0F))<=6 &&
              std::abs(lit_blended[1]-(coverage*one_pixel[1]+(1.0F-coverage)*5.0F))<=6,
              "original lit source opacity/blend pixel differs");
        std::cout<<"original-applied-lit-pixels=pass generation="<<generation
                 <<" ambient="<<ambient[0]<<":"<<ambient[1]<<":"<<ambient[2]
                 <<" directional="<<lit[0]<<":"<<lit[1]<<":"<<lit[2]
                 <<" point="<<point_pixel[0]<<":"<<point_pixel[1]<<":"<<point_pixel[2]
                 <<" four="<<four[0]<<":"<<four[1]<<":"<<four[2]
                 <<" normal="<<unnormalized[0]<<":"<<normalized[0]
                 <<" specular="<<specular_off[0]<<":"<<specular_on[0]
                 <<" stages="<<one_pixel[0]<<":"<<two_pixel[0]
                 <<" fog="<<lit_fog[0]<<" blend="<<lit_blended[0]<<'\n';
    }
    device.destroy(depth); device.destroy(color);
    check(device.wait_idle(),device.last_error());
}
}

int main()
{
    try {
        check(SDL_Init(SDL_INIT_VIDEO),"SDL video initialization failed");
        original_unlit_source_pixels(1,160,120);
        original_unlit_source_pixels(2,240,160);
        original_lit_source_pixels(1);
        original_lit_source_pixels(2);
        SDL_Quit();
        return 0;
    } catch (const std::exception& error) {
        SDL_Quit();
        std::cerr<<error.what()<<'\n';
        return 1;
    }
}
