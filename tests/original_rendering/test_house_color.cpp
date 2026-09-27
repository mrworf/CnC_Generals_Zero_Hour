#include "house_color_texture_cpu.h"
#include "original_gpu_edge.h"
#include "zh/original_process.h"
#include "zh/renderer/recording_device.h"
#include "zh/platform/bgfx_device.h"
#include "mesh.h"
#include "proto.h"
#include "meshmdl.h"
#include "matinfo.h"
#include "chunkio.h"
#include "RAMFILE.H"
#include "ffactory.h"
#include "TARGA.H"
#include "w3d_file.h"
#include "texture.h"
#include "ww3d.h"
#include "vertmaterial.h"
#include "mapper.h"
#include "clone_graph.h"
#include "dx8renderer.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "camera.h"
#include "scene.h"
#include "statistics.h"
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <map>
#include <memory>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "Common/GameMemory.h"
#undef min
#undef max

#undef assert
#define assert(value) do { if (!(value)) throw std::runtime_error("generated house-color invariant: " #value); } while(false)

struct HouseColorGeneratedProbeAccess {
    static TextureClass* recolor(W3DAssetManager& manager,TextureClass* source,int color) {
        return manager.Recolor_Texture(source,color);
    }
    static void fault(int ordinal) {
        W3DAssetManager::house_color_fault_ordinal()=ordinal;
        W3DAssetManager::house_color_fault_count()=0;
    }
    static unsigned faults() { return W3DAssetManager::house_color_fault_count(); }
    static const auto& pixels(TextureClass* texture) {
        assert(texture && texture->HouseColorPixels);return *texture->HouseColorPixels;
    }
    static unsigned cache_size(W3DAssetManager& manager) { return manager.TextureHash.Get_Size(); }
    static auto* table(W3DAssetManager& manager) { return manager.TextureHash.Get_Table(); }
    static auto* buckets(W3DAssetManager& manager) { return manager.TextureHash.Get_Hash(); }
    static unsigned access(TextureClass* texture) { return texture->LastAccessed; }
    static int prototype_count(W3DAssetManager& manager) { return manager.Prototypes.Count(); }
    static int prototype_capacity(W3DAssetManager& manager) { return manager.Prototypes.Length(); }
    static int prototype_growth(W3DAssetManager& manager) { return manager.Prototypes.Growth_Step(); }
    static auto* prototype_array(W3DAssetManager& manager) { return &manager.Prototypes[0]; }
    static bool demand(W3DAssetManager& manager) { return manager.WW3D_Load_On_Demand; }
    static void seed_alias(W3DAssetManager& manager,const char* key,TextureClass* texture) {
        manager.TextureHash.Insert(StringClass(key),texture);texture->Add_Ref();
    }
    static void reserve_prototypes(W3DAssetManager& manager,int capacity) {
        assert(manager.Prototypes.Resize(capacity));
    }
    static void remap(zh::original_runtime::HouseColorTexturePixels& pixels,unsigned color,bool palette,bool alpha) {
        W3DAssetManager::remap_house_color_pixels(pixels,color,palette,alpha);
    }
};

