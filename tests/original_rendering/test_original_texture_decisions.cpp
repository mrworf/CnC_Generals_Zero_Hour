#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"
#include "bitmaphandler.h"
#include "ww3dformat.h"
#include "TARGA.H"
#include "texture.h"
#include "textureloader.h"
#include "assetmgr.h"
#include "chunkio.h"
#include "RAMFILE.H"
#include "w3d_file.h"
#include "ww3d.h"
#include "ddsfile.h"
#include "ffactory.h"
#include "wwfile.h"

#include <array>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
#define check(condition) do { if (!(condition)) throw std::runtime_error("original texture decision invariant: " #condition); } while (false)

using Bytes=std::vector<unsigned char>;
class OwnedFile final : public FileClass {
public:
    OwnedFile(std::string name,Bytes bytes):name_(std::move(name)),bytes_(std::move(bytes)) {}
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
    std::string name_; Bytes bytes_; size_t pos_=0; bool open_=false;
};
class OwnedFactory final : public FileFactoryClass {
public:
    std::map<std::string,Bytes> files; int owners=0;
    FileClass* Get_File(const char* name) override { ++owners; return new OwnedFile(name,files[name]); }
    void Return_File(FileClass* file) override { --owners; delete file; }
};

Bytes compressed_fixture(unsigned width,unsigned mip_count,unsigned fourcc=0x31545844) {
    LegacyDDSURFACEDESC2 header{};
    header.Size=sizeof(header); header.Width=header.Height=width;
    header.MipMapCount=mip_count; header.PixelFormat.FourCC=fourcc;
    unsigned bytes=0;
    const unsigned block_bytes=fourcc==0x31545844 ? 8 : 16;
    for (unsigned i=0;i<mip_count;++i) {
        const unsigned side=std::max(4u,width>>i);
        bytes+=(side/4)*(side/4)*block_bytes;
    }
    Bytes result(4+sizeof(header)+bytes);
    std::memcpy(result.data(),"DDS ",4);
    std::memcpy(result.data()+4,&header,sizeof(header));
    for (unsigned i=0;i<bytes;++i) result[4+sizeof(header)+i]=static_cast<unsigned char>(i+1);
    return result;
}
Bytes targa_fixture(unsigned depth=24) {
    TGAHeader header{};
    header.ImageType=TGA_TRUECOLOR; header.Width=2; header.Height=2;
    header.PixelDepth=depth; header.ImageDescriptor=0x20 | (depth==32 ? 8 : 0);
    Bytes result(sizeof(header)+(depth/8)*4+14,0);
    std::memcpy(result.data(),&header,sizeof(header));
    for (unsigned i=0;i<(depth/8)*4;++i)
        result[sizeof(header)+i]=static_cast<unsigned char>(i);
    return result;
}

