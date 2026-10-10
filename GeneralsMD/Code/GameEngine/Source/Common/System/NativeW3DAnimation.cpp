// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/NativeW3DAnimation.h"
#include "Common/Errors.h"
#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>
#include <string_view>

namespace {
using Bytes=std::span<const std::uint8_t>;
constexpr std::uint32_t flag=0x80000000;
constexpr float pi=3.141592654f;
void valid(bool condition){if(!condition)throw ERROR_CORRUPT_FILE_FORMAT;}
std::uint32_t word(Bytes b,std::size_t n){valid(n<=b.size() && b.size()-n>=4);
  return std::uint32_t(b[n])|(std::uint32_t(b[n+1])<<8)|(std::uint32_t(b[n+2])<<16)|(std::uint32_t(b[n+3])<<24);}
unsigned half(Bytes b,std::size_t n){valid(n<=b.size() && b.size()-n>=2);return unsigned(b[n])|(unsigned(b[n+1])<<8);}
float real(Bytes b,std::size_t n){auto value=std::bit_cast<float>(word(b,n));valid(std::isfinite(value));return value;}
bool equalName(std::string_view a,std::string_view b){if(a.size()!=b.size())return false;
  auto lower=[](unsigned char c){return c>='A' && c<='Z'?c+('a'-'A'):c;};
  for(std::size_t n=0;n<a.size();++n)if(lower(a[n])!=lower(b[n]))return false;
  return true;}
float lerp(float a,float b,float t){return a+(b-a)*t;}

// Source WWMath table construction and portable Fast_Slerp, without private
// renderer types or process initialization dependencies. Tables own no heap.
struct MathTables {
  std::array<float,1024> sine{},acosine{};
  std::array<float,256> filter{};
  MathTables(){for(unsigned n=0;n<1024;++n){float angle=float(n)*2.f*pi/1024;
      sine[n]=float(std::sin(double(angle)));float value=float(int(n)-512)*(1.f/512);
      acosine[n]=float(std::acos(double(value)));}
    filter={0.00000001f,0.0000001f,0.000001f,0.00001f,0.0001f,0.001f,0.01f,0.1f,
            1.f,10.f,100.f,1000.f,10000.f,100000.f,1000000.f,10000000.f};
    for(unsigned n=0;n<240;++n){float ratio=float(n)/240;float angle=(90.f*ratio)*pi/180.f;filter[n+16]=1.f-std::sin(angle);}}
};
const MathTables& tables(){static const MathTables value;return value;}
float fastSin(float value){value*=1024.f/(2.f*pi);const int index=int(std::floor(value));float fraction=value-float(index);
  return (1.f-fraction)*tables().sine[unsigned(index)&1023]+fraction*tables().sine[unsigned(index+1)&1023];}
float fastAcos(float value){if(std::fabs(value)>0.975f)return std::acos(value);
  value*=512.f;const int index=int(std::floor(value));float fraction=value-float(index);
  return (1.f-fraction)*tables().acosine[index+512]+fraction*tables().acosine[index+513];}
std::array<float,4> slerp(const std::array<float,4>& a,const std::array<float,4>& b,float t){
  if(t==0)return a;
  if(t==1)return b;
  float dot=a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3];valid(std::isfinite(dot));
  const bool flip=dot<0;if(flip)dot=-dot;float complement;
  if(1.f-dot<0.0001f*0.0001f)complement=1.f-t;
  else {float theta=fastAcos(dot),inverse=1.f/fastSin(theta);complement=fastSin(theta-t*theta)*inverse;t=fastSin(t*theta)*inverse;}
  if(flip)t=-t;
  std::array<float,4> result;for(unsigned n=0;n<4;++n){result[n]=complement*a[n]+t*b[n];valid(std::isfinite(result[n]));}return result;
}
// Original retail x87 frame conversion rounds to nearest/even. Do not inherit
// the truncating non-MSVC fallback or the caller's floating-point round mode.
std::uint32_t rounded(float frame){const double base=std::floor(double(frame)),fraction=double(frame)-base;
  auto result=std::uint32_t(base);if(fraction>0.5 || (fraction==0.5 && (result&1)))++result;return result;}
std::int32_t signedRounded(float frame){
  const double base=std::floor(double(frame)),fraction=double(frame)-base;
  valid(base>=INT32_MIN && base<=INT32_MAX);
  auto result=std::int64_t(base);
  if(fraction>0.5 || (fraction==0.5 && result%2!=0))++result;
  valid(result>=INT32_MIN && result<=INT32_MAX);return std::int32_t(result);
}
}