namespace {
using Bytes=std::vector<unsigned char>;
using Edge=zh::original_runtime::OriginalGpuEdge;
using Device=zh::renderer::RecordingGpuDevice;
using Pixels=zh::original_runtime::HouseColorTexturePixels;
#include "generated_model_packet.inc"

template<class T> struct Ref {
    T* value;
    explicit Ref(T* p=nullptr):value(p) {}
    ~Ref() { if(value) value->Release_Ref(); }
};
class InputFile final:public FileClass {
    std::string name_;Bytes bytes_;std::size_t pos_=0;bool open_=false;
public:
    InputFile(const char* name,Bytes data):name_(name),bytes_(std::move(data)) {}
    const char* File_Name() const override { return name_.c_str(); }
    const char* Set_Name(const char* name) override { name_=name;return name_.c_str(); }
    int Create() override { return 0; } int Delete() override { return 0; }
    bool Is_Available(int=0) override { return !bytes_.empty(); }
    bool Is_Open() const override { return open_; }
    int Open(const char* name,int rights=READ) override { Set_Name(name);return Open(rights); }
    int Open(int rights=READ) override { pos_=0;return open_=rights==READ&&!bytes_.empty(); }
    int Read(void* out,int count) override {
        if(!open_||count<0)return 0;
        const auto n=std::min<std::size_t>(count,bytes_.size()-pos_);
        std::memcpy(out,bytes_.data()+pos_,n);pos_+=n;return n;
    }
    int Seek(int delta,int origin=SEEK_CUR) override {
        const long long base=origin==SEEK_SET?0:origin==SEEK_END?bytes_.size():pos_;
        const long long next=base+delta;assert(next>=0&&next<=static_cast<long long>(bytes_.size()));
        return pos_=next;
    }
    int Size() override { return bytes_.size(); }
    int Write(const void*,int) override { return 0; }
    void Close() override { open_=false; }
};
class Inputs final:public FileFactoryClass {
public:
    std::map<std::string,Bytes> files;unsigned reads=0;int owners=0;
    FileClass* Get_File(const char* name) override {
        ++reads;++owners;auto found=files.find(name);
        return new InputFile(name,found==files.end()?Bytes{}:found->second);
    }
    void Return_File(FileClass* file) override { --owners;delete file; }
};
struct FactoryOwner {
    Inputs inputs;FileFactoryClass* prior=_TheFileFactory;
    FactoryOwner() { _TheFileFactory=&inputs; }
    ~FactoryOwner() { assert(inputs.owners==0);_TheFileFactory=prior; }
};
Bytes targa(unsigned width,unsigned height) {
    TGAHeader header{};header.ImageType=TGA_TRUECOLOR;header.Width=width;header.Height=height;
    header.PixelDepth=32;header.ImageDescriptor=0x28;
    Bytes result(sizeof(header)+std::size_t(width)*height*4+14,0);
    std::memcpy(result.data(),&header,sizeof(header));
    for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;++x) {
        auto* p=result.data()+sizeof(header)+(y*width+x)*4;
        p[0]=31+x*7;p[1]=80+y*13;p[2]=190-x*5;p[3]=(x+y)%2?255:0;
    }
    return result;
}
void box_controls() {
    Pixels pixels;pixels.format=WW3D_FORMAT_A8R8G8B8;pixels.width=pixels.height=16;
    pixels.pitches={64};pixels.mips={Bytes(1024)};
    for(unsigned p=0;p<256;++p)for(unsigned c=0;c<4;++c)pixels.mips[0][p*4+c]=p;
    const auto exact=pixels.mips[0];
    zh::original_runtime::build_house_color_box_mips(pixels,0);
    assert(pixels.mips.size()==5&&pixels.mips[0]==exact);
    // Entire overlapping Wine D3DX9 native-expectation ramp, not a D3DX8 claim.
    for(unsigned y=0;y<8;++y)for(unsigned x=0;x<8;++x)for(unsigned c=0;c<4;++c)
        assert(pixels.mips[1][(y*8+x)*4+c]==32*y+2*x+9);
    for(unsigned height:{1u,2u,4u}) {
        Pixels chain;chain.format=WW3D_FORMAT_A8R8G8B8;chain.width=1;chain.height=height;
        chain.pitches={4};chain.mips={Bytes(height*4)};
        for(unsigned p=0;p<height;++p) { chain.mips[0][p*4]=p%2?255:254;chain.mips[0][p*4+1]=p%2; }
        zh::original_runtime::build_house_color_box_mips(chain,0);
        assert(chain.mips.back()[0]==(height==1?254:255));
        assert(chain.mips.back()[1]==(height==1?0:1));
    }
    Pixels boundary;boundary.format=WW3D_FORMAT_A8R8G8B8;boundary.width=8192;boundary.height=2048;
    boundary.pitches={boundary.width*4};boundary.mips={Bytes(zh::original_runtime::house_color_byte_limit,0x6d)};
    zh::original_runtime::build_house_color_box_mips(boundary,1);
    assert(boundary.mips.size()==1&&boundary.mips[0].size()==zh::original_runtime::house_color_byte_limit&&
        boundary.mips[0].front()==0x6d&&boundary.mips[0].back()==0x6d);
    const auto* backing=boundary.mips[0].data();bool rejected=false;
    try { zh::original_runtime::build_house_color_box_mips(boundary,0); }
    catch(const std::runtime_error&) { rejected=true; }
    assert(rejected&&boundary.mips[0].data()==backing&&boundary.mips.size()==1&&boundary.pitches[0]==32768);
    for(unsigned count:{15u,16u,~0u}) {
        rejected=false;
        try { zh::original_runtime::build_house_color_box_mips(boundary,count); }
        catch(const std::runtime_error&) { rejected=true; }
        assert(rejected&&boundary.mips[0].data()==backing&&boundary.mips.size()==1);
    }
}
void remap_controls() {
    constexpr std::array<unsigned,16> scale={255,239,223,211,195,174,167,151,135,123,107,91,79,63,47,35};
    for(auto format:{WW3D_FORMAT_A8R8G8B8,WW3D_FORMAT_A4R4G4B4,WW3D_FORMAT_A1R5G5B5,WW3D_FORMAT_R5G6B5}) {
        const unsigned bytes=format==WW3D_FORMAT_A8R8G8B8?4:2;
        Pixels pixels;pixels.format=format;pixels.width=16;pixels.height=2;
        pixels.pitches={16*bytes};pixels.mips={Bytes(32*bytes,0x77)};
        const auto before=pixels.mips[0];HouseColorGeneratedProbeAccess::remap(pixels,0xff0000,true,false);
        assert(!std::memcmp(pixels.mips[0].data()+16*bytes,before.data()+16*bytes,16*bytes));
        for(unsigned p=0;p<16;++p) {
            if(bytes==4) {
                const auto* value=pixels.mips[0].data()+p*4;
                // Native float multiplication/truncation is preserved, not integer-rounded.
                const unsigned red=static_cast<unsigned char>((float(scale[p])*(255.0f/255.0f/255.0f))*255.0f);
                assert(value[0]==0&&value[1]==0&&value[2]==red&&value[3]==255);
            } else {
                unsigned short value;std::memcpy(&value,pixels.mips[0].data()+p*2,2);
                const unsigned red=static_cast<unsigned char>((float(scale[p])*(255.0f/255.0f/255.0f))*255.0f);
                const unsigned expected=format==WW3D_FORMAT_A4R4G4B4?0xf000|((red&0xf0)<<4):
                    format==WW3D_FORMAT_A1R5G5B5?0x8000|((red&0xf8)<<7):(red&0xf8)<<8;
                assert(value==expected);
            }
        }
    }
    for(auto format:{WW3D_FORMAT_A8R8G8B8,WW3D_FORMAT_A4R4G4B4}) {
        const unsigned bytes=format==WW3D_FORMAT_A8R8G8B8?4:2;
        Pixels pixels;pixels.format=format;pixels.width=4;pixels.height=1;
        pixels.pitches={4*bytes};pixels.mips={Bytes(4*bytes)};
        const std::array<unsigned,4> words={0x00ff0000,0xffff0000,0,0x00808080};
        const std::array<unsigned short,4> shorts={0x0f00,0xff00,0,0x0777};
        std::memcpy(pixels.mips[0].data(),bytes==4?static_cast<const void*>(words.data()):shorts.data(),4*bytes);
        HouseColorGeneratedProbeAccess::remap(pixels,0x0000ff00,false,true);
        const std::array<unsigned,4> expected={0xff00ff00,0xffff0000,0xff000000,0xff808080};
        const std::array<unsigned short,4> expected16={0xf0f0,0xff00,0xf000,0xf777};
        assert(!std::memcmp(pixels.mips[0].data(),bytes==4?static_cast<const void*>(expected.data()):expected16.data(),4*bytes));
    }
}
void negative_controls() {
    FactoryOwner provider;
    for(const char* name:{"zhcd.tga","zhcx.tga","zhca.tga"})provider.inputs.files[name]=targa(16,4);
    Device device;
    {
        Edge edge(device);W3DAssetManager manager;
        for(const char* name:{"zhcd.tga","zhcD.tga","zhca.tga","zhcA.tga","zhcx.tga"}) {
            // Owned direct source permits distinct authored selector case without cache normalization.
            const char* path=name[3]=='D'?"zhcd.tga":name[3]=='A'?"zhca.tga":name;
            Ref<TextureClass> source(new TextureClass(name,path,MIP_LEVELS_1,WW3D_FORMAT_A8R8G8B8,false,false));
            Ref<TextureClass> color(HouseColorGeneratedProbeAccess::recolor(manager,source.value,0x00e03040));
            assert(color.value&&color.value->Is_Procedural()&&!color.value->Is_Reducible());
            if(name[3]=='x') {
                const auto input=targa(16,4);const auto& bytes=HouseColorGeneratedProbeAccess::pixels(color.value).mips[0];
                assert(!std::memcmp(bytes.data(),input.data()+sizeof(TGAHeader),bytes.size()));
            }
        }
        for(const char* name:{"abc","zhca-missing.tga","zhcd-short.tga"}) {
            if(std::string(name)=="zhcd-short.tga")provider.inputs.files[name]=targa(8,1);
            Ref<TextureClass> source(new TextureClass(name,name,MIP_LEVELS_1,WW3D_FORMAT_A8R8G8B8,false,false));
            const auto resource=device.resource_counts();const auto* table=HouseColorGeneratedProbeAccess::table(manager);
            bool rejected=false;
            try { Ref<TextureClass> color(HouseColorGeneratedProbeAccess::recolor(manager,source.value,0x00e03041)); }
            catch(const std::runtime_error&) { rejected=true; }
            assert(rejected&&device.resource_counts()==resource&&HouseColorGeneratedProbeAccess::table(manager)==table&&
                !source.value->Is_Initialized()&&provider.inputs.owners==0);
        }
        const std::string too_long(255,'x');
        Ref<TextureClass> long_source(new TextureClass(too_long.c_str(),"zhcx.tga",MIP_LEVELS_1,WW3D_FORMAT_A8R8G8B8,false,false));
        const auto reads=provider.inputs.reads;bool long_rejected=false;
        try { Ref<TextureClass> color(HouseColorGeneratedProbeAccess::recolor(manager,long_source.value,0x00e03042)); }
        catch(const std::runtime_error&) { long_rejected=true; }
        assert(long_rejected&&provider.inputs.reads==reads);
        Ref<TextureClass> procedural(new TextureClass("!",nullptr,MIP_LEVELS_1));
        assert(HouseColorGeneratedProbeAccess::recolor(manager,procedural.value,0x00e03043)==nullptr);
        Ref<TextureClass> invalid(new TextureClass("zhca-invalid.tga","zhca.tga",MIP_LEVELS_1,
            WW3D_FORMAT_A8R8G8B8,false,false));
        invalid.value->Get_Filter().Set_U_Addr_Mode(static_cast<TextureFilterClass::TxtAddrMode>(2));
        bool tuple_rejected=false;
        try { Ref<TextureClass> color(HouseColorGeneratedProbeAccess::recolor(manager,invalid.value,0x00e03044)); }
        catch(const std::runtime_error&) { tuple_rejected=true; }
        assert(tuple_rejected&&provider.inputs.reads==reads);
        Ref<TextureClass> foreign(manager.Get_Texture("#14692400#zhca.tga",MIP_LEVELS_1));
        Ref<TextureClass> source(new TextureClass("zhca.tga","zhca.tga",MIP_LEVELS_1,WW3D_FORMAT_A8R8G8B8,false,false));
        bool duplicate_rejected=false;
        try { Ref<TextureClass> color(HouseColorGeneratedProbeAccess::recolor(manager,source.value,14692400)); }
        catch(const std::runtime_error&) { duplicate_rejected=true; }
        assert(duplicate_rejected&&provider.inputs.reads==reads);
    }
    assert(device.resource_counts()==zh::renderer::ResourceCounts{});
}
void recolor_generation(unsigned generation) {
    FactoryOwner provider;provider.inputs.files["zhca.tga"]=targa(16,4);
    Device device;
    {
        Edge edge(device);W3DAssetManager manager;
        Ref<TextureClass> source(manager.Get_Texture("zhca.tga",MIP_LEVELS_ALL,
            WW3D_FORMAT_A8R8G8B8,false,TextureBaseClass::TEX_REGULAR,false));
        assert(source.value&&!source.value->Is_Initialized());
        const auto access=HouseColorGeneratedProbeAccess::access(source.value);
        const auto resources=device.resource_counts();
        const unsigned size=HouseColorGeneratedProbeAccess::cache_size(manager);
        const auto* table=HouseColorGeneratedProbeAccess::table(manager);
        const auto* buckets=HouseColorGeneratedProbeAccess::buckets(manager);
        unsigned boundaries=0;
        for(int fault=0;fault<128;++fault) {
            HouseColorGeneratedProbeAccess::fault(fault);
            TextureClass* accepted=nullptr;bool rejected=false;
            try { accepted=HouseColorGeneratedProbeAccess::recolor(manager,source.value,0x0080e040); }
            catch(const std::bad_alloc&) { rejected=true; }
            HouseColorGeneratedProbeAccess::fault(-1);
            if(!rejected) { assert(accepted);accepted->Release_Ref();boundaries=fault;break; }
            assert(!source.value->Is_Initialized()&&HouseColorGeneratedProbeAccess::access(source.value)==access);
            assert(HouseColorGeneratedProbeAccess::cache_size(manager)==size&&
                HouseColorGeneratedProbeAccess::table(manager)==table&&
                HouseColorGeneratedProbeAccess::buckets(manager)==buckets&&device.resource_counts()==resources);
            assert(provider.inputs.owners==0);
        }
        assert(boundaries>8);
        Ref<TextureClass> color(HouseColorGeneratedProbeAccess::recolor(manager,source.value,0x0080e040));
        const auto& pixels=HouseColorGeneratedProbeAccess::pixels(color.value);
        assert(pixels.width==16&&pixels.height==4&&pixels.mips.size()==5);
        const auto original=targa(16,4);
        for(unsigned p=0;p<64;++p) {
            assert(pixels.mips[0][p*4+3]==255);
            if(original[sizeof(TGAHeader)+p*4+3]==255)
                assert(!std::memcmp(pixels.mips[0].data()+p*4,original.data()+sizeof(TGAHeader)+p*4,4));
        }
        const auto accepted_resources=device.resource_counts();const auto reads=provider.inputs.reads;
        Ref<TextureClass> alias(HouseColorGeneratedProbeAccess::recolor(manager,source.value,0x0080e040));
        assert(alias.value==color.value&&provider.inputs.reads==reads&&device.resource_counts()==accepted_resources);
        assert(device.texture_bytes(edge.texture_handle(color.value))==pixels.mips[0]);
        color.value->Invalidate();assert(!color.value->Is_Initialized());
        device.fail_next_texture_upload();bool upload_rejected=false;
        try { color.value->Init(); } catch(const std::runtime_error&) { upload_rejected=true; }
        assert(upload_rejected&&!color.value->Is_Initialized()&&provider.inputs.reads==reads);
        color.value->Init();assert(color.value->Is_Initialized()&&provider.inputs.reads==reads);
        assert(device.texture_bytes(edge.texture_handle(color.value))==pixels.mips[0]);
        std::printf("house-color generation=%u boundaries=%u\n",generation,boundaries);
    }
    assert(device.resource_counts()==zh::renderer::ResourceCounts{});
}
void cache_boundary_controls() {
    const int raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
    const int pool=TheMemoryPoolFactory->getLiveAllocationCount();
    {
        FactoryOwner provider;provider.inputs.files["zhca.tga"]=targa(16,4);
        Device device;Edge edge(device);W3DAssetManager manager;
        Ref<TextureClass> source(manager.Get_Texture("zhca.tga",MIP_LEVELS_1,
            WW3D_FORMAT_A8R8G8B8,false,TextureBaseClass::TEX_REGULAR,false));
        for(unsigned slot=1;slot<65536;++slot) {
            char key[48];std::snprintf(key,sizeof(key),"generated-capacity-%u",slot);
            HouseColorGeneratedProbeAccess::seed_alias(manager,key,source.value);
        }
        assert(HouseColorGeneratedProbeAccess::cache_size(manager)==65536);
        for(unsigned extra=0;extra<2;++extra) {
            const auto* table=HouseColorGeneratedProbeAccess::table(manager);
            const auto* buckets=HouseColorGeneratedProbeAccess::buckets(manager);
            const auto size=HouseColorGeneratedProbeAccess::cache_size(manager);
            const int refs=source.value->Num_Refs();const auto reads=provider.inputs.reads;
            const auto operations=device.operation_counts();HouseColorGeneratedProbeAccess::fault(-1);
            bool rejected=false;
            try { Ref<TextureClass> color(HouseColorGeneratedProbeAccess::recolor(manager,source.value,0x00204060)); }
            catch(const std::runtime_error&) { rejected=true; }
            const auto after=device.operation_counts();
            assert(rejected&&HouseColorGeneratedProbeAccess::faults()==0&&
                HouseColorGeneratedProbeAccess::table(manager)==table&&
                HouseColorGeneratedProbeAccess::buckets(manager)==buckets&&
                HouseColorGeneratedProbeAccess::cache_size(manager)==size&&source.value->Num_Refs()==refs&&
                !source.value->Is_Initialized()&&provider.inputs.reads==reads&&provider.inputs.owners==0&&
                after.commands==operations.commands&&device.resource_counts()==zh::renderer::ResourceCounts{});
            if(!extra) HouseColorGeneratedProbeAccess::seed_alias(manager,"generated-capacity-bound-plus-one",source.value);
        }
    }
    assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw&&TheMemoryPoolFactory->getLiveAllocationCount()==pool);
}
class CapacityPrototype final:public PrototypeClass {
    char name[48];
public:
    explicit CapacityPrototype(unsigned ordinal) { std::snprintf(name,sizeof(name),"GENERATED.CAPACITY%u",ordinal); }
    const char* Get_Name() const override { return name; }
    int Get_Class_ID() const override { return RenderObjClass::CLASSID_MESH; }
    RenderObjClass* Create() override { return nullptr; }
    void DeleteSelf() override { delete this; }
};
void prototype_boundary_controls() {
    const int raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
    const int pool=TheMemoryPoolFactory->getLiveAllocationCount();
    {
        FactoryOwner provider;provider.inputs.files["zhca.tga"]=targa(16,4);
        std::vector<char> packet(65536);RAMFileClass output(packet.data(),packet.size());assert(output.Open(FileClass::WRITE));
        ChunkSaveClass writer(&output);
        make_mesh(writer,false,false,false,1,true,false,0,false,false,"TEST",nullptr,0,false,"ZHCA.TGA");
        const int bytes=output.Size();output.Close();
        Device device;Edge edge(device);W3DAssetManager manager;
        RAMFileClass input(packet.data(),bytes);assert(static_cast<WW3DAssetManager&>(manager).Load_3D_Assets(input));
        HouseColorGeneratedProbeAccess::reserve_prototypes(manager,65536);
        unsigned ordinal=0;
        while(HouseColorGeneratedProbeAccess::prototype_count(manager)<65536)
            manager.Add_Prototype(new CapacityPrototype(ordinal++));
        for(unsigned extra=0;extra<2;++extra) {
            const auto* array=HouseColorGeneratedProbeAccess::prototype_array(manager);
            const int count=HouseColorGeneratedProbeAccess::prototype_count(manager);
            const int capacity=HouseColorGeneratedProbeAccess::prototype_capacity(manager);
            const auto* table=HouseColorGeneratedProbeAccess::table(manager);
            const auto reads=provider.inputs.reads;const auto operations=device.operation_counts();
            HouseColorGeneratedProbeAccess::fault(-1);bool rejected=false;
            try { Ref<RenderObjClass> object(manager.Create_Render_Obj("TEST.LITONE01",extra?-2.0f:0.0f,0)); }
            catch(const std::runtime_error&) { rejected=true; }
            assert(rejected&&HouseColorGeneratedProbeAccess::faults()==0&&
                HouseColorGeneratedProbeAccess::prototype_array(manager)==array&&
                HouseColorGeneratedProbeAccess::prototype_count(manager)==count&&
                HouseColorGeneratedProbeAccess::prototype_capacity(manager)==capacity&&
                HouseColorGeneratedProbeAccess::table(manager)==table&&provider.inputs.reads==reads&&
                device.operation_counts().commands==operations.commands&&device.resource_counts()==zh::renderer::ResourceCounts{});
            if(!extra) {
                manager.Remove_Prototype("GENERATED.CAPACITY0");
                Ref<RenderObjClass> retry(manager.Create_Render_Obj("TEST.LITONE01",0,0));
                assert(retry.value&&HouseColorGeneratedProbeAccess::prototype_count(manager)==65536&&
                    HouseColorGeneratedProbeAccess::prototype_capacity(manager)==65536);
                manager.Add_Prototype(new CapacityPrototype(ordinal++));
            }
        }
        manager.Remove_Prototype("GENERATED.CAPACITY1");
        // Capacity remains above the admitted maximum after generic fixture growth.
        // Reconstruct a fresh equivalent owner rather than normalize accepted state.
    }
    assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw&&TheMemoryPoolFactory->getLiveAllocationCount()==pool);
}
void sibling_fault_controls() {
    const int raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
    const int pool=TheMemoryPoolFactory->getLiveAllocationCount();
    {
        FactoryOwner provider;provider.inputs.files["zhca.tga"]=targa(16,4);
        Device device;Edge edge(device);W3DAssetManager manager;
        Ref<TextureClass> source(manager.Get_Texture("zhca.tga",MIP_LEVELS_1,
            WW3D_FORMAT_A8R8G8B8,false,TextureBaseClass::TEX_REGULAR,false));
        Ref<TextureClass> first(HouseColorGeneratedProbeAccess::recolor(manager,source.value,0x00e02040));
        Ref<TextureClass> second(HouseColorGeneratedProbeAccess::recolor(manager,source.value,0x0020e040));
        const auto first_handle=edge.texture_handle(first.value),second_handle=edge.texture_handle(second.value);
        const auto first_bytes=device.texture_bytes(first_handle),second_bytes=device.texture_bytes(second_handle);
        const auto* table=HouseColorGeneratedProbeAccess::table(manager);
        const auto* buckets=HouseColorGeneratedProbeAccess::buckets(manager);
        const auto size=HouseColorGeneratedProbeAccess::cache_size(manager);
        const int source_refs=source.value->Num_Refs(),first_refs=first.value->Num_Refs(),second_refs=second.value->Num_Refs();
        const auto resources=device.resource_counts();unsigned boundaries=0;bool accepted=false;
        for(int ordinal=0;ordinal<128;++ordinal) {
          for(unsigned repeat=0;repeat<2;++repeat) {
            const auto operations=device.operation_counts();
            HouseColorGeneratedProbeAccess::fault(ordinal);TextureClass* result=nullptr;bool rejected=false;
            try { result=HouseColorGeneratedProbeAccess::recolor(manager,source.value,0x004020e0); }
            catch(const std::bad_alloc&) { rejected=true; }
            HouseColorGeneratedProbeAccess::fault(-1);
            if(!rejected) {
                assert(result&&edge.texture_handle(result)!=first_handle&&edge.texture_handle(result)!=second_handle);
                result->Release_Ref();boundaries=ordinal;accepted=true;break;
            }
            assert(HouseColorGeneratedProbeAccess::table(manager)==table&&HouseColorGeneratedProbeAccess::buckets(manager)==buckets&&
                HouseColorGeneratedProbeAccess::cache_size(manager)==size&&source.value->Num_Refs()==source_refs&&
                first.value->Num_Refs()==first_refs&&second.value->Num_Refs()==second_refs&&
                edge.texture_handle(first.value)==first_handle&&edge.texture_handle(second.value)==second_handle&&
                device.texture_bytes(first_handle)==first_bytes&&device.texture_bytes(second_handle)==second_bytes&&
                device.resource_counts()==resources&&provider.inputs.owners==0);
            const auto after=device.operation_counts();const auto creates=after.creates-operations.creates;
            assert(creates<=1&&after.uploads-operations.uploads==creates&&after.commands-operations.commands==3*creates&&
                after.passes==operations.passes&&after.draws==operations.draws&&after.presents==operations.presents&&
                after.failures==operations.failures);
          }
          if(accepted) break;
        }
        assert(accepted&&boundaries>=15);
        const auto reads=provider.inputs.reads;
        Ref<TextureClass> first_alias(HouseColorGeneratedProbeAccess::recolor(manager,source.value,0x00e02040));
        assert(first_alias.value==first.value&&provider.inputs.reads==reads);
    }
    assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw&&TheMemoryPoolFactory->getLiveAllocationCount()==pool);
}
void prototype_generation(unsigned generation) {
    const int entry_raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
    const int entry_pool=TheMemoryPoolFactory->getLiveAllocationCount();
    {
    FactoryOwner provider;provider.inputs.files["zhca.tga"]=targa(16,4);
    std::vector<char> packet(65536);
    RAMFileClass output(packet.data(),packet.size());assert(output.Open(FileClass::WRITE));
    ChunkSaveClass writer(&output);
    make_mesh(writer,false,false,false,1,true,false,0,false,false,"TEST",nullptr,0,false,"ZHCA.TGA");
    make_hierarchy(writer,false);make_hlod(writer,false,false,false,false,"TEST.COLORHLOD","TEST.LITONE01");
    const int size=output.Size();output.Close();
    Device device;
    {
        Edge edge(device);W3DAssetManager manager;
        RAMFileClass input(packet.data(),size);
        assert(static_cast<WW3DAssetManager&>(manager).Load_3D_Assets(input));
        Ref<RenderObjClass> source(manager.Create_Render_Obj("TEST.LITONE01"));
        Ref<MaterialInfoClass> source_material(static_cast<MeshClass*>(source.value)->Get_Material_Info());
        Ref<TextureMapperClass> random(new RandomTextureMapperClass(0,Vector2(1,1),1));
        source_material.value->Peek_Vertex_Material(0)->Set_Mapper(random.value,1);
        HouseColorGeneratedProbeAccess::reserve_prototypes(manager,HouseColorGeneratedProbeAccess::prototype_count(manager));
        const auto cache_size=HouseColorGeneratedProbeAccess::cache_size(manager);
        const auto* cache_table=HouseColorGeneratedProbeAccess::table(manager);
        const auto* cache_buckets=HouseColorGeneratedProbeAccess::buckets(manager);
        const auto* prototype_array=HouseColorGeneratedProbeAccess::prototype_array(manager);
        const int count=HouseColorGeneratedProbeAccess::prototype_count(manager);
        const int capacity=HouseColorGeneratedProbeAccess::prototype_capacity(manager);
        const int growth=HouseColorGeneratedProbeAccess::prototype_growth(manager);
        const bool demand=HouseColorGeneratedProbeAccess::demand(manager);
        const auto resources=device.resource_counts();
        auto* source_model=static_cast<MeshClass*>(source.value)->Peek_Model();
        const int model_refs=source_model->Num_Refs();
        const int material_refs=source_material.value->Num_Refs();
        auto* vertex_material=source_material.value->Peek_Vertex_Material(0);
        const int vertex_refs=vertex_material->Num_Refs();
        const int mapper_refs=random.value->Num_Refs();
        auto* source_texture=source_material.value->Peek_Texture(0);
        const int texture_refs=source_texture->Num_Refs();
        unsigned boundaries=0;
        for(int ordinal=0;ordinal<128;++ordinal) {
            auto random_before=ww3d_clone::capture_mapper_random();
            const auto operations=device.operation_counts();
            HouseColorGeneratedProbeAccess::fault(ordinal);RenderObjClass* result=nullptr;bool failed=false;
            try { result=manager.Create_Render_Obj("TEST.COLORHLOD",1.0f,0x0070d050); }
            catch(const std::bad_alloc&) { failed=true; }
            HouseColorGeneratedProbeAccess::fault(-1);
            if(!failed) { assert(result);result->Release_Ref();boundaries=ordinal;break; }
            assert(HouseColorGeneratedProbeAccess::cache_size(manager)==cache_size&&
                HouseColorGeneratedProbeAccess::table(manager)==cache_table&&
                HouseColorGeneratedProbeAccess::buckets(manager)==cache_buckets&&
                HouseColorGeneratedProbeAccess::prototype_array(manager)==prototype_array&&
                HouseColorGeneratedProbeAccess::prototype_count(manager)==count&&
                HouseColorGeneratedProbeAccess::prototype_capacity(manager)==capacity&&
                HouseColorGeneratedProbeAccess::demand(manager)==demand&&device.resource_counts()==resources);
            assert(provider.inputs.owners==0);
            assert(static_cast<MeshClass*>(source.value)->Peek_Model()==source_model&&
                source_model->Num_Refs()==model_refs&&source_material.value->Num_Refs()==material_refs&&
                source_material.value->Peek_Vertex_Material(0)==vertex_material&&vertex_material->Num_Refs()==vertex_refs&&
                vertex_material->Peek_Mapper(1)==random.value&&random.value->Num_Refs()==mapper_refs&&
                source_material.value->Peek_Texture(0)==source_texture&&source_texture->Num_Refs()==texture_refs);
            auto random_after=ww3d_clone::capture_mapper_random();
            for(int value=0;value<5;++value) assert(random_after()==random_before());
            const auto after=device.operation_counts();
            const auto creates=after.creates-operations.creates;
            assert(creates<=1&&after.uploads-operations.uploads==creates&&
                after.commands-operations.commands==3*creates&&
                after.passes==operations.passes&&after.draws==operations.draws&&
                after.presents==operations.presents&&after.failures==operations.failures);
        }
        assert(boundaries>16);
        Ref<RenderObjClass> result(manager.Create_Render_Obj("TEST.COLORHLOD",1.0f,0x0070d050));
        assert(result.value&&result.value->Get_ObjectColor()==0x0070d050);
        assert(HouseColorGeneratedProbeAccess::prototype_count(manager)==count+1);
        assert(HouseColorGeneratedProbeAccess::prototype_capacity(manager)==capacity+growth&&
            HouseColorGeneratedProbeAccess::prototype_growth(manager)==growth);
        const auto reads=provider.inputs.reads;const auto accepted_resources=device.resource_counts();
        Ref<RenderObjClass> cached(manager.Create_Render_Obj("TEST.COLORHLOD",1.0f,0x0070d050));
        assert(cached.value&&provider.inputs.reads==reads&&device.resource_counts()==accepted_resources);
        Ref<RenderObjClass> child(result.value->Get_Sub_Object(0));
        Ref<MaterialInfoClass> material(static_cast<MeshClass*>(child.value)->Get_Material_Info());
        auto* color=material.value->Peek_Texture(0);
        assert(color&&HouseColorGeneratedProbeAccess::pixels(color).mips.size()==1);
        const auto bytes=HouseColorGeneratedProbeAccess::pixels(color).mips[0];
        color->Invalidate();device.fail_next_texture_upload();bool failed=false;
        try { Ref<RenderObjClass> rejected(manager.Create_Render_Obj("TEST.COLORHLOD",1.0f,0x0070d050)); }
        catch(const std::runtime_error&) { failed=true; }
        assert(failed&&!color->Is_Initialized()&&provider.inputs.reads==reads);
        Ref<RenderObjClass> replay(manager.Create_Render_Obj("TEST.COLORHLOD",1.0f,0x0070d050));
        assert(replay.value&&color->Is_Initialized()&&provider.inputs.reads==reads&&
            device.texture_bytes(edge.texture_handle(color))==bytes);
        std::printf("house-color prototype generation=%u boundaries=%u\n",generation,boundaries);
    }
    assert(device.resource_counts()==zh::renderer::ResourceCounts{});
    }
    assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==entry_raw&&
        TheMemoryPoolFactory->getLiveAllocationCount()==entry_pool);
}
void scale_controls() {
    FactoryOwner provider;
    provider.inputs.files["zhca.tga"]=targa(16,4);
    std::vector<char> packet(65536);
    RAMFileClass output(packet.data(),packet.size());assert(output.Open(FileClass::WRITE));
    ChunkSaveClass writer(&output);
    make_mesh(writer,false,false,false,1,true,false,0,false,false,"TEST",nullptr,0,false,"ZHCA.TGA");
    const int size=output.Size();output.Close();
    Device device;
    {
        W3DAssetManager manager;
        RAMFileClass input(packet.data(),size);
        assert(static_cast<WW3DAssetManager&>(manager).Load_3D_Assets(input));
        Ref<MeshClass> source(static_cast<MeshClass*>(manager.Create_Render_Obj("TEST.LITONE01")));
        Ref<RenderObjClass> colored;
        assert(source.value);
        auto* source_model=source.value->Peek_Model();
        const int vertex_count=source_model->Get_Vertex_Count();
        std::vector<Vector3> vertices(source_model->Get_Vertex_Array(),source_model->Get_Vertex_Array()+vertex_count);
        for(float scale : {0.0f,-2.0f}) {
            Ref<MeshClass> scaled(static_cast<MeshClass*>(manager.Create_Render_Obj("TEST.LITONE01",scale,0)));
            assert(scaled.value&&scaled.value->Peek_Model()!=source_model);
            const auto* actual=scaled.value->Peek_Model()->Get_Vertex_Array();
            for(int i=0;i<vertex_count;++i) {
                assert(actual[i].X==vertices[i].X*scale&&actual[i].Y==vertices[i].Y*scale&&actual[i].Z==vertices[i].Z*scale);
            }
            assert(!std::memcmp(source_model->Get_Vertex_Array(),vertices.data(),vertices.size()*sizeof(Vector3)));
        }
        const auto* table=HouseColorGeneratedProbeAccess::table(manager);
        const auto* buckets=HouseColorGeneratedProbeAccess::buckets(manager);
        const auto cache_size=HouseColorGeneratedProbeAccess::cache_size(manager);
        const auto* prototypes=HouseColorGeneratedProbeAccess::prototype_array(manager);
        const int count=HouseColorGeneratedProbeAccess::prototype_count(manager);
        const int capacity=HouseColorGeneratedProbeAccess::prototype_capacity(manager);
        const bool demand=HouseColorGeneratedProbeAccess::demand(manager);
        const auto resources=device.resource_counts();
        const int model_refs=source_model->Num_Refs();
        const int raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
        const int live=TheMemoryPoolFactory->getLiveAllocationCount();
        const auto reads=provider.inputs.reads;
        for(float scale : {std::numeric_limits<float>::quiet_NaN(),
                          std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()}) {
            HouseColorGeneratedProbeAccess::fault(-1);bool rejected=false;
            try { Ref<RenderObjClass> no(manager.Create_Render_Obj("TEST.LITONE01",scale,0)); }
            catch(const std::runtime_error&) { rejected=true; }
            assert(rejected&&HouseColorGeneratedProbeAccess::faults()==0);
            assert(HouseColorGeneratedProbeAccess::table(manager)==table&&
                HouseColorGeneratedProbeAccess::buckets(manager)==buckets&&
                HouseColorGeneratedProbeAccess::cache_size(manager)==cache_size&&
                HouseColorGeneratedProbeAccess::prototype_array(manager)==prototypes&&
                HouseColorGeneratedProbeAccess::prototype_count(manager)==count&&
                HouseColorGeneratedProbeAccess::prototype_capacity(manager)==capacity&&
                HouseColorGeneratedProbeAccess::demand(manager)==demand&&device.resource_counts()==resources);
            assert(source.value->Peek_Model()==source_model&&source_model->Num_Refs()==model_refs&&
                !std::memcmp(source_model->Get_Vertex_Array(),vertices.data(),vertices.size()*sizeof(Vector3))&&
                provider.inputs.reads==reads&&provider.inputs.owners==0&&
                TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw&&
                TheMemoryPoolFactory->getLiveAllocationCount()==live);
        }
        // Ordinary pre-device model preparation owns no GPU work. Recolor and
        // replacement still require an Edge before any lookup or mutation.
        for(bool replacement : {false,true}) {
            HouseColorGeneratedProbeAccess::fault(-1);bool rejected=false;
            try { Ref<RenderObjClass> no(manager.Create_Render_Obj("TEST.LITONE01",1.0f,
                replacement ? 0 : 0x0070d050,replacement ? "OLD.TGA" : nullptr,
                replacement ? "NEW.TGA" : nullptr)); }
            catch(const std::runtime_error&) { rejected=true; }
            assert(rejected&&HouseColorGeneratedProbeAccess::faults()==0&&
                HouseColorGeneratedProbeAccess::table(manager)==table&&
                HouseColorGeneratedProbeAccess::buckets(manager)==buckets&&
                HouseColorGeneratedProbeAccess::cache_size(manager)==cache_size&&
                HouseColorGeneratedProbeAccess::prototype_array(manager)==prototypes&&
                HouseColorGeneratedProbeAccess::prototype_count(manager)==count&&
                HouseColorGeneratedProbeAccess::prototype_capacity(manager)==capacity&&
                HouseColorGeneratedProbeAccess::demand(manager)==demand&&device.resource_counts()==resources&&
                source.value->Peek_Model()==source_model&&source_model->Num_Refs()==model_refs&&
                provider.inputs.reads==reads&&provider.inputs.owners==0&&
                TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw&&
                TheMemoryPoolFactory->getLiveAllocationCount()==live);
        }
        {
            Edge edge(device);
            zh::renderer::TextureDesc target;target.width=target.height=4;target.render_target=true;
            const auto output=device.create_texture(target,"generated scale target");
            target.format=zh::renderer::TextureFormat::depth24_stencil8;
            const auto depth=device.create_texture(target,"generated scale depth");
            edge.bind_frame_targets(output,depth,4,4);
            edge.begin_source_frame(true,true,0,0,0,1);
            const auto frame_resources=device.resource_counts();
            const int frame_raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
            const int frame_live=TheMemoryPoolFactory->getLiveAllocationCount();
            bool rejected=false;
            try { Ref<RenderObjClass> no(manager.Create_Render_Obj("TEST.LITONE01",0.0f,0)); }
            catch(const std::runtime_error&) { rejected=true; }
            assert(rejected&&!edge.source_buffers_retirable()&&
                HouseColorGeneratedProbeAccess::table(manager)==table&&
                HouseColorGeneratedProbeAccess::buckets(manager)==buckets&&
                HouseColorGeneratedProbeAccess::cache_size(manager)==cache_size&&
                HouseColorGeneratedProbeAccess::prototype_array(manager)==prototypes&&
                HouseColorGeneratedProbeAccess::prototype_count(manager)==count&&
                HouseColorGeneratedProbeAccess::prototype_capacity(manager)==capacity&&
                HouseColorGeneratedProbeAccess::demand(manager)==demand&&device.resource_counts()==frame_resources&&
                source.value->Peek_Model()==source_model&&source_model->Num_Refs()==model_refs&&
                provider.inputs.reads==reads&&provider.inputs.owners==0&&
                TheDynamicMemoryAllocator->getRawUsedBlockCount()==frame_raw&&
                TheMemoryPoolFactory->getLiveAllocationCount()==frame_live);
            edge.end_source_frame(false);
            Ref<RenderObjClass> retry(manager.Create_Render_Obj("TEST.LITONE01",0.0f,0));
            assert(retry.value&&HouseColorGeneratedProbeAccess::prototype_count(manager)==count);
            colored.value=manager.Create_Render_Obj("TEST.LITONE01",1.0f,0x0070d050);
            assert(colored.value);
            device.destroy(depth);device.destroy(output);
        }
        Ref<MaterialInfoClass> colored_material(static_cast<MeshClass*>(colored.value)->Get_Material_Info());
        auto* texture=colored_material.value->Peek_Texture(0);
        assert(texture&&!texture->Is_Initialized());
        char colored_key[128];std::snprintf(colored_key,sizeof(colored_key),"#%d!1!#test.litone01",0x0070d050);
        const auto* resident_table=HouseColorGeneratedProbeAccess::table(manager);
        const auto* resident_buckets=HouseColorGeneratedProbeAccess::buckets(manager);
        const auto resident_size=HouseColorGeneratedProbeAccess::cache_size(manager);
        const auto* resident_prototypes=HouseColorGeneratedProbeAccess::prototype_array(manager);
        const int resident_count=HouseColorGeneratedProbeAccess::prototype_count(manager);
        const int resident_capacity=HouseColorGeneratedProbeAccess::prototype_capacity(manager);
        const int texture_refs=texture->Num_Refs();
        const int resident_raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
        const int resident_live=TheMemoryPoolFactory->getLiveAllocationCount();
        const auto resident_reads=provider.inputs.reads;
        HouseColorGeneratedProbeAccess::fault(-1);bool rejected=false;
        try { Ref<RenderObjClass> no(manager.Create_Render_Obj(colored_key,2.0f,0)); }
        catch(const std::runtime_error&) { rejected=true; }
        assert(rejected&&HouseColorGeneratedProbeAccess::faults()==0&&!texture->Is_Initialized()&&
            texture->Num_Refs()==texture_refs&&device.resource_counts()==zh::renderer::ResourceCounts{}&&
            HouseColorGeneratedProbeAccess::table(manager)==resident_table&&
            HouseColorGeneratedProbeAccess::buckets(manager)==resident_buckets&&
            HouseColorGeneratedProbeAccess::cache_size(manager)==resident_size&&
            HouseColorGeneratedProbeAccess::prototype_array(manager)==resident_prototypes&&
            HouseColorGeneratedProbeAccess::prototype_count(manager)==resident_count&&
            HouseColorGeneratedProbeAccess::prototype_capacity(manager)==resident_capacity&&
            provider.inputs.reads==resident_reads&&provider.inputs.owners==0&&
            TheDynamicMemoryAllocator->getRawUsedBlockCount()==resident_raw&&
            TheMemoryPoolFactory->getLiveAllocationCount()==resident_live);
        {
            Edge edge(device);
            Ref<RenderObjClass> retry(manager.Create_Render_Obj(colored_key,2.0f,0));
            assert(retry.value&&texture->Is_Initialized()&&provider.inputs.reads==resident_reads);
        }
    }
    assert(device.resource_counts()==zh::renderer::ResourceCounts{});
}
void generation_replay() {
    FactoryOwner provider;provider.inputs.files["zhca.tga"]=targa(16,4);
    W3DAssetManager manager;
    Ref<TextureClass> source(manager.Get_Texture("zhca.tga",MIP_LEVELS_ALL,
        WW3D_FORMAT_A8R8G8B8,false,TextureBaseClass::TEX_REGULAR,false));
    Ref<TextureClass> retained;
    std::vector<Bytes> expected;unsigned reads=0;
    for(unsigned generation=0;generation<2;++generation) {
        Device device;
        {
            Edge edge(device);
            if(!retained.value) {
                retained.value=HouseColorGeneratedProbeAccess::recolor(manager,source.value,0x0060d030);
                expected=HouseColorGeneratedProbeAccess::pixels(retained.value).mips;
                reads=provider.inputs.reads;provider.inputs.files.clear();
            } else {
                assert(!retained.value->Is_Initialized());
                device.fail_next_texture_create();bool failed=false;
                try { Ref<TextureClass> no(HouseColorGeneratedProbeAccess::recolor(manager,source.value,0x0060d030)); }
                catch(const std::runtime_error&) { failed=true; }
                assert(failed&&!retained.value->Is_Initialized()&&provider.inputs.reads==reads&&
                    device.resource_counts()==zh::renderer::ResourceCounts{});
                Ref<TextureClass> alias(HouseColorGeneratedProbeAccess::recolor(manager,source.value,0x0060d030));
                assert(alias.value==retained.value&&provider.inputs.reads==reads);
            }
            for(unsigned level=0;level<expected.size();++level)
                assert(device.texture_bytes(edge.texture_handle(retained.value),level)==expected[level]);
            assert(provider.inputs.reads==reads);
        }
        assert(!retained.value->Is_Initialized()&&device.resource_counts()==zh::renderer::ResourceCounts{});
    }
}
void retirement_controls() {
    FactoryOwner provider;provider.inputs.files["zhca.tga"]=targa(16,4);
    Device device;
    {
        Edge edge(device);W3DAssetManager manager;
        Ref<TextureClass> source(manager.Get_Texture("zhca.tga",MIP_LEVELS_1,
            WW3D_FORMAT_A8R8G8B8,false,TextureBaseClass::TEX_REGULAR,false));
        Ref<TextureClass> color(HouseColorGeneratedProbeAccess::recolor(manager,source.value,0x0050c020));
        zh::renderer::TextureDesc target;target.width=target.height=4;target.render_target=true;
        const auto output=device.create_texture(target,"generated retirement target");
        target.format=zh::renderer::TextureFormat::depth24_stencil8;
        const auto depth=device.create_texture(target,"generated retirement depth");
        edge.bind_frame_targets(output,depth,4,4);
        const auto* table=HouseColorGeneratedProbeAccess::table(manager);
        const auto* buckets=HouseColorGeneratedProbeAccess::buckets(manager);
        const auto handle=edge.texture_handle(color.value);
        const auto bytes=device.texture_bytes(handle);
        const auto refs=color.value->Num_Refs();const auto reads=provider.inputs.reads;
        edge.begin_source_frame(true,true,0,0,0,1);
        const auto frame_resources=device.resource_counts();
        auto reject=[&](auto call) {
            bool rejected=false;try { call(); } catch(const std::runtime_error&) { rejected=true; }
            assert(rejected&&HouseColorGeneratedProbeAccess::table(manager)==table&&
                HouseColorGeneratedProbeAccess::buckets(manager)==buckets&&color.value->Num_Refs()==refs&&
                color.value->Is_Initialized()&&edge.texture_handle(color.value)==handle&&
                device.texture_bytes(handle)==bytes&&device.resource_counts()==frame_resources&&provider.inputs.reads==reads);
        };
        reject([&]{color.value->Invalidate();});
        reject([&]{color.value->Set_HSV_Shift(Vector3(1,0,0));});
        assert(color.value->Get_HSV_Shift()==Vector3(0,0,0));
        reject([&]{manager.Free_Assets();});reject([&]{manager.Release_All_Textures();});
        reject([&]{manager.Release_Unused_Textures();});reject([&]{manager.Release_Unused_Assets();});
        DynamicVectorClass<StringClass> exclusions;
        reject([&]{manager.Free_Assets_With_Exclusion_List(exclusions);});
        reject([&]{Ref<TextureClass> other(HouseColorGeneratedProbeAccess::recolor(manager,source.value,0x00c04020));});
        edge.end_source_frame(false);
        manager.Release_All_Textures();assert(color.value->Num_Refs()==refs-1);
        assert(color.value->Is_Initialized()&&device.texture_bytes(handle)==bytes);
        device.destroy(depth);device.destroy(output);
    }
    assert(device.resource_counts()==zh::renderer::ResourceCounts{});
}
void physical_generation(unsigned generation) {
    FactoryOwner provider;provider.inputs.files["zhca.tga"]=targa(16,4);
    std::vector<char> packet(65536);RAMFileClass output(packet.data(),packet.size());
    assert(output.Open(FileClass::WRITE));ChunkSaveClass writer(&output);
    make_mesh(writer,false,false,false,1,true,false,0,false,false,"TEST",nullptr,0,false,"ZHCA.TGA",true);
    const int size=output.Size();output.Close();
    zh::renderer::BgfxOptions options;options.shader_root=ZH_BGFX_SHADER_DIR;
    zh::renderer::BgfxGpuDevice device(options);
    zh::renderer::TextureDesc target;target.width=160;target.height=120;target.render_target=true;
    const auto color=device.create_texture(target,"generated house-color pixels");
    target.format=zh::renderer::TextureFormat::depth24_stencil8;
    const auto depth=device.create_texture(target,"generated house-color depth");assert(color&&depth);
    {
        Edge edge(device);assert(WW3D::Init(nullptr,nullptr,false)==WW3D_ERROR_OK);
        W3DAssetManager manager;RAMFileClass input(packet.data(),size);
        assert(static_cast<WW3DAssetManager&>(manager).Load_3D_Assets(input));
        Ref<RenderObjClass> customized(manager.Create_Render_Obj("TEST.LITONE01",5.0f,0x0040d020));
        auto* mesh=static_cast<MeshClass*>(customized.value);assert(mesh);
        auto* model=mesh->Peek_Model();model->Set_Flag(MeshGeometryClass::SORT,false);
        auto* material=model->Peek_Single_Material();assert(material);
        material->Set_Lighting(false);material->Set_Diffuse_Color_Source(VertexMaterialClass::MATERIAL);
        material->Set_Diffuse(Vector3(1,1,1));material->Set_Mapper(nullptr,0);
        ShaderClass shader;shader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
        shader.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
        model->Set_Single_Shader(shader);
        mesh->Set_Position(Vector3(-2.5f,-2.5f,-10));
        auto* texture=model->Peek_Single_Texture();assert(texture);
        assert(HouseColorGeneratedProbeAccess::pixels(texture).mips.size()==5);
        edge.bind_frame_targets(color,depth,160,120);
        CameraClass camera;camera.Set_Clip_Planes(1,100);
        SimpleSceneClass scene;scene.Add_Render_Object(mesh);
        auto frame=[&] {
            assert(WW3D::Begin_Render(true,true,Vector3(0,0,0),1)==WW3D_ERROR_OK);
            assert(WW3D::Render(&scene,&camera,true,true,Vector3(0,0,0))==WW3D_ERROR_OK);
            assert(WW3D::End_Render(false)==WW3D_ERROR_OK);
            return device.readback_rgba(color);
        };
        const auto accepted=frame();unsigned visible=0;std::vector<std::array<unsigned char,3>> colors;
        for(std::size_t p=0;p<accepted.size();p+=4)
            if(accepted[p]||accepted[p+1]||accepted[p+2]) {
                ++visible;const std::array<unsigned char,3> rgb={accepted[p],accepted[p+1],accepted[p+2]};
                if(std::find(colors.begin(),colors.end(),rgb)==colors.end())colors.push_back(rgb);
            }
        assert(visible>200&&visible<160*120&&colors.size()>4);
        texture->Get_Filter().Set_Min_Filter(TextureFilterClass::FILTER_TYPE_NONE);
        texture->Get_Filter().Set_Mag_Filter(TextureFilterClass::FILTER_TYPE_NONE);
        texture->Get_Filter().Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_FAST);
        Ref<ScaleTextureMapperClass> minified(new ScaleTextureMapperClass(Vector2(128,128),0));
        material->Set_Mapper(minified.value,0);
        const auto lower=frame();const auto& last=HouseColorGeneratedProbeAccess::pixels(texture).mips.back();
        assert(last.size()==4);unsigned lower_visible=0;
        for(std::size_t p=0;p<lower.size();p+=4)if(lower[p]||lower[p+1]||lower[p+2]) {
            ++lower_visible;
            assert(lower[p]==last[2]&&lower[p+1]==last[1]&&lower[p+2]==last[0]);
        }
        assert(lower_visible==visible);
        material->Set_Mapper(nullptr,0);
        texture->Get_Filter().Set_Min_Filter(TextureFilterClass::FILTER_TYPE_DEFAULT);
        texture->Get_Filter().Set_Mag_Filter(TextureFilterClass::FILTER_TYPE_DEFAULT);
        texture->Get_Filter().Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_DEFAULT);
        assert(frame()==accepted);
        const auto resident=device.live_resource_count();
        HouseColorGeneratedProbeAccess::fault(18);bool rejected=false;
        try { Ref<RenderObjClass> failed(manager.Create_Render_Obj("TEST.LITONE01",5.0f,0x00d04030)); }
        catch(const std::bad_alloc&) { rejected=true; }
        HouseColorGeneratedProbeAccess::fault(-1);
        assert(rejected&&device.live_resource_count()==resident&&frame()==accepted);
        Ref<RenderObjClass> retry(manager.Create_Render_Obj("TEST.LITONE01",5.0f,0x00d04030));assert(retry.value);
        assert(frame()==accepted);
        scene.Remove_Render_Object(mesh);
        TheDX8MeshRenderer.Shutdown();DynamicVBAccessClass::_Deinit();DynamicIBAccessClass::_Deinit();
        assert(WW3D::Shutdown()==WW3D_ERROR_OK);
        Debug_Statistics::Shutdown_Statistics();
        std::printf("house-color physical generation=%u visible=%u distinct=%zu\n",generation,visible,colors.size());
    }
    device.destroy(depth);device.destroy(color);assert(device.wait_idle());assert(device.live_resource_count()==0);
}
}