void original_loader_decisions() {
    OwnedFactory factory;
    auto* prior=_TheFileFactory;
    _TheFileFactory=&factory;
    try {
        WW3D::Set_Thumbnail_Enabled(false);
        WW3D::Set_Texture_Reduction(1,4);
        factory.files["owned.dds"]=compressed_fixture(16,4);
        { DDSFileClass source("owned.tga",0); check(source.Is_Available() && source.Get_Mip_Level_Count()==2); }
        zh::renderer::RecordingGpuDevice device;
        zh::original_runtime::OriginalGpuEdge edge(device);
        TextureClass texture("owned","owned.tga",MIP_LEVELS_ALL,WW3D_FORMAT_UNKNOWN,true,true);
        texture.Init();
        check(texture.Is_Initialized());
        check(factory.owners==0 && device.resource_counts().textures==1);
        const auto first_handle=edge.texture_handle(&texture);
        check(device.texture_bytes(first_handle,0).size()==32);
        check(device.texture_bytes(first_handle,0).front()==static_cast<unsigned char>(129));
        const auto snapshot=device.snapshot();
        check(!snapshot.empty());
        // The original DDS provider removes two lowest mips, then the
        // original Begin_Load requests one reduction with an 8x8 destination.
        bool reduced=false;
        if (snapshot.find("create_texture")!=std::string::npos &&
            snapshot.find("upload_texture")!=std::string::npos &&
            texture.Get_Width()==8 && texture.Get_Height()==8) reduced=true;
        check(reduced);
        texture.Invalidate();
        check(device.resource_counts().total()==0);
        device.fail_next_texture_create();
        bool create_failure=false;
        try { texture.Init(); } catch (const std::runtime_error& e) {
            create_failure=std::string(e.what()).find("creation failed")!=std::string::npos;
        }
        check(create_failure && !texture.Is_Initialized() && factory.owners==0 &&
            device.resource_counts().total()==0);
        device.fail_next_texture_upload();
        bool upload_failure=false;
        try { texture.Init(); } catch (const std::runtime_error& e) {
            upload_failure=std::string(e.what()).find("mip upload failed")!=std::string::npos;
        }
        check(upload_failure && !texture.Is_Initialized() && factory.owners==0 &&
            device.resource_counts().total()==0);
        factory.files["owned.dds"]=compressed_fixture(16,4);
        factory.files["owned.dds"].resize(128+3);
        bool truncated=false;
        try { texture.Init(); } catch (const std::runtime_error& e) {
            truncated=std::string(e.what()).find("mip source is missing or malformed")!=std::string::npos;
        }
        check(truncated && !texture.Is_Initialized() && factory.owners==0 &&
            device.resource_counts().total()==0);
        factory.files["owned.dds"]=compressed_fixture(16,4);
        texture.Init();
        check(texture.Is_Initialized() && factory.owners==0);
        texture.Invalidate();
        for (unsigned fourcc:{0x33545844u,0x35545844u}) {
            factory.files["owned.dds"]=compressed_fixture(16,4,fourcc);
            texture.Init();
            const auto blocks=device.texture_bytes(edge.texture_handle(&texture),0);
            check(blocks.size()==64 && blocks.front()==1);
            texture.Invalidate();
            check(factory.owners==0 && device.resource_counts().total()==0);
        }
        factory.files["owned.dds"]=compressed_fixture(16,4);
        TextureClass single_mip("owned-one","owned.tga",MIP_LEVELS_1,
            WW3D_FORMAT_UNKNOWN,true,true);
        single_mip.Init();
        check(single_mip.Is_Initialized() && factory.owners==0);
        check(device.texture_bytes(edge.texture_handle(&single_mip),0).size()==128);
        factory.files["owned.tga"]=targa_fixture();
        TextureClass uncompressed("owned-tga","owned.tga",MIP_LEVELS_ALL,
            WW3D_FORMAT_UNKNOWN,false,true);
        uncompressed.Init();
        check(uncompressed.Is_Initialized() && factory.owners==0);
        const auto pixels=device.texture_bytes(edge.texture_handle(&uncompressed),0);
        const Bytes expected{0,1,2,255,3,4,5,255,6,7,8,255,9,10,11,255};
        check(pixels==expected);
        const auto mip=device.texture_bytes(edge.texture_handle(&uncompressed),1);
        check(mip==Bytes({3,4,5,255}));
        uncompressed.Invalidate();
        factory.files["owned.tga"]=targa_fixture(32);
        TextureClass rgba("owned-rgba","owned.tga",MIP_LEVELS_ALL,
            WW3D_FORMAT_UNKNOWN,false,true);
        rgba.Init();
        check(rgba.Get_Texture_Format()==WW3D_FORMAT_A8R8G8B8);
        check(device.texture_bytes(edge.texture_handle(&rgba),0)==
            Bytes({0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}));
        rgba.Invalidate();
        factory.files["owned.tga"]={};
        uncompressed.Init();
        check(uncompressed.Is_Missing_Texture() && factory.owners==0);
        const auto missing_handle=edge.texture_handle(&uncompressed);
        const auto missing_pixels=device.texture_bytes(missing_handle,0);
        check(missing_pixels.size()==128*128*4);
        check(Bytes(missing_pixels.begin(),missing_pixels.begin()+4)==Bytes({255,0,255,127}));
        texture.Invalidate();
        factory.files["owned.dds"]={};
        texture.Init();
        check(texture.Is_Missing_Texture() && edge.texture_handle(&texture)==missing_handle);
        texture.Invalidate();
        uncompressed.Invalidate();
        check(device.resource_counts().textures==2); // shared source fallback and single-mip owner
        WW3D::Set_Thumbnail_Enabled(true);
        bool thumbnail=false;
        try { texture.Init(); } catch (const std::runtime_error& e) {
            thumbnail=std::string(e.what()).find("thumbnail")!=std::string::npos;
        }
        check(thumbnail && factory.owners==0);
        WW3D::Set_Thumbnail_Enabled(false);
        WW3D::Set_Texture_Reduction(0,1);
    } catch (...) {
        WW3D::Set_Thumbnail_Enabled(false);
        WW3D::Set_Texture_Reduction(0,1);
        _TheFileFactory=prior; throw;
    }
    _TheFileFactory=prior;
}

