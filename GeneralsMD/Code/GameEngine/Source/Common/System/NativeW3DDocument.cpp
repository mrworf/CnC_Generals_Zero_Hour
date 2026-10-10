// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/NativeW3DDocument.h"
#include "Common/Errors.h"
#include "Common/file.h"
#include <bit>
#include <cmath>
#include <limits>

namespace {
using Bytes=std::span<const std::uint8_t>;
constexpr std::size_t none=std::numeric_limits<std::size_t>::max();
void valid(bool condition){if(!condition)throw ERROR_CORRUPT_FILE_FORMAT;}
std::uint32_t word(Bytes bytes,std::size_t at){valid(at<=bytes.size() && bytes.size()-at>=4);
  return std::uint32_t(bytes[at])|(std::uint32_t(bytes[at+1])<<8)|(std::uint32_t(bytes[at+2])<<16)|(std::uint32_t(bytes[at+3])<<24);}
std::uint16_t half(Bytes bytes,std::size_t at){valid(at<=bytes.size() && bytes.size()-at>=2);return std::uint16_t(bytes[at])|(std::uint16_t(bytes[at+1])<<8);}
float real(Bytes bytes,std::size_t at){float result=std::bit_cast<float>(word(bytes,at));valid(std::isfinite(result));return result;}
template<std::size_t N>std::array<float,N> floats(Bytes bytes,std::size_t at){std::array<float,N> values{};for(std::size_t n=0;n<N;++n)values[n]=real(bytes,at+4*n);return values;}
template<std::size_t N>std::array<float,N> retainedFloats(Bytes bytes,std::size_t at){std::array<float,N> values{};for(std::size_t n=0;n<N;++n)values[n]=std::bit_cast<float>(word(bytes,at+4*n));return values;}
std::string name(Bytes bytes,std::size_t at,std::size_t width,bool empty=false){valid(at<=bytes.size() && width<=bytes.size()-at);
  std::size_t length=0;while(length<width && bytes[at+length])++length;valid(length<width && (empty || length));
  return {reinterpret_cast<const char*>(bytes.data()+at),length};}
bool container(std::uint32_t type){return type==0 || type==0x100 || type==0x200 || type==0x280 || type==0x700 || type==0x702 || type==0x705 || type==0x706;}
}
NativeW3DDocument NativeW3DDocument::decode(Bytes bytes){valid(!bytes.empty() && bytes.size()<=std::size_t(INT32_MAX));
  NativeW3DDocument candidate;candidate.m_bytes.assign(bytes.begin(),bytes.end());candidate.parse();return candidate;}
