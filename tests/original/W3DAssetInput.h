// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "AllocationFault.h"
#include "Common/NativeW3DAssets.h"
#include "Common/W3DModelConditionData.h"
#include "Common/NativeDataFile.h"
#include "Common/NativeUserStorage.h"
#include "Common/FileSystem.h"
#include "Common/Errors.h"
#include <algorithm>
#include <bit>
#include <cstring>
#include <cmath>
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <new>
#include <unistd.h>

namespace W3DAssetInput {

using Data=std::vector<std::uint8_t>;
using Assets=NativeW3DAssets;
using Failure=NativeW3DAssetFailure;
class GeneratedFixtureFailure : public std::runtime_error {public: explicit GeneratedFixtureFailure(const char* message):std::runtime_error(message){}};
inline void require(bool value,const char* message){if(!value)throw GeneratedFixtureFailure(message);}
inline void word(Data& out,std::uint32_t value){for(unsigned n=0;n<4;++n)out.push_back(std::uint8_t(value>>(8*n)));}
inline void bigWord(Data& out,std::uint32_t value){for(int n=3;n>=0;--n)out.push_back(std::uint8_t(value>>(8*n)));}
inline void half(Data& out,unsigned value){out.push_back(std::uint8_t(value));out.push_back(std::uint8_t(value>>8));}
inline void real(Data& out,float value){word(out,std::bit_cast<std::uint32_t>(value));}
inline void text(Data& out,std::string_view value,std::size_t width=16){require(value.size()<width,"generated fixed name");out.insert(out.end(),value.begin(),value.end());out.resize(out.size()+width-value.size(),0);}
inline Data join(std::initializer_list<Data> entries){Data out;for(const auto& entry:entries)out.insert(out.end(),entry.begin(),entry.end());return out;}
inline Data chunk(unsigned type,const Data& bytes,bool nested=false){Data out;word(out,type);word(out,std::uint32_t(bytes.size())|(nested?0x80000000:0));out.insert(out.end(),bytes.begin(),bytes.end());return out;}
inline Data tree(std::string_view name="Tree",float base=10){Data header,pivots;word(header,0x40001);text(header,name);word(header,3);for(unsigned n=0;n<3;++n)real(header,0);
  for(unsigned n=0;n<3;++n){text(pivots,n==0?"Root":n==1?"Arm":"Muzzle");word(pivots,n==0?UINT32_MAX:n-1);
    real(pivots,n==0?99:n==1?base:4);for(unsigned i=0;i<8;++i)real(pivots,0);real(pivots,1);}
  return chunk(0x100,join({chunk(0x101,header),chunk(0x102,pivots)}),true);}
inline Data mesh(std::string_view container,std::string_view name,bool skin=false,unsigned lastBone=2,float radius=3){Data header,vertices,triangles,influences;
  word(header,0x40002);word(header,skin?0x00020000:0x1234);text(header,name);text(header,container);
  for(auto value:{1u,3u,0u,0u,0u,0u,0u,skin?0x10u:0u,1u})word(header,value);
  for(float value:{0.f,0.f,0.f,2.f,2.f,2.f,1.f,1.f,1.f,radius})real(header,value);
  for(unsigned n=0;n<3;++n){real(vertices,float(n));real(vertices,0);real(vertices,0);half(influences,n==2?lastBone:n);for(unsigned i=0;i<6;++i)influences.push_back(0xa5);}
  for(auto value:{0u,1u,2u,0xf1234567u})word(triangles,value);for(float value:{0.f,0.f,1.f,0.f})real(triangles,value);
  return chunk(0,join({chunk(0x1f,header),chunk(2,vertices),chunk(0x20,triangles),skin?chunk(0xe,influences):Data{},chunk(0xfefefefe,{9,8,7})}),true);}
inline Data null(std::string_view name){Data out;word(out,0x10000);word(out,0xf1234567);word(out,0xabcdef01);word(out,0x98765432);text(out,name,32);return chunk(0x750,out);}
inline Data objects(unsigned type,const std::vector<std::pair<std::string,unsigned>>& entries,float maximum){Data header,out;word(header,entries.size());real(header,maximum);out=chunk(0x703,header);
  for(const auto& [name,bone]:entries){Data object;word(object,bone);text(object,name,32);auto bytes=chunk(0x704,object);out.insert(out.end(),bytes.begin(),bytes.end());}return chunk(type,out,true);}
inline Data hlod(std::string_view name,std::string_view hierarchy,const std::vector<std::pair<std::string,unsigned>>& entries){Data header;word(header,0x10000);word(header,1);text(header,name);text(header,hierarchy);
  return chunk(0x700,join({chunk(0x701,header),objects(0x702,entries,100)}),true);}
inline Data outer(){Data header;word(header,0x10000);word(header,2);text(header,"Outer");text(header,"Tree");
  return join({chunk(0x700,join({chunk(0x701,header),objects(0x702,{{"Inner",1}},5),objects(0x702,{{"Body.Part",2},{"NULL",0}},std::numeric_limits<float>::max()),
    objects(0x705,{{"Marker.Null",2}},0),objects(0x706,{{"NotAFile",2}},0)}),true),chunk(0x12345678,{1,2,3})});}
inline Data animation(std::string_view hierarchy="Tree",std::string_view name="Walk"){Data header,channel;word(header,0x40001);text(header,name);text(header,hierarchy);word(header,2);word(header,30);
  half(channel,0);half(channel,1);half(channel,1);half(channel,0);half(channel,1);half(channel,0);real(channel,2);real(channel,6);
  return chunk(0x200,join({chunk(0x201,header),chunk(0x202,channel)}),true);}
struct Root {
  char raw[40]="/tmp/zh-assets-input-XXXXXX";
  std::filesystem::path path;
  Root(){require(::mkdtemp(raw),"generated fixture root");try{path=raw;}catch(...){::rmdir(raw);throw;}}
  ~Root(){std::error_code ignored;std::filesystem::remove_all(path,ignored);}
  void write(std::string_view name,const Data& bytes)const{auto file=path/std::string(name);std::filesystem::create_directories(file.parent_path());
    std::ofstream out(file,std::ios::binary|std::ios::trunc);out.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());require(bool(out),"generated asset write");}
};
inline std::vector<std::pair<std::string,Data>> entries(){return {
  {"Art/W3D/Outer.w3d",outer()},{"Art/W3D/Inner.w3d",hlod("Inner","Tree",{{"Body.Part",2}})},
  {"Art/W3D/Body.w3d",mesh("Body","Part",true)},{"Art/W3D/Tree.w3d",tree()},
  {"Art/W3D/Marker.w3d",null("Marker.Null")},{"Art/W3D/Walk.w3d",animation()},
  {"Art/W3D/OtherTree.w3d",tree("OtherTree")},{"Art/W3D/OtherWalk.w3d",animation("OtherTree","OtherWalk")},
  {"Art/W3D/Static.w3d",mesh("Static","Mesh")},{"Art/W3D/Default.w3d",hlod("Default","",{{"Static.Mesh",0}})},
  {"Art/W3D/DupWinner.w3d",join({hlod("DupWinner","",{{"Static.Mesh",0}}),mesh("","DupWinner")})},
  {"Art/W3D/DupMesh.w3d",join({mesh("","DupMesh"),hlod("DupMesh","",{{"Static.Mesh",0}})})},
  {"Art/W3D/DupTree.w3d",join({tree("Tree",123),hlod("DupTree","Tree",{{"Body.Part",2}})})},
  {"Art/W3D/Localized.w3d",mesh("","Localized",false,0,3)},
  {"Data/English/Art/W3D/Localized.w3d",mesh("","Localized",false,0,7)},
  {"Art/W3D/CycleA.w3d",hlod("CycleA","",{{"CycleB",0}})},{"Art/W3D/CycleB.w3d",hlod("CycleB","",{{"CycleA",0}})},
  {"Art/W3D/Broken.w3d",hlod("Broken","Tree",{{"Missing.Part",1}})},
  {"Art/W3D/BadBone.w3d",hlod("BadBone","Tree",{{"Body.Part",3}})},
  {"Art/W3D/BadSkin.w3d",hlod("BadSkin","Tree",{{"SkinBroken.Part",1}})},
  {"Art/W3D/SkinBroken.w3d",mesh("SkinBroken","Part",true,3)},
  {"Art/W3D/BadTree.w3d",hlod("BadTree","AbsentTree",{{"Static.Mesh",0}})},
  {"Art/W3D/WrongName.w3d",mesh("","Unrelated")},{"Art/W3D/Malformed.w3d",{1,2,3}}
};}
inline Data big(const std::vector<std::pair<std::string,Data>>& entries){std::uint32_t offset=16;for(const auto& [name,bytes]:entries){(void)bytes;offset+=9+name.size();}
  Data out{'B','I','G','F'};bigWord(out,0);bigWord(out,entries.size());bigWord(out,offset);
  for(const auto& [name,bytes]:entries){bigWord(out,offset);bigWord(out,bytes.size());out.insert(out.end(),name.begin(),name.end());out.push_back(0);offset+=bytes.size();}
  for(const auto& [name,bytes]:entries){(void)name;out.insert(out.end(),bytes.begin(),bytes.end());}return out;}
struct Context {
  Root root,user;
  FileSystem files;
  std::unique_ptr<NativeUserStorage> storage;
  Context(bool archived=false,const std::vector<std::pair<std::string,Data>>& extra={}){auto data=entries();data.insert(data.end(),extra.begin(),extra.end());if(archived)root.write("scene.big",big(data));else for(const auto& [name,bytes]:data)root.write(name,bytes);
    files.mountReadOnly({root.path.string()});user.write("data/W3D/UserOnly.w3d",mesh("","UserOnly",false,0,11));
    storage=std::make_unique<NativeUserStorage>(NativeUserPaths{(user.path/"data").string(),(user.path/"cache").string()},files);files.attachUserStorage(storage.get());}
  std::string userModels()const{return (user.path/"data/W3D").string();}
};

}