void original_texture_generation_lifetime() {
    OwnedFactory factory;
    factory.files["owned.dds"]=compressed_fixture(16,4);
    auto* prior=_TheFileFactory;
    _TheFileFactory=&factory;
    WW3D::Set_Thumbnail_Enabled(false);
    WW3D::Set_Texture_Reduction(0,1);
    try {
        zh::renderer::RecordingGpuDevice device;
        TextureClass texture("owned-generation","owned.tga",MIP_LEVELS_1,
            WW3D_FORMAT_UNKNOWN,true,true);
        zh::renderer::TextureHandle stale;
        std::uint64_t first_generation=0;
        {
            zh::original_runtime::OriginalGpuEdge first(device);
            first_generation=first.generation();
            texture.Init();
            stale=first.texture_handle(&texture);
            check(texture.Is_Initialized() && device.resource_counts().textures==1);
        }
        check(!texture.Is_Initialized() && device.resource_counts().total()==0);
        check(device.texture_bytes(stale).empty());
        {
            zh::original_runtime::OriginalGpuEdge second(device);
            check(second.generation()!=first_generation);
            texture.Init();
            check(second.texture_handle(&texture)!=stale);
            check(device.resource_counts().textures==1 && factory.owners==0);
        }
        check(!texture.Is_Initialized() && device.resource_counts().total()==0);
    } catch (...) { _TheFileFactory=prior; throw; }
    _TheFileFactory=prior;
}

void original_texture_device_fallback() {
    OwnedFactory factory;
    factory.files["owned.dds"]=compressed_fixture(16,4);
    auto* prior=_TheFileFactory;
    _TheFileFactory=&factory;
    WW3D::Set_Thumbnail_Enabled(false);
    WW3D::Set_Texture_Reduction(1,4);
    try {
        zh::renderer::RecordingGpuDevice device;
        zh::original_runtime::OriginalGpuEdge edge(device);
        TextureClass texture("owned-fallback","owned.tga",MIP_LEVELS_ALL,
            WW3D_FORMAT_UNKNOWN,true,true);
        device.set_texture_format_supported(zh::renderer::TextureFormat::bc1,false);
        texture.Init();
        check(texture.Get_Texture_Format()==WW3D_FORMAT_DXT2);
        const auto bc2=device.texture_bytes(edge.texture_handle(&texture),0);
        check(bc2.size()==64 && bc2[0]==255 && bc2[7]==255 && bc2[8]==129);
        texture.Invalidate();
        device.set_texture_format_supported(zh::renderer::TextureFormat::bc2,false);
        device.set_texture_format_supported(zh::renderer::TextureFormat::bc3,false);
        texture.Init();
        check(texture.Get_Texture_Format()==WW3D_FORMAT_X8R8G8B8);
        check(device.texture_bytes(edge.texture_handle(&texture),0).size()==8*8*4);
        texture.Invalidate();
        device.set_texture_format_supported(zh::renderer::TextureFormat::bgra8,false);
        bool unsupported=false;
        try { texture.Init(); } catch (const std::runtime_error&) { unsupported=true; }
        check(unsupported && !texture.Is_Initialized() && device.resource_counts().total()==0);
        device.set_texture_format_supported(zh::renderer::TextureFormat::bgra8,true);
        texture.Init();
        check(texture.Is_Initialized() && factory.owners==0);
    } catch (...) { WW3D::Set_Texture_Reduction(0,1); _TheFileFactory=prior; throw; }
    WW3D::Set_Texture_Reduction(0,1);
    _TheFileFactory=prior;
}