NativeW3DDocument NativeW3DDocument::read(File& file){const Int size=file.size();valid(size>0 && file.seek(0,File::START)==0);
  NativeW3DDocument candidate;candidate.m_bytes.resize(std::size_t(size));Int cursor=0;
  while(cursor<size){const Int count=file.read(candidate.m_bytes.data()+cursor,size-cursor);valid(count>0 && count<=size-cursor);cursor+=count;}
  candidate.parse();return candidate;
}
void NativeW3DDocument::replace(Bytes bytes){auto candidate=decode(bytes);swap(candidate);}
void NativeW3DDocument::swap(NativeW3DDocument& other)noexcept{
  m_bytes.swap(other.m_bytes);m_chunks.swap(other.m_chunks);m_roots.swap(other.m_roots);
  m_hierarchies.swap(other.m_hierarchies);m_hlods.swap(other.m_hlods);m_meshes.swap(other.m_meshes);m_animations.swap(other.m_animations);
  m_boxes.swap(other.m_boxes);
}
Bytes NativeW3DDocument::payload(std::size_t index)const{valid(index<m_chunks.size());const auto& chunk=m_chunks[index];return Bytes(m_bytes).subspan(chunk.offset,chunk.size);}
void NativeW3DDocument::parseChunks(std::size_t begin,std::size_t end,unsigned depth,std::size_t parent){
  valid(depth<255);for(auto cursor=begin;cursor<end;){valid(end-cursor>=8);
    const auto type=word(m_bytes,cursor),packed=word(m_bytes,cursor+4);const std::size_t size=packed&0x7fffffff;
    valid(size<=end-cursor-8);const auto index=m_chunks.size();m_chunks.push_back({type,bool(packed&0x80000000),cursor+8,size,{}});
    if(parent==none)m_roots.push_back(index);else m_chunks[parent].children.push_back(index);
    if((packed&0x80000000) || container(type))parseChunks(cursor+8,cursor+8+size,depth+1,index);
    cursor+=8+size;
  }
}
void NativeW3DDocument::parse(){parseChunks(0,m_bytes.size(),0,none);
  for(auto index:m_roots)switch(m_chunks[index].type){case 0:parseMesh(index);break;case 0x100:parseHierarchy(index);break;
    case 0x700:parseHlod(index);break;case 0x740:parseBox(index);break;case 0x200:case 0x280:parseAnimation(index);break;default:break;}
}
void NativeW3DDocument::parseBox(std::size_t index){const auto bytes=payload(index);valid(!m_chunks[index].hasSubchunks && bytes.size()>=68);
  Box box;box.version=word(bytes,0);box.attributes=word(bytes,4);box.name=name(bytes,8,32);
  for(unsigned n=0;n<4;++n)box.color[n]=bytes[40+n];
  box.center=floats<3>(bytes,44);box.extent=floats<3>(bytes,56);
  for(const auto value:box.extent)valid(value>=0);
  m_boxes.push_back(std::move(box));
}
void NativeW3DDocument::parseHierarchy(std::size_t index){const auto& children=m_chunks[index].children;
  valid(children.size()>=2 && m_chunks[children[0]].type==0x101);const auto header=payload(children[0]);valid(header.size()>=36);
  Hierarchy hierarchy;hierarchy.version=word(header,0);hierarchy.name=name(header,4,16);hierarchy.center=retainedFloats<3>(header,24);
  const auto count=word(header,20);const bool old=hierarchy.version<0x30000;std::size_t pivots=none;
  for(auto child:children)if(m_chunks[child].type==0x102){valid(pivots==none);pivots=child;}
  valid(pivots!=none && (count>0 || old));const auto data=payload(pivots);valid(data.size()%60==0 && count==data.size()/60);
  hierarchy.pivots.reserve(std::size_t(count)+old);
  if(old){Pivot root;root.name="RootTransform";root.rotation[3]=1;hierarchy.pivots.push_back(std::move(root));}
  for(std::size_t n=0;n<count;++n){const auto at=n*60;Pivot pivot;pivot.name=name(data,at,16);pivot.parent=word(data,at+16);
    pivot.translation=floats<3>(data,at+20);pivot.euler=retainedFloats<3>(data,at+32);pivot.rotation=floats<4>(data,at+44);
    if(old){valid(pivot.parent==UINT32_MAX || pivot.parent<count);pivot.parent=pivot.parent==UINT32_MAX?0:pivot.parent+1;}
    const auto position=hierarchy.pivots.size();valid(position==0?pivot.parent==UINT32_MAX:pivot.parent<position);
    hierarchy.pivots.push_back(std::move(pivot));
  }
  m_hierarchies.push_back(std::move(hierarchy));
}
void NativeW3DDocument::parseHlod(std::size_t index){const auto& children=m_chunks[index].children;
  valid(!children.empty() && m_chunks[children[0]].type==0x701);auto header=payload(children[0]);valid(header.size()>=40);
  Hlod hlod;hlod.version=word(header,0);const auto count=word(header,4);hlod.name=name(header,8,16);hlod.hierarchy=name(header,24,16,true);
  valid(count<=children.size()-1);hlod.levels.reserve(count);
  auto array=[&](std::size_t child){const auto& entries=m_chunks[child].children;
    valid(!entries.empty() && m_chunks[entries[0]].type==0x703);auto h=payload(entries[0]);valid(h.size()>=8);
    const auto models=word(h,0);valid(models==entries.size()-1);Lod lod;lod.maxScreenSize=real(h,4);lod.objects.reserve(models);
    for(std::size_t n=1;n<entries.size();++n){valid(m_chunks[entries[n]].type==0x704);auto record=payload(entries[n]);valid(record.size()>=36);
      lod.objects.push_back({word(record,0),name(record,4,32)});}
    return lod;
  };
  for(std::size_t n=1;n<=count;++n){valid(m_chunks[children[n]].type==0x702);hlod.levels.push_back(array(children[n]));}
  bool aggregate=false,proxy=false;
  for(std::size_t n=std::size_t(count)+1;n<children.size();++n){auto type=m_chunks[children[n]].type;
    if(type==0x705){valid(!aggregate);aggregate=true;hlod.aggregates=array(children[n]).objects;}
    else if(type==0x706){valid(!proxy);proxy=true;hlod.proxies=array(children[n]).objects;}}
  m_hlods.push_back(std::move(hlod));
}
void NativeW3DDocument::parseMesh(std::size_t index){std::size_t headerIndex=none;
  for(auto child:m_chunks[index].children)if(m_chunks[child].type==0x1f){valid(headerIndex==none);headerIndex=child;}
  valid(headerIndex!=none);auto h=payload(headerIndex);valid(h.size()>=116);Mesh mesh;mesh.chunk=index;
  mesh.version=word(h,0);mesh.attributes=word(h,4);mesh.name=name(h,8,16);mesh.container=name(h,24,16,true);
  mesh.triangles=word(h,40);mesh.vertices=word(h,44);mesh.materials=word(h,48);mesh.damageStages=word(h,52);
  mesh.sortLevel=std::bit_cast<std::int32_t>(word(h,56));mesh.prelitVersion=word(h,60);mesh.futureCount=word(h,64);
  mesh.vertexChannels=word(h,68);mesh.faceChannels=word(h,72);mesh.minimum=floats<3>(h,76);mesh.maximum=floats<3>(h,88);
  mesh.sphereCenter=floats<3>(h,100);mesh.sphereRadius=real(h,112);valid(mesh.sphereRadius>=0);
  auto backing=[&](std::uint32_t type,std::size_t count,std::size_t stride,bool required){std::size_t found=none;
    for(auto child:m_chunks[index].children)if(m_chunks[child].type==type){valid(found==none);found=child;}
    valid(!required || found!=none);if(found==none)return Bytes{};auto bytes=payload(found);valid(bytes.size()%stride==0 && count==bytes.size()/stride);return bytes;};
  auto vertices=backing(2,mesh.vertices,12,mesh.vertices!=0);for(std::size_t at=0;at<vertices.size();at+=12)(void)floats<3>(vertices,at);
  auto normals=backing(3,mesh.vertices,12,bool(mesh.vertexChannels&2));for(std::size_t at=0;at<normals.size();at+=12)(void)floats<3>(normals,at);
  (void)backing(0xe,mesh.vertices,8,bool(mesh.vertexChannels&0x10));
  auto triangles=backing(0x20,mesh.triangles,32,mesh.triangles!=0);
  for(std::size_t at=0;at<triangles.size();at+=32){for(unsigned n=0;n<3;++n)valid(word(triangles,at+4*n)<mesh.vertices);(void)floats<4>(triangles,at+16);}
  m_meshes.push_back(std::move(mesh));
}
void NativeW3DDocument::parseAnimation(std::size_t index){const auto& children=m_chunks[index].children;const bool compressed=m_chunks[index].type==0x280;
  valid(!children.empty() && m_chunks[children[0]].type==(compressed?0x281u:0x201u));auto h=payload(children[0]);valid(h.size()>=44);
  Animation animation;animation.chunk=index;animation.compressed=compressed;animation.version=word(h,0);
  animation.name=name(h,4,16);animation.hierarchy=name(h,20,16);animation.frames=word(h,36);
  animation.frameRate=compressed?half(h,40):word(h,40);animation.flavor=compressed?half(h,42):0;
  valid(animation.frames>0 && animation.frameRate>0 && (!compressed || animation.flavor<2));
  m_animations.push_back(std::move(animation)); // Channel sampling/admission is a later, explicit phase.
}