int main(int argc,char** argv) {
    try {
        initMemoryManager();WW3D::Set_Thumbnail_Enabled(false);
        TextureFilterClass::_Init_Filters(TextureFilterClass::TEXTURE_FILTER_TRILINEAR);
        box_controls();remap_controls();
        // Declare the shipping reset baseline, including its four static defaults.
        DX8Wrapper::Reset_Source_State();
        const int entry_raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
        const int entry_pool=TheMemoryPoolFactory->getLiveAllocationCount();
        struct Services {
            Services() { char category[64]{};assert(zh::original_process::initialize_services(0,category,sizeof(category))); }
            ~Services() { zh::original_process::shutdown_services(); }
        };
        {
        Services services;
        if(argc==1) { negative_controls();cache_boundary_controls();prototype_boundary_controls();sibling_fault_controls();generation_replay();retirement_controls(); }
        for(unsigned generation=0;generation<2;++generation) {
            if(argc>1&&std::string(argv[1])=="--gpu") physical_generation(generation);
            else { scale_controls();recolor_generation(generation);prototype_generation(generation); }
        }
        }
        assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==entry_raw&&
            TheMemoryPoolFactory->getLiveAllocationCount()==entry_pool);
        std::puts("house-color generated CPU controls passed");return 0;
    } catch(const std::exception& error) { std::fprintf(stderr,"house-color category=%s\n",error.what());return 1; }
}