NativeW3DAnimation NativeW3DAnimation::decode(const NativeW3DDocument& document,std::size_t index,
                                            const NativeW3DDocument::Hierarchy& hierarchy){
  valid(index<document.animations().size() && !hierarchy.pivots.empty());const auto& metadata=document.animations()[index];
  valid(equalName(metadata.hierarchy,hierarchy.name));NativeW3DAnimation candidate;
  candidate.m_name=metadata.hierarchy+"."+metadata.name;candidate.m_hierarchy=metadata.hierarchy;
  candidate.m_frames=metadata.frames;candidate.m_frameRate=metadata.frameRate;candidate.m_compressed=metadata.compressed;
  candidate.m_nodes.resize(hierarchy.pivots.size());
  for(auto child:document.chunks()[metadata.chunk].children){const auto type=document.chunks()[child].type;
    if(type==(metadata.compressed?0x282u:0x202u))candidate.readChannel(document.payload(child),metadata.version,metadata.flavor);
    else if(type==(metadata.compressed?0x283u:0x203u))candidate.readVisibility(document.payload(child),metadata.version);}
  return candidate;
}
void NativeW3DAnimation::replace(const NativeW3DDocument& document,std::size_t index,const NativeW3DDocument::Hierarchy& hierarchy){
  auto candidate=decode(document,index,hierarchy);swap(candidate);
}
void NativeW3DAnimation::swap(NativeW3DAnimation& other)noexcept{m_name.swap(other.m_name);m_hierarchy.swap(other.m_hierarchy);
  std::swap(m_frames,other.m_frames);std::swap(m_frameRate,other.m_frameRate);std::swap(m_compressed,other.m_compressed);m_nodes.swap(other.m_nodes);}

void NativeW3DAnimation::readChannel(Bytes bytes,std::uint32_t version,unsigned flavor){
  Channel channel;unsigned type,pivot,count;std::size_t start;
  if(!m_compressed){valid(bytes.size()>=16);channel.first=half(bytes,0);channel.last=half(bytes,2);
    channel.width=half(bytes,4);type=half(bytes,6);pivot=half(bytes,8)+(version<0x30000);
    valid(channel.first<=channel.last);count=channel.last-channel.first+1;start=12;}
  else {valid(bytes.size()>=12);count=word(bytes,0);pivot=half(bytes,4);channel.width=bytes[6];type=bytes[7];start=8;}
  valid(type<=6 && (channel.width==(type==6?4u:1u)) && count>0);
  channel.present=true;
  if(!m_compressed || flavor==0){const std::size_t stride=4*(channel.width+(m_compressed?1:0));
    valid(count<=(bytes.size()-start)/stride);channel.values.reserve(std::size_t(count)*channel.width);
    channel.sparse=m_compressed;if(channel.sparse)channel.times.reserve(count);
    for(std::size_t n=0;n<count;++n){auto at=start+n*stride;
      if(channel.sparse){auto time=word(bytes,at);
        valid(n==0?(time&~flag)==0:(time&~flag)>(channel.times.back()&~flag));channel.times.push_back(time);at+=4;}
      for(unsigned component=0;component<channel.width;++component)channel.values.push_back(real(bytes,at+4*component));}
  } else {valid(flavor==1 && bytes.size()>=16);const float scale=real(bytes,8);start=12;
    const std::size_t baseBytes=4*channel.width,packets=(std::size_t(count)-1+15)/16;
    valid(baseBytes<=bytes.size()-start && packets<=(bytes.size()-start-baseBytes)/(9*channel.width));
    channel.first=0;channel.last=count-1;channel.values.resize(std::size_t(count)*channel.width);
    for(unsigned component=0;component<channel.width;++component){float value=real(bytes,start+4*component);channel.values[component]=value;
      for(std::size_t frame=1;frame<count;++frame){auto at=start+baseBytes+(((frame-1)/16)*channel.width+component)*9;
        const float filter=tables().filter[bytes[at]]*scale;const auto nibble=(frame-1)%16;
        int factor=(bytes[at+1+nibble/2]>>(nibble%2?4:0))&15;if(factor&8)factor-=16;
        value+=float(factor)*filter;valid(std::isfinite(value));channel.values[frame*channel.width+component]=value;}}
  }
  // Original loaders fully decode then discard channels indexing absent bones.
  // Duplicate valid channels replace their prior owner (fixing the source leak).
  if(pivot<m_nodes.size())m_nodes[pivot].channels[type]=std::move(channel);
}
void NativeW3DAnimation::readVisibility(Bytes bytes,std::uint32_t version){Visibility visibility;unsigned pivot,type,count;
  visibility.present=true;visibility.sparse=m_compressed;
  if(!m_compressed){valid(bytes.size()>=10);visibility.first=half(bytes,0);visibility.last=half(bytes,2);
    type=half(bytes,4);pivot=half(bytes,6)+(version<0x30000);visibility.defaultValue=bytes[8]==1;
    valid(visibility.first<=visibility.last);count=visibility.last-visibility.first+1;const std::size_t size=(count+7)/8;
    valid(bytes.size()==9+size);visibility.bits.assign(bytes.begin()+9,bytes.end());}
  else {valid(bytes.size()>=12);count=word(bytes,0);pivot=half(bytes,4);type=bytes[6];visibility.defaultValue=bytes[7]!=0;
    valid(count>0 && std::size_t(count)==(bytes.size()-8)/4 && (bytes.size()-8)%4==0);visibility.times.reserve(count);
    for(std::size_t n=0;n<count;++n){auto time=word(bytes,8+4*n);
      valid(n==0 || (time&~flag)>(visibility.times.back()&~flag));visibility.times.push_back(time);}}
  valid(type<=1);if(type==0 && pivot<m_nodes.size())m_nodes[pivot].visibility=std::move(visibility);
}

