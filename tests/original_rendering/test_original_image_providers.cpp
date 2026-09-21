#include "ddsfile.h"
#include "TARGA.H"
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
using Bytes = std::vector<unsigned char>;
#define check(condition) do { if (!(condition)) throw std::runtime_error("original image provider invariant: " #condition); } while (false)

class FixtureFile final : public FileClass {
public:
    FixtureFile(std::string name, Bytes bytes) : name_(std::move(name)), bytes_(std::move(bytes)) {}
    const char* File_Name() const override { return name_.c_str(); }
    const char* Set_Name(const char* name) override { name_=name; return name_.c_str(); }
    int Create() override { return 0; }
    int Delete() override { return 0; }
    bool Is_Available(int = 0) override { return !bytes_.empty(); }
    bool Is_Open() const override { return open_; }
    int Open(const char* name, int rights = READ) override { Set_Name(name); return Open(rights); }
    int Open(int rights = READ) override { open_=!bytes_.empty() && rights==READ; cursor_=0; return open_; }
    int Read(void* data, int count) override {
        if (!open_ || count<0) return 0;
        const auto n=std::min(static_cast<size_t>(count), bytes_.size()-cursor_);
        std::memcpy(data, bytes_.data()+cursor_, n);
        cursor_+=n;
        return static_cast<int>(n);
    }
    int Seek(int offset, int mode = SEEK_CUR) override {
        const auto base=mode==SEEK_SET ? 0LL : mode==SEEK_END ? static_cast<long long>(bytes_.size()) : static_cast<long long>(cursor_);
        const auto next=base+offset;
        if (next<0 || next>static_cast<long long>(bytes_.size()))
            throw std::runtime_error("fixture seek invalid " + std::to_string(offset) + " mode=" + std::to_string(mode));
        cursor_=static_cast<size_t>(next);
        return static_cast<int>(cursor_);
    }
    int Size() override { return static_cast<int>(bytes_.size()); }
    int Write(const void*, int) override { return 0; }
    void Close() override { open_=false; }
private:
    std::string name_;
    Bytes bytes_;
    size_t cursor_=0;
    bool open_=false;
};

class FixtureFactory final : public FileFactoryClass {
public:
    std::map<std::string,Bytes> files;
    int living=0;
    FileClass* Get_File(const char* name) override {
        ++living;
        return new FixtureFile(name, files[name]);
    }
    void Return_File(FileClass* file) override { --living; delete file; }
};

Bytes dds(unsigned fourcc, unsigned size) {
    LegacyDDSURFACEDESC2 header{};
    header.Size=sizeof(header);
    header.Width=header.Height=4;
    header.MipMapCount=1;
    header.PixelFormat.FourCC=fourcc;
    Bytes bytes(4+sizeof(header)+size);
    std::memcpy(bytes.data(), "DDS ", 4);
    std::memcpy(bytes.data()+4, &header, sizeof(header));
    for (unsigned i=0;i<size;++i) bytes[4+sizeof(header)+i]=static_cast<unsigned char>(i+1);
    return bytes;
}

void dds_provider(FixtureFactory& factory) {
    constexpr std::array<unsigned,3> codes{0x31545844,0x33545844,0x35545844};
    constexpr std::array<WW3DFormat,3> formats{WW3D_FORMAT_DXT1,WW3D_FORMAT_DXT3,WW3D_FORMAT_DXT5};
    for (unsigned i=0;i<codes.size();++i) {
        const unsigned size=i ? 16 : 8;
        factory.files["owned.dds"]=dds(codes[i],size);
        DDSFileClass file("owned.tga",0); // The original provider chooses .dds.
        check(file.Is_Available() && file.Get_Format()==formats[i]);
        check(file.Get_Width(0)==4 && file.Get_Height(0)==4 && file.Get_Level_Size(0)==size);
        check(file.Load());
        check(std::memcmp(file.Get_Memory_Pointer(0),factory.files["owned.dds"].data()+128,size)==0);
        check(!file.Load() && factory.living==0);
    }
    factory.files["owned.dds"]={};
    DDSFileClass missing("owned.tga",0);
    check(!missing.Is_Available() && !missing.Load());
    auto invalid=dds(codes[0],8);
    invalid[0]='!';
    factory.files["owned.dds"]=invalid;
    DDSFileClass bad_magic("owned.tga",0);
    check(!bad_magic.Is_Available() && !bad_magic.Load());
    invalid=dds(0x00434241,8);
    factory.files["owned.dds"]=invalid;
    DDSFileClass bad_format("owned.tga",0);
    check(!bad_format.Is_Available() && !bad_format.Load());
    invalid=dds(codes[0],8);
    invalid[4]=0;
    factory.files["owned.dds"]=invalid;
    DDSFileClass bad_header("owned.tga",0);
    check(!bad_header.Is_Available() && !bad_header.Load());
    invalid=dds(codes[0],8);
    invalid.resize(128+3);
    factory.files["owned.dds"]=invalid;
    DDSFileClass truncated("owned.tga",0);
    check(truncated.Is_Available() && !truncated.Load());
    invalid=dds(codes[0],8);
    auto* invalid_header=reinterpret_cast<LegacyDDSURFACEDESC2*>(invalid.data()+4);
    invalid_header->Width=0;
    factory.files["owned.dds"]=invalid;
    DDSFileClass bad_size("owned.tga",0);
    check(!bad_size.Is_Available() && !bad_size.Load());
    invalid_header->Width=4;
    invalid_header->MipMapCount=100;
    factory.files["owned.dds"]=invalid;
    DDSFileClass bad_mips("owned.tga",0);
    check(!bad_mips.Is_Available() && !bad_mips.Load());
    invalid=dds(codes[0],32+3);
    invalid_header=reinterpret_cast<LegacyDDSURFACEDESC2*>(invalid.data()+4);
    invalid_header->Width=invalid_header->Height=8;
    invalid_header->MipMapCount=2;
    factory.files["owned.dds"]=invalid;
    DDSFileClass reduced_short("owned.tga",1);
    check(reduced_short.Is_Available() && !reduced_short.Load());
    invalid=dds(codes[0],32+8);
    invalid_header=reinterpret_cast<LegacyDDSURFACEDESC2*>(invalid.data()+4);
    invalid_header->Width=invalid_header->Height=8;
    invalid_header->MipMapCount=2;
    factory.files["owned.dds"]=invalid;
    DDSFileClass reduced("owned.tga",1);
    check(reduced.Is_Available() && reduced.Get_Width(0)==4 && reduced.Get_Level_Size(0)==8 && reduced.Load());
    check(std::memcmp(reduced.Get_Memory_Pointer(0),invalid.data()+128+32,8)==0);
    check(factory.living==0);
    factory.files["owned.dds"]=dds(codes[0],8);
    DDSFileClass retry("owned.tga",0);
    check(retry.Load() && factory.living==0);
}

