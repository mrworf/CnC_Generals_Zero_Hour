// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/NativeW3DModelBounds.h"
#include "Common/Errors.h"
#include <algorithm>
#include <cmath>

namespace {
using Matrix=NativeW3DPose::Matrix;
void valid(bool value){if(!value)throw ERROR_CORRUPT_FILE_FORMAT;}
template<std::size_t N>void finite(const std::array<float,N>& values){for(auto value:values)valid(std::isfinite(value));}
float length(const std::array<float,3>& v){const auto result=std::sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);valid(std::isfinite(result));return result;}
std::array<float,3> point(const Matrix& tm,const std::array<float,3>& v){std::array<float,3> out;
  for(unsigned row=0;row<3;++row){out[row]=tm[row*4+3];for(unsigned col=0;col<3;++col)out[row]+=tm[row*4+col]*v[col];}finite(out);return out;
}
Matrix inverse(const Matrix& world){finite(world);auto out=NativeW3DPose::identity();
  for(unsigned row=0;row<3;++row)for(unsigned col=0;col<3;++col)out[row*4+col]=world[col*4+row];
  for(unsigned row=0;row<3;++row){out[row*4+3]=0;for(unsigned col=0;col<3;++col)out[row*4+3]-=out[row*4+col]*world[col*4+3];}finite(out);return out;
}
Matrix multiply(const Matrix& a,const Matrix& b){Matrix out;
  for(unsigned row=0;row<3;++row){for(unsigned col=0;col<3;++col){out[row*4+col]=0;for(unsigned k=0;k<3;++k)out[row*4+col]+=a[row*4+k]*b[k*4+col];}
    out[row*4+3]=a[row*4+3];for(unsigned k=0;k<3;++k)out[row*4+3]+=a[row*4+k]*b[k*4+3];}finite(out);return out;
}
bool boundingName(std::string_view name){if(auto dot=name.find('.');dot!=name.npos)name.remove_prefix(dot+1);
  constexpr std::string_view expected="boundingbox";if(name.size()!=expected.size())return false;
  for(unsigned n=0;n<name.size();++n){auto c=name[n];if(c>='A'&&c<='Z')c+= 'a'-'A';if(c!=expected[n])return false;}return true;
}
}
NativeW3DModelBounds::Box NativeW3DModelBounds::transform(const Box& box,const Matrix& tm){Box out;out.center=point(tm,box.center);
  for(unsigned row=0;row<3;++row)for(unsigned col=0;col<3;++col)out.extent[row]+=std::abs(tm[row*4+col])*box.extent[col];finite(out.extent);return out;
}
NativeW3DModelBounds NativeW3DModelBounds::decode(const NativeW3DAssets::Model& model,float scale){valid(bool(model)&&std::isfinite(scale));
  NativeW3DModelBounds out;using Kind=NativeW3DAssets::Prototype::Kind;
  switch(model->kind){
    case Kind::Mesh:{const auto& mesh=model->document->meshes()[model->metadataIndex];
      for(unsigned n=0;n<3;++n){out.m_base.center[n]=(mesh.minimum[n]+mesh.maximum[n])*.5f*scale;out.m_base.extent[n]=(mesh.maximum[n]-mesh.minimum[n])*.5f*scale;out.m_sphereCenter[n]=mesh.sphereCenter[n]*scale;}
      out.m_radius=mesh.sphereRadius*scale;break;}
    case Kind::Null:out.m_base.extent={.1f,.1f,.1f};out.m_radius=.1f;break;
    case Kind::AlignedBox:case Kind::OrientedBox:{const auto& box=model->document->boxes()[model->metadataIndex];out.m_base={box.center,box.extent};out.m_sphereCenter=box.center;out.m_radius=length(box.extent);break;}
    case Kind::Hlod:{out.m_hlod=true;valid(!model->levels.empty());auto pose=NativeW3DAssets::createPose(model);
      const auto& highest=model->levels.back().objects;
      for(auto it=highest.rbegin();it!=highest.rend();++it)if(it->model->kind==Kind::OrientedBox&&boundingName(it->model->name)){
        const auto& box=it->model->document->boxes()[it->model->metadataIndex];out.m_override=Override{it->bone,{box.center,box.extent}};break;}
      bool first=true;std::array<float,3> minimum{},maximum{};
      auto add=[&](const NativeW3DAssets::Attachment& attachment){auto child=decode(attachment.model,scale);NativeW3DPose childPose;
        if(attachment.model->hierarchy){childPose=NativeW3DAssets::createPose(attachment.model);childPose.evaluate(NativeW3DPose::identity(),nullptr,0,scale);}
        const auto local=child.objectBox(childPose,NativeW3DPose::identity());const auto tm=pose.pristineBone(attachment.bone,NativeW3DPose::identity(),scale);const auto box=transform(local,tm);
        for(unsigned n=0;n<3;++n){const auto lo=box.center[n]-box.extent[n],hi=box.center[n]+box.extent[n];valid(std::isfinite(lo)&&std::isfinite(hi));
          if(first){minimum[n]=lo;maximum[n]=hi;}else {minimum[n]=std::min(minimum[n],lo);maximum[n]=std::max(maximum[n],hi);}}
        first=false;
      };
      for(const auto& lod:model->levels)for(const auto& attachment:lod.objects)add(attachment);for(const auto& attachment:model->aggregates)add(attachment);
      if(!first)for(unsigned n=0;n<3;++n){out.m_base.center[n]=(minimum[n]+maximum[n])*.5f;out.m_base.extent[n]=(maximum[n]-minimum[n])*.5f;}
      out.m_sphereCenter=out.m_base.center;out.m_radius=length(out.m_base.extent);break;
    }
  }
  finite(out.m_base.center);finite(out.m_base.extent);finite(out.m_sphereCenter);valid(std::isfinite(out.m_radius)&&out.m_radius>=0);
  for(auto extent:out.m_base.extent)valid(extent>=0);return out;
}
NativeW3DModelBounds::Box NativeW3DModelBounds::objectBox(const NativeW3DPose& pose,const Matrix& world)const{
  if(!m_override)return m_base;
  return transform(m_override->local,multiply(inverse(world),pose.bone(m_override->bone).transform));
}
float NativeW3DModelBounds::worldSphereRadius(const NativeW3DPose& pose,const Matrix& world,float objectScale)const{
  const float radius=m_hlod?length(objectBox(pose,world).extent):m_radius;
  const float result=m_override?radius:radius*objectScale;valid(std::isfinite(result)&&result>=0);return result;
}