void original_dds_to_targa_fallback() {
    OwnedFactory factory;
    factory.files["owned.dds"]=compressed_fixture(16,4);
    factory.files["owned.dds"][0]='!'; // rejected compressed source
    factory.files["owned.tga"]=targa_fixture();
    auto* prior=_TheFileFactory;
    _TheFileFactory=&factory;
    WW3D::Set_Thumbnail_Enabled(false);
    WW3D::Set_Texture_Reduction(0,1);
    try {
        zh::renderer::RecordingGpuDevice device;
        zh::original_runtime::OriginalGpuEdge edge(device);
        TextureClass texture("owned-source-order","owned.tga",MIP_LEVELS_ALL,
            WW3D_FORMAT_UNKNOWN,true,true);
        texture.Init();
        check(texture.Is_Initialized() && !texture.Is_Missing_Texture());
        check(texture.Get_Texture_Format()==WW3D_FORMAT_X8R8G8B8);
        check(device.texture_bytes(edge.texture_handle(&texture),0)==
            Bytes({0,1,2,255,3,4,5,255,6,7,8,255,9,10,11,255}));
        check(factory.owners==0);
    } catch (...) { _TheFileFactory=prior; throw; }
    _TheFileFactory=prior;
}

void original_stage_filter_state() {
    OwnedFactory factory;
    factory.files["stage.dds"]=compressed_fixture(16,4);
    auto* prior=_TheFileFactory;
    _TheFileFactory=&factory;
    WW3D::Set_Thumbnail_Enabled(false);
    WW3D::Set_Texture_Reduction(0,1);
    WW3D::Enable_Texturing(true);
    try {
        zh::renderer::RecordingGpuDevice device;
        zh::original_runtime::OriginalGpuEdge edge(device);
        WW3D::Set_Texture_Filter(TextureFilterClass::TEXTURE_FILTER_TRILINEAR);
        TextureClass texture("stage","stage.tga",MIP_LEVELS_ALL,WW3D_FORMAT_UNKNOWN,true,true);
        TextureClass orphan("orphan","stage.tga",MIP_LEVELS_ALL,WW3D_FORMAT_UNKNOWN,true,true);
        bool no_owner=false;
        try { edge.select_texture(0,&orphan); } catch (const std::runtime_error&) { no_owner=true; }
        check(no_owner);
        texture.Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        texture.Apply(0);
        const auto zero=edge.pending_stage(0);
        check(zero.texture==edge.texture_handle(&texture) && zero.sampler);
        const auto filter=device.sampler_descriptor(zero.sampler);
        check(filter.min_filter==zh::renderer::Filter::linear &&
            filter.mag_filter==zh::renderer::Filter::linear &&
            filter.mip_filter==zh::renderer::Filter::linear &&
            filter.address_u==zh::renderer::AddressMode::clamp_edge &&
            filter.address_v==zh::renderer::AddressMode::repeat);
        texture.Get_Filter().Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_NONE);
        texture.Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
        texture.Apply(1);
        const auto one=edge.pending_stage(1);
        check(one.texture==zero.texture && one.sampler!=zero.sampler);
        check(device.sampler_descriptor(one.sampler).maximum_lod==0.0F);
        WW3D::Enable_Texturing(false);
        texture.Apply(0);
        check(!edge.pending_stage(0).texture && edge.pending_stage(0).sampler);
        WW3D::Enable_Texturing(true);
        device.fail_next_sampler_create();
        bool injected=false;
        try { texture.Apply(0); } catch (const std::runtime_error& e) {
            injected=std::string(e.what()).find("sampler creation failed")!=std::string::npos;
        }
        check(injected);
        texture.Apply(0);
        check(edge.pending_stage(0).texture==edge.texture_handle(&texture));
        texture.Get_Filter().Set_U_Addr_Mode(static_cast<TextureFilterClass::TxtAddrMode>(9));
        bool unsupported=false;
        try { texture.Apply(0); } catch (const std::runtime_error&) { unsupported=true; }
        check(unsupported);
        texture.Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_REPEAT);
        bool stage_out_of_bounds=false;
        try { texture.Apply(8); } catch (const std::runtime_error&) { stage_out_of_bounds=true; }
        check(stage_out_of_bounds);
        texture.Invalidate();
        bool released_stage=false;
        try { (void)edge.pending_stage(0); }
        catch (const std::runtime_error&) { released_stage=true; }
        check(released_stage);
        released_stage=false;
        try { (void)edge.pending_stage(1); }
        catch (const std::runtime_error&) { released_stage=true; }
        check(released_stage);
        check(device.resource_counts().textures==0 && !device.pass_active());
        const auto snapshot=device.snapshot();
        auto prior_state=snapshot.find("original TextureClass::Apply stage=0 selected");
        check(prior_state!=std::string::npos);
        for (unsigned state=0;state<5;++state) {
            const auto next=snapshot.find("original TextureFilterClass stage=0 property="+
                std::to_string(state),prior_state);
            check(next!=std::string::npos && next>prior_state);
            prior_state=next;
        }
        check(snapshot.find("begin_pass")==std::string::npos &&
            snapshot.find("draw ")==std::string::npos);
        WW3D::Set_Texture_Filter(TextureFilterClass::TEXTURE_FILTER_ANISOTROPIC);
        texture.Apply(0);
        texture.Apply(1);
        check(device.sampler_descriptor(edge.pending_stage(0).sampler).maximum_anisotropy==2);
        check(device.sampler_descriptor(edge.pending_stage(1).sampler).maximum_anisotropy==1);
        texture.Invalidate();
        bool unavailable=false;
        try { TextureFilterClass::_Init_Filters(static_cast<TextureFilterClass::TextureFilterMode>(99)); }
        catch (const std::runtime_error&) { unavailable=true; }
        check(unavailable);
        WW3D::Set_Texture_Filter(TextureFilterClass::TEXTURE_FILTER_BILINEAR);
        TextureClass optional("optional","absent.tga",MIP_LEVELS_ALL,WW3D_FORMAT_UNKNOWN,false,true);
        optional.Apply(0);
        check(optional.Is_Missing_Texture() &&
            edge.pending_stage(0).texture==edge.texture_handle(&optional));
        optional.Invalidate();
        released_stage=false;
        try { (void)edge.pending_stage(0); }
        catch (const std::runtime_error&) { released_stage=true; }
        check(released_stage && device.resource_counts().textures==1);
    } catch (...) {
        WW3D::Enable_Texturing(true);
        _TheFileFactory=prior; throw;
    }
    _TheFileFactory=prior;
}