void targa_provider(FixtureFactory& factory) {
    TGAHeader header{};
    header.ImageType=TGA_TRUECOLOR;
    header.Width=2;
    header.Height=2;
    header.PixelDepth=24;
    header.ImageDescriptor=0x20;
    Bytes bytes(26+sizeof(header)); // The authored reader probes the optional footer.
    std::memcpy(bytes.data(),&header,sizeof(header));
    constexpr std::array<unsigned char,12> pixels{1,2,3,4,5,6,7,8,9,10,11,12};
    constexpr std::array<unsigned char,12> authored_bottom_origin{7,8,9,10,11,12,1,2,3,4,5,6};
    std::memcpy(bytes.data()+sizeof(header),pixels.data(),pixels.size());
    factory.files["owned.tga"]=bytes;
    { Targa image;
      const auto opened=image.Open("owned.tga",TGA_READMODE);
      if (opened) throw std::runtime_error("original Targa open error " + std::to_string(opened));
      image.Close();
      const auto result=image.Load("owned.tga",TGAF_IMAGE,false);
      if (result) throw std::runtime_error("original Targa load error " + std::to_string(result));
      check(image.Header.Width==2 && image.Header.Height==2);
      check(std::memcmp(image.GetImage(),authored_bottom_origin.data(),pixels.size())==0);
      check((image.Header.ImageDescriptor & TGAIDF_YORIGIN)==0); }
    factory.files["owned.tga"]={};
    { Targa image; check(image.Load("owned.tga",TGAF_IMAGE,false)!=0); }
    bytes.resize(26+sizeof(header));
    bytes[sizeof(header)+1]=0;
    bytes.resize(27); // Even the optional-footer probe must reject this.
    factory.files["owned.tga"]=bytes;
    { Targa image; check(image.Load("owned.tga",TGAF_IMAGE,false)!=0); }
    bytes.assign(26+sizeof(header),0);
    header.Width=0;
    std::memcpy(bytes.data(),&header,sizeof(header));
    factory.files["owned.tga"]=bytes;
    { Targa image; check(image.Open("owned.tga",TGA_READMODE)==TGAERR_SYNTAX); }
    header.Width=2;
    header.ImageType=TGA_TRUECOLOR_ENCODED;
    bytes.assign(26+sizeof(header),0);
    std::memcpy(bytes.data(),&header,sizeof(header));
    bytes[sizeof(header)]=0x84; // Five-pixel RLE packet in a four-pixel image.
    factory.files["owned.tga"]=bytes;
    { Targa image; check(image.Load("owned.tga",TGAF_IMAGE,false)==TGAERR_SYNTAX); }
    header.ImageType=TGA_NOIMAGE;
    bytes.assign(26+sizeof(header),0);
    std::memcpy(bytes.data(),&header,sizeof(header));
    factory.files["owned.tga"]=bytes;
    { Targa image; check(image.Load("owned.tga",TGAF_IMAGE,false)==TGAERR_NOTSUPPORTED); }
    header.ImageType=TGA_TRUECOLOR;
    bytes.assign(26+sizeof(header),0);
    std::memcpy(bytes.data(),&header,sizeof(header));
    std::memcpy(bytes.data()+sizeof(header),pixels.data(),pixels.size());
    factory.files["owned.tga"]=bytes;
    { Targa image; check(image.Load("owned.tga",TGAF_IMAGE,false)==0); }
    check(factory.living==0);
}
} // namespace

int main() {
    FixtureFactory factory;
    auto* previous=_TheFileFactory;
    _TheFileFactory=&factory;
    try { dds_provider(factory); targa_provider(factory); }
    catch (...) { _TheFileFactory=previous; throw; }
    _TheFileFactory=previous;
    check(factory.living==0);
    std::cout << "original-rendering runtime provider=GeneralsMD Targa DDS image graph\n";
}