std::array<float,4> NativeW3DAnimation::sampleChannel(const Channel& channel,float frame,RawSampling mode)const{
  const unsigned width=channel.width;std::array<float,4> identity{0,0,0,width==4?1.f:0.f};if(!channel.present)return identity;
  auto at=[&](std::int64_t index){auto result=identity;
    if(!channel.sparse && (index<std::int64_t(channel.first) || index>std::int64_t(channel.last)))return result;
    auto offset=std::size_t(channel.sparse?index:index-channel.first)*width;
    for(unsigned n=0;n<width;++n)result[n]=channel.values[offset+n];
    return result;};
  std::uint32_t first,second;float ratio;
  if(channel.sparse){auto integer=std::uint32_t(frame);auto it=std::upper_bound(channel.times.begin(),channel.times.end(),integer,
      [](std::uint32_t value,std::uint32_t key){return value<(key&~flag);});
    first=std::uint32_t(it-channel.times.begin()-1);if(first+1==channel.times.size() || (channel.times[first+1]&flag))return at(first);
    second=first+1;ratio=(frame-float(channel.times[first]&~flag))/float((channel.times[second]&~flag)-(channel.times[first]&~flag));
  } else if(!m_compressed && mode==RawSampling::RuntimeGeneralsRounded){
    auto index=signedRounded(frame);if(std::int64_t(index)>=m_frames)index=0;return at(index);
  } else if(!m_compressed && mode==RawSampling::GeneralsRounded){auto index=rounded(frame);if(index>=m_frames)index=0;return at(index);}
  else {first=std::uint32_t(frame);ratio=frame-float(first);second=first+1;
    if(m_compressed){first=std::min(first,channel.last);second=std::min(second,channel.last);}
    else if(second>=m_frames)second=0;}
  const auto a=at(first),b=at(second);if(width==4)return slerp(a,b,ratio);auto result=a;result[0]=lerp(a[0],b[0],ratio);valid(std::isfinite(result[0]));return result;
}
NativeW3DAnimation::Motion NativeW3DAnimation::sample(std::size_t pivot,float frame,RawSampling mode)const{
  valid(mode==RawSampling::GeneralsRounded || mode==RawSampling::Interpolated || mode==RawSampling::RuntimeGeneralsRounded);
  valid(pivot<m_nodes.size() && std::isfinite(frame));
  if(!m_compressed && mode==RawSampling::RuntimeGeneralsRounded)(void)signedRounded(frame);
  else valid(frame>=0 && double(frame)<double(m_frames));
  const auto& node=m_nodes[pivot];Motion result;
  for(unsigned n=0;n<3;++n)result.translation[n]=sampleChannel(node.channels[n],frame,mode)[0];
  result.rotation=sampleChannel(node.channels[6],frame,mode);const auto& visibility=node.visibility;
  if(visibility.present){if(visibility.sparse){auto integer=std::uint32_t(frame);auto it=std::upper_bound(visibility.times.begin(),visibility.times.end(),integer,
        [](std::uint32_t value,std::uint32_t key){return value<(key&~flag);});
      auto index=it==visibility.times.begin()?0:std::size_t(it-visibility.times.begin()-1);result.visible=(visibility.times[index]&flag)!=0;
    }else {std::int64_t index=mode==RawSampling::RuntimeGeneralsRounded?signedRounded(frame):
        mode==RawSampling::GeneralsRounded?std::int64_t(rounded(frame)):std::int64_t(std::uint32_t(frame));
      if(index>=m_frames)index=0;
      result.visible=index<std::int64_t(visibility.first) || index>std::int64_t(visibility.last)?visibility.defaultValue:
        (visibility.bits[(index-visibility.first)/8]&(1u<<((index-visibility.first)%8)))!=0;}}
  return result;
}
NativeW3DAnimation::Motion NativeW3DAnimation::blend(const Motion& a,const Motion& b,float percentage){
  valid(std::isfinite(percentage) && percentage>=0 && percentage<=1);Motion result;
  for(unsigned n=0;n<4;++n)valid(std::isfinite(a.rotation[n]) && std::isfinite(b.rotation[n]));
  for(unsigned n=0;n<3;++n){result.translation[n]=float((1.0-percentage)*a.translation[n]+percentage*b.translation[n]);valid(std::isfinite(result.translation[n]));}
  result.rotation=slerp(a.rotation,b.rotation,percentage);result.visible=a.visible || b.visible;return result;
}
