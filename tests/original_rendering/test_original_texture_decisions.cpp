#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"
#include "bitmaphandler.h"
#include "ww3dformat.h"
#include "TARGA.H"
#include "texture.h"
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

Bytes compressed_fixture(unsigned width,unsigned mip_count) {
    LegacyDDSURFACEDESC2 header{};
    header.Size=sizeof(header); header.Width=header.Height=width;
    header.MipMapCount=mip_count; header.PixelFormat.FourCC=0x31545844;
    unsigned bytes=0;
    for (unsigned i=0;i<mip_count;++i) { const unsigned side=std::max(4u,width>>i); bytes+=side*side/2; }
    Bytes result(4+sizeof(header)+bytes);
    std::memcpy(result.data(),"DDS ",4);
    std::memcpy(result.data()+4,&header,sizeof(header));
    return result;
}
Bytes targa_fixture() {
    TGAHeader header{};
    header.ImageType=TGA_TRUECOLOR; header.Width=2; header.Height=2;
    header.PixelDepth=24; header.ImageDescriptor=0x20;
    Bytes result(sizeof(header)+26,0);
    std::memcpy(result.data(),&header,sizeof(header));
    for (unsigned i=0;i<12;++i) result[sizeof(header)+i]=static_cast<unsigned char>(i);
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
        bool physical=false;
        try { texture.Init(); }
        catch (const std::runtime_error& e) { physical=std::string(e.what()).find("GPU device translation")!=std::string::npos; }
        check(physical && !texture.Is_Initialized());
        check(factory.owners==0 && device.resource_counts().total()==0);
        const auto snapshot=device.snapshot();
        check(!snapshot.empty());
        // The original DDS provider removes two lowest mips, then the
        // original Begin_Load requests one reduction with an 8x8 destination.
        bool reduced=false;
        if (snapshot.find("original TextureLoader selected")!=std::string::npos &&
            snapshot.find("width=8")!=std::string::npos &&
            snapshot.find("reduction=1")!=std::string::npos) reduced=true;
        check(reduced);
        factory.files["owned.dds"]={};
        bool absent=false;
        try { texture.Init(); } catch (const std::runtime_error& e) {
            absent=std::string(e.what()).find("missing or invalid")!=std::string::npos;
        }
        check(absent && factory.owners==0);
        factory.files["owned.dds"]=compressed_fixture(16,4);
        bool retry=false;
        try { texture.Init(); } catch (const std::runtime_error& e) {
            retry=std::string(e.what()).find("GPU device translation")!=std::string::npos;
        }
        check(retry && factory.owners==0);
        TextureClass single_mip("owned-one","owned.tga",MIP_LEVELS_1,
            WW3D_FORMAT_UNKNOWN,true,true);
        bool single_edge=false;
        try { single_mip.Init(); } catch (const std::runtime_error& e) {
            single_edge=std::string(e.what()).find("GPU device translation")!=std::string::npos;
        }
        check(single_edge && factory.owners==0);
        check(device.snapshot().find("width=16 height=16 mips=1 reduction=0")!=std::string::npos);
        factory.files["owned.tga"]=targa_fixture();
        TextureClass uncompressed("owned-tga","owned.tga",MIP_LEVELS_ALL,
            WW3D_FORMAT_UNKNOWN,false,true);
        bool uncompressed_edge=false;
        try { uncompressed.Init(); } catch (const std::runtime_error& e) {
            uncompressed_edge=std::string(e.what()).find("GPU device translation")!=std::string::npos;
        }
        check(uncompressed_edge && !uncompressed.Is_Initialized() && factory.owners==0);
        check(device.snapshot().find("width=2 height=2")!=std::string::npos);
        factory.files["owned.tga"]={1,2,3};
        bool malformed=false;
        try { uncompressed.Init(); } catch (const std::runtime_error&) { malformed=true; }
        check(malformed && factory.owners==0);
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
    std::cout << "original-rendering runtime provider=GeneralsMD WW3D2 texture format bitmap decisions\n";
}