void original_w3d_texture_stage() {
    OwnedFactory factory;
    factory.files["owned-stage.tga"]=targa_fixture();
    auto* prior=_TheFileFactory;
    _TheFileFactory=&factory;
    WW3D::Set_Thumbnail_Enabled(false);
    WW3D::Set_Texture_Filter(TextureFilterClass::TEXTURE_FILTER_BILINEAR);
    try {
        zh::renderer::RecordingGpuDevice device;
        {
            zh::original_runtime::OriginalGpuEdge edge(device);
            WW3DAssetManager manager;
            std::array<char,512> storage{};
            RAMFileClass output(storage.data(),storage.size());
            check(output.Open(FileClass::WRITE));
            ChunkSaveClass writer(&output);
            check(writer.Begin_Chunk(W3D_CHUNK_TEXTURE));
            constexpr char name[]="owned-stage.tga";
            check(writer.Begin_Chunk(W3D_CHUNK_TEXTURE_NAME));
            check(writer.Write(name,sizeof(name))==sizeof(name));
            check(writer.End_Chunk());
            W3dTextureInfoStruct info{};
            info.Attributes=W3DTEXTURE_NO_LOD | W3DTEXTURE_CLAMP_U;
            check(writer.Begin_Chunk(W3D_CHUNK_TEXTURE_INFO));
            check(writer.Write(&info,sizeof(info))==sizeof(info));
            check(writer.End_Chunk() && writer.End_Chunk());
            const auto size=output.Size();
            output.Close();
            RAMFileClass input(storage.data(),size);
            check(input.Open(FileClass::READ));
            ChunkLoadClass loader(&input);
            TextureClass* source=Load_Texture(loader);
            check(source!=nullptr);
            check(source->Get_Filter().Get_U_Addr_Mode()==TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
            check(source->Get_Filter().Get_Mip_Mapping()==TextureFilterClass::FILTER_TYPE_NONE);
            source->Apply(1);
            auto selected=edge.pending_stage(1);
            check(selected.texture==edge.texture_handle(source));
            const auto sampler=device.sampler_descriptor(selected.sampler);
            check(sampler.address_u==zh::renderer::AddressMode::clamp_edge &&
                sampler.maximum_lod==0.0F);
            source->Release_Ref();
            check(factory.owners==0);
        }
        check(device.resource_counts().total()==0);
    } catch (...) { _TheFileFactory=prior; throw; }
    _TheFileFactory=prior;
}

void format_fallbacks()
{
    zh::renderer::RecordingGpuDevice device;
    Targa source;
    source.Header.PixelDepth=24;
    source.Header.ImageType=TGA_TRUECOLOR;
    WW3DFormat inferred=WW3D_FORMAT_UNKNOWN;
    unsigned bpp=0;
    Get_WW3D_Format(inferred,bpp,source);
    check(inferred==WW3D_FORMAT_R8G8B8 && bpp==3);
    bool no_device=false;
    try { (void)Get_Valid_Texture_Format(inferred,false); }
    catch (const std::runtime_error&) { no_device=true; }
    check(no_device);
    {
        zh::original_runtime::OriginalGpuEdge edge(device);
        WW3DFormat dest=WW3D_FORMAT_UNKNOWN;
        Get_WW3D_Format(dest,inferred,bpp,source);
        check(inferred==WW3D_FORMAT_R8G8B8 && dest==WW3D_FORMAT_X8R8G8B8);
        check(Get_Valid_Texture_Format(WW3D_FORMAT_DXT1,true)==WW3D_FORMAT_DXT1);
        device.set_texture_format_supported(zh::renderer::TextureFormat::bc1,false);
        check(Get_Valid_Texture_Format(WW3D_FORMAT_DXT1,true)==WW3D_FORMAT_DXT2);
        device.set_texture_format_supported(zh::renderer::TextureFormat::bc2,false);
        device.set_texture_format_supported(zh::renderer::TextureFormat::bc3,false);
        check(Get_Valid_Texture_Format(WW3D_FORMAT_DXT1,true)==WW3D_FORMAT_X8R8G8B8);
        device.set_texture_format_supported(zh::renderer::TextureFormat::bgra8,false);
        bool unsupported=false;
        try { (void)Get_Valid_Texture_Format(WW3D_FORMAT_DXT1,true); }
        catch (const std::runtime_error&) { unsupported=true; }
        check(unsupported);
    }
    check(device.resource_counts().total()==0);
    unsigned width=0,height=1,depth=1;
    bool invalid_dimension=false;
    try { TextureLoader::Validate_Texture_Size(width,height,depth); }
    catch (const std::runtime_error&) { invalid_dimension=true; }
    check(invalid_dimension);
    width=16385;
    invalid_dimension=false;
    try { TextureLoader::Validate_Texture_Size(width,height,depth); }
    catch (const std::runtime_error&) { invalid_dimension=true; }
    check(invalid_dimension);
}

void original_bitmap_pixels()
{
    constexpr std::array<unsigned char,12> bgr{0,0,255, 0,255,0, 255,0,0, 255,255,255};
    std::array<unsigned char,16> bgra{};
    BitmapHandlerClass::Copy_Image(bgra.data(),2,2,8,WW3D_FORMAT_A8R8G8B8,
        const_cast<unsigned char*>(bgr.data()),2,2,6,WW3D_FORMAT_R8G8B8,nullptr,0,false);
    constexpr std::array<unsigned char,16> expected{0,0,255,255, 0,255,0,255, 255,0,0,255, 255,255,255,255};
    check(bgra==expected);
    std::array<unsigned char,4> mip{};
    BitmapHandlerClass::Create_Mipmap_B8G8R8A8(mip.data(),4,bgra.data(),8,2,2);
    check(mip[3]==252 && mip[0]==126 && mip[1]==126 && mip[2]==126);
}
} // namespace

int main()
{
    format_fallbacks();
    original_bitmap_pixels();
    original_loader_decisions();
    original_texture_generation_lifetime();
    original_texture_device_fallback();
    original_dds_to_targa_fallback();
    original_stage_filter_state();
    original_w3d_texture_stage();
    std::cout << "original-rendering runtime provider=GeneralsMD WW3D2 texture format bitmap decisions\n";
}
