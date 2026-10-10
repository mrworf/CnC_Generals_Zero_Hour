// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/NativeW3DDocument.h"
#include "Common/FileSystem.h"
#include "Common/FileOwner.h"
#include <bit>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string_view>
namespace {
using Data=std::vector<std::uint8_t>;
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void word(Data& data,std::uint32_t value){for(unsigned n=0;n<4;++n)data.push_back(std::uint8_t(value>>(8*n)));}
void real(Data& data,float value){word(data,std::bit_cast<std::uint32_t>(value));}
void text(Data& data,std::string_view value,std::size_t width){require(value.size()<width,"generated fixed name");data.insert(data.end(),value.begin(),value.end());data.resize(data.size()+width-value.size(),0);}
Data join(std::initializer_list<Data> records){Data output;for(const auto& record:records)output.insert(output.end(),record.begin(),record.end());return output;}
Data chunk(std::uint32_t type,const Data& payload,bool nested=false){Data output;word(output,type);word(output,std::uint32_t(payload.size())|(nested?0x80000000:0));output.insert(output.end(),payload.begin(),payload.end());return output;}
void set(Data& data,std::size_t offset,std::uint32_t value){require(offset+4<=data.size(),"generated mutation extent");for(unsigned n=0;n<4;++n)data[offset+n]=std::uint8_t(value>>(8*n));}
Data pivot(std::string_view name,std::uint32_t parent,float x=0){Data p;text(p,name,16);word(p,parent);
  for(float value:{x,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,1.f})real(p,value);require(p.size()==60,"source pivot width");return p;}
Data hierarchy(bool old=false){Data header;word(header,old?0x20000:0x40001);text(header,old?"Legacy":"GeneratedTree",16);word(header,old?2:3);
  for(float n:{4.f,5.f,6.f})real(header,n);require(header.size()==36,"source hierarchy header width");
  auto pivots=old?join({pivot("RootBone",UINT32_MAX,12),pivot("Child",0,4)}):join({pivot("Root",UINT32_MAX,99),pivot("Arm",0,12),pivot("Muzzle",1,4)});
  return chunk(0x100,join({chunk(0x101,header),chunk(0x102,pivots),chunk(0xdead0010,{1,2,3})}),true);
}
Data array(std::uint32_t type,float maximum){Data header;word(header,1);real(header,maximum);Data object;word(object,2);text(object,"Generated.Mesh",32);
  return chunk(type,join({chunk(0x703,header),chunk(0x704,object)}),true);}
Data hlod(){Data header;word(header,0x10000);word(header,2);text(header,"GeneratedModel",16);text(header,"GeneratedTree",16);
  return chunk(0x700,join({chunk(0x701,header),array(0x702,5),array(0x702,std::numeric_limits<float>::max()),array(0x705,0),array(0x706,0)}),true);}
Data mesh(){Data header;word(header,0x40002);word(header,0x12345678);text(header,"Mesh",16);text(header,"Generated",16);
  for(auto value:{1u,3u,1u,0u,0xfffffffdu,77u,23u,0x12u,1u})word(header,value);
  for(float value:{0.f,0.f,0.f,2.f,2.f,2.f,1.f,1.f,1.f,3.f})real(header,value);require(header.size()==116,"source mesh-header width");
  Data positions,normals,bones,triangle;
  for(unsigned n=0;n<3;++n){for(float value:{float(n),0.f,0.f})real(positions,value);for(float value:{0.f,0.f,1.f})real(normals,value);
    bones.push_back(std::uint8_t(n));bones.push_back(0);for(unsigned padding=0;padding<6;++padding)bones.push_back(0xa5);}
  for(auto index:{0u,1u,2u})word(triangle,index);word(triangle,0xf1234567);for(float value:{0.f,0.f,1.f,0.f})real(triangle,value);
  return chunk(0,join({chunk(0x1f,header),chunk(2,positions),chunk(3,normals),chunk(0xe,bones),chunk(0x20,triangle),chunk(0x777,{0xf0,0xa5,0x00,0xff})}),true);
}
Data animation(bool compressed){Data header;word(header,compressed?1:0x40001);text(header,compressed?"Compressed":"Raw",16);text(header,"GeneratedTree",16);word(header,2);word(header,30);
  // Channels are retained opaque in this input-owner phase, not sampled poses.
  return chunk(compressed?0x280:0x200,join({chunk(compressed?0x281:0x201,header),chunk(compressed?0x282:0x202,{1,2,3,4,5,6})}),true);
}
Data generated(){return join({hierarchy(),hierarchy(true),hlod(),mesh(),animation(false),animation(true),chunk(0xcafebeef,{9,8,7})});}
void functional(const Data& input){auto document=NativeW3DDocument::decode(input);
  require(document.bytes().size()==input.size() && std::equal(input.begin(),input.end(),document.bytes().begin()),"entire authored payload preserved");
  require(document.roots().size()==7 && document.hierarchies().size()==2 && document.hlods().size()==1 && document.meshes().size()==1 && document.animations().size()==2,"whole-file typed metadata cohort");
  const auto& tree=document.hierarchies()[0];require(tree.pivots.size()==3 && tree.pivots[2].parent==1 && tree.pivots[0].translation[0]==99 && tree.pivots[1].translation[0]==12,"authored hierarchy index, translation and root metadata retained");
  const auto& old=document.hierarchies()[1];require(old.pivots.size()==3 && old.pivots[0].name=="RootTransform" && old.pivots[1].parent==0 && old.pivots[2].parent==1,"pre3.0 synthetic root and shifted source parents");
  const auto& model=document.hlods()[0];require(model.levels.size()==2 && model.levels[0].objects[0].bone==2 && model.aggregates.size()==1 && model.proxies.size()==1,"distinct ordered LOD, aggregate and proxy relationships");
  const auto& m=document.meshes()[0];require(m.attributes==0x12345678 && m.sortLevel==-3 && m.futureCount==23 && m.vertexChannels==0x12 && m.vertices==3 && m.triangles==1 && m.sphereRadius==3,"packed mesh metadata and initialized declared backing");
  require(!document.animations()[0].compressed && document.animations()[1].compressed && document.animations()[1].frameRate==30 && document.animations()[1].flavor==0,"raw and compressed header widths");
  const auto* bytes=document.bytes().data();auto moved=std::move(document);require(moved.bytes().data()==bytes && moved.hierarchies()[0].pivots[2].name=="Muzzle","move transfers whole owned byte/metadata cohort");
  Data header;word(header,0x20000);text(header,"EmptyLegacy",16);word(header,0);for(unsigned n=0;n<3;++n)real(header,0);
  auto empty=NativeW3DDocument::decode(chunk(0x100,join({chunk(0x101,header),chunk(0x102,{})}),true));
  require(empty.hierarchies()[0].pivots.size()==1 && empty.hierarchies()[0].pivots[0].name=="RootTransform","legacy empty hierarchy retains source synthetic root");
  auto unused=hierarchy();set(unused,16+24,0x7fc00000);set(unused,8+8+36+8+32,0x7fc00000);
  auto preserved=NativeW3DDocument::decode(unused);require(std::bit_cast<std::uint32_t>(preserved.hierarchies()[0].center[0])==0x7fc00000 &&
    std::bit_cast<std::uint32_t>(preserved.hierarchies()[0].pivots[0].euler[0])==0x7fc00000,"unused hierarchy center/Euler representations are preserved, not newly interpreted");
}
template<class F>void reject(F operation){bool failed=false;try{operation();}catch(ErrorCode){failed=true;}require(failed,"invalid W3D input rejected");}
void negative(const Data& input){auto accepted=NativeW3DDocument::decode(input);const auto* bytes=accepted.bytes().data();const auto* pivots=accepted.hierarchies()[0].pivots.data();
  auto bad=[&](const Data& candidate){const auto live=AllocationFault::live();reject([&]{accepted.replace(candidate);});
    require(accepted.bytes().data()==bytes && accepted.hierarchies()[0].pivots.data()==pivots && AllocationFault::live()==live,"malformed candidate preserves accepted owner/views and retires backing");};
  for(std::size_t length=0;length<8;++length)bad(Data(input.begin(),input.begin()+length));
  auto value=input;set(value,4,0xffffffff);bad(value);
  value=hierarchy();set(value,8+8+20,99);bad(value); // Declared pivot count vs initialized payload.
  value=hierarchy();set(value,8+8+36+8+60+16,1);bad(value); // Child self-parent.
  value=hierarchy();set(value,8+8+36+8+20,0x7fc00000);bad(value); // Defined NaN representation.
  value=hierarchy();for(std::size_t n=0;n<16;++n)value[16+4+n]='X';bad(value);
  value=hlod();set(value,16+4,7);bad(value);
  value=mesh();set(value,16+44,4);bad(value); // Vertex count cannot be a descriptor-only claim.
  value=mesh();set(value,16+112,0x7f800000);bad(value);
  value=animation(true);set(value,16+40,0x0002001e);bad(value);
  value=chunk(0xaabbccdd,{1},true);bad(value); // Incomplete nested header.
  Data nested=chunk(0xeeee,{1});for(unsigned n=0;n<256;++n)nested=chunk(0xffff,nested,true);bad(nested);
  auto tree=hierarchy();for(std::size_t length=1;length<tree.size();++length)bad(Data(tree.begin(),tree.begin()+length));
  accepted.replace(input);require(accepted.hierarchies().size()==2,"corrected accepted-owner retry");
}
void faults(const Data& input){auto accepted=NativeW3DDocument::decode(input);
  AllocationFault::arm(std::numeric_limits<std::size_t>::max());try{accepted.replace(input);}catch(...){AllocationFault::disarm();throw;}
  const auto terminal=AllocationFault::attempts();AllocationFault::disarm();require(terminal>0 && terminal<256,"bounded complete W3D owner census");
  for(std::size_t ordinal=0;ordinal<=terminal;++ordinal){const auto live=AllocationFault::live();const auto* bytes=accepted.bytes().data();const auto* pivots=accepted.hierarchies()[0].pivots.data();bool failed=false;
    AllocationFault::arm(ordinal);try{accepted.replace(input);}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
    const auto hit=AllocationFault::triggered();const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
    require(failed==(ordinal<terminal) && failed==hit && attempts==(failed?ordinal+1:terminal),"all W3D replacement prefixes and exact terminal");
    if(failed)require(accepted.bytes().data()==bytes && accepted.hierarchies()[0].pivots.data()==pivots,"failure preserves accepted borrowed view identity");
    require(AllocationFault::live()==live,"all candidate or replaced owner backing retires exactly");
    accepted.replace(input);require(AllocationFault::live()==live && accepted.meshes().size()==1,"every failure/terminal has same-owner corrected retry");
  }std::cout<<"W3D document prefixes [0,"<<terminal<<"); terminal "<<terminal<<"; every retry\n";
}
void vfs(const Data& input){char pattern[]="/tmp/zh-w3d-input-XXXXXX";auto* path=::mkdtemp(pattern);require(path,"generated W3D mount");
  std::filesystem::path root=path;struct Remove{std::filesystem::path& root;~Remove(){std::error_code ignored;std::filesystem::remove_all(root,ignored);}}remove{root};
  {std::ofstream output(root/"generated.w3d",std::ios::binary);output.write(reinterpret_cast<const char*>(input.data()),input.size());require(bool(output),"generated W3D bytes");}
  FileSystem files;files.mountReadOnly({root.string()});
  {FileCloseOwner file(files.openFile("generated.w3d",File::READ|File::BINARY|File::STREAMING));require(bool(file),"original VFS read owner");
    auto document=NativeW3DDocument::read(*file);require(document.hierarchies().size()==2 && std::equal(input.begin(),input.end(),document.bytes().begin()),"original File consumer decodes unchanged source bytes");}
  reject([&]{FileCloseOwner forbidden(files.openFile("generated.w3d",File::WRITE|File::BINARY));});
  {std::ifstream source(root/"generated.w3d",std::ios::binary);Data retained(input.size());source.read(reinterpret_cast<char*>(retained.data()),retained.size());require(retained==input,"generated source content unchanged");}
}
}
int main(int argc,char** argv){bool initialized=false;try{require(argc==2,"W3D family");initMemoryManager();initialized=true;
  {const auto input=generated();vfs(input);for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();const std::string_view family=argv[1];
    if(family=="functional")functional(input);else if(family=="negative")negative(input);else if(family=="faults")faults(input);else if(family=="vfs")vfs(input);else throw std::runtime_error("unknown W3D family");
    require(AllocationFault::live()==live,"repeated whole W3D document lifetime retires standard ownership");}}
  shutdownMemoryManager();initialized=false;std::cout<<"PASS W3D input owner; model sampling/binding and N2 pending\n";return 0;
 }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode){std::cerr<<"FAIL source error\n";}
 AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
