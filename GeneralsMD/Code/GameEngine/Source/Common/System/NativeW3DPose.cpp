// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/NativeW3DPose.h"
#include "Common/Errors.h"
#include <cmath>
#include <utility>

namespace {
using Matrix=NativeW3DPose::Matrix;
void valid(bool condition){if(!condition)throw ERROR_CORRUPT_FILE_FORMAT;}
void finite(const Matrix& matrix){for(float value:matrix)valid(std::isfinite(value));}
bool equal(std::string_view a,std::string_view b){if(a.size()!=b.size())return false;
  auto lower=[](unsigned char c){return c>='A' && c<='Z'?c+('a'-'A'):c;};
  for(std::size_t n=0;n<a.size();++n)if(lower(a[n])!=lower(b[n]))return false;
  return true;}
Matrix multiply(const Matrix& a,const Matrix& b){Matrix out{};
  for(unsigned row=0;row<3;++row)for(unsigned col=0;col<4;++col){out[row*4+col]=a[row*4]*b[col]+a[row*4+1]*b[4+col]+a[row*4+2]*b[8+col];
    if(col==3)out[row*4+col]+=a[row*4+3];}
  finite(out);return out;}
Matrix rotation(const std::array<float,4>& q){for(float value:q)valid(std::isfinite(value));Matrix out=NativeW3DPose::identity();
  out[0]=float(1.0-2.0*(q[1]*q[1]+q[2]*q[2]));out[1]=float(2.0*(q[0]*q[1]-q[2]*q[3]));out[2]=float(2.0*(q[2]*q[0]+q[1]*q[3]));
  out[4]=float(2.0*(q[0]*q[1]+q[2]*q[3]));out[5]=float(1.0-2.0f*(q[2]*q[2]+q[0]*q[0]));out[6]=float(2.0*(q[1]*q[2]-q[0]*q[3]));
  out[8]=float(2.0*(q[2]*q[0]-q[1]*q[3]));out[9]=float(2.0*(q[1]*q[2]+q[0]*q[3]));out[10]=float(1.0-2.0*(q[1]*q[1]+q[0]*q[0]));finite(out);return out;}
Matrix scaled(Matrix matrix,float scale){matrix[3]*=scale;matrix[7]*=scale;matrix[11]*=scale;finite(matrix);return matrix;}
}
NativeW3DPose::Matrix NativeW3DPose::identity()noexcept{return {1,0,0,0,0,1,0,0,0,0,1,0};}
NativeW3DPose NativeW3DPose::decode(const NativeW3DDocument::Hierarchy& hierarchy){
  valid(!hierarchy.pivots.empty());NativeW3DPose candidate;candidate.m_name=hierarchy.name;candidate.m_nodes.reserve(hierarchy.pivots.size());
  for(std::size_t n=0;n<hierarchy.pivots.size();++n){const auto& pivot=hierarchy.pivots[n];
    valid(n==0?pivot.parent==UINT32_MAX:pivot.parent<n);auto base=rotation(pivot.rotation);
    for(unsigned axis=0;axis<3;++axis){valid(std::isfinite(pivot.translation[axis]));base[axis*4+3]=pivot.translation[axis];}
    candidate.m_nodes.push_back({pivot.name,pivot.parent,base,identity(),false,false});}
  candidate.m_pose.resize(candidate.m_nodes.size());candidate.m_candidate.resize(candidate.m_nodes.size());candidate.evaluate(identity());return candidate;
}
void NativeW3DPose::replace(const NativeW3DDocument::Hierarchy& hierarchy){auto candidate=decode(hierarchy);swap(candidate);}
void NativeW3DPose::swap(NativeW3DPose& other)noexcept{m_name.swap(other.m_name);m_nodes.swap(other.m_nodes);m_pose.swap(other.m_pose);m_candidate.swap(other.m_candidate);}
std::optional<std::size_t> NativeW3DPose::findBone(std::string_view name)const noexcept{for(std::size_t n=0;n<m_nodes.size();++n)if(equal(m_nodes[n].name,name))return n;return std::nullopt;}
const NativeW3DPose::Bone& NativeW3DPose::bone(std::size_t index)const{valid(index<m_pose.size());return m_pose[index];}
NativeW3DPose::Matrix NativeW3DPose::pristineBone(std::size_t index,const Matrix& root,float scale)const{
  valid(index<m_nodes.size() && std::isfinite(scale));finite(root);Matrix result=identity();
  while(index!=0){const auto& node=m_nodes[index];result=multiply(scaled(node.base,scale),result);index=node.parent;}
  return multiply(root,result);
}
void NativeW3DPose::capture(std::size_t index){valid(index<m_nodes.size());m_nodes[index].captured=true;}
void NativeW3DPose::release(std::size_t index){valid(index<m_nodes.size());m_nodes[index].captured=false;}
void NativeW3DPose::control(std::size_t index,const Matrix& matrix,bool world){valid(index<m_nodes.size() && m_nodes[index].captured);finite(matrix);
  m_nodes[index].capture=matrix;m_nodes[index].world=world;}
void NativeW3DPose::evaluate(const Matrix& root,const NativeW3DAnimation* motion,float frame,float scale){evaluate(root,motion,frame,nullptr,0,0,scale);}
void NativeW3DPose::evaluateRuntime(const Matrix& root,const NativeW3DAnimation* motion,float frame,float scale){evaluate(root,motion,frame,nullptr,0,0,scale,true);}
void NativeW3DPose::evaluateBlend(const Matrix& root,const NativeW3DAnimation& a,float frameA,const NativeW3DAnimation& b,float frameB,float percentage,float scale){
  valid(std::isfinite(percentage) && percentage>=0 && percentage<=1);evaluate(root,&a,frameA,&b,frameB,percentage,scale);
}
void NativeW3DPose::evaluate(const Matrix& root,const NativeW3DAnimation* a,float frameA,const NativeW3DAnimation* b,float frameB,float percentage,float scale,bool runtime){
  valid(!m_nodes.empty() && std::isfinite(scale));finite(root);
  const auto mode=runtime?NativeW3DAnimation::RawSampling::RuntimeGeneralsRounded:
    b?NativeW3DAnimation::RawSampling::Interpolated:NativeW3DAnimation::RawSampling::GeneralsRounded;
  for(const auto& pair:{std::pair{a,frameA},std::pair{b,frameB}})if(pair.first){
    valid(equal(pair.first->hierarchy(),m_name) && pair.first->pivots()==m_nodes.size());(void)pair.first->sample(0,pair.second,mode);}
  m_candidate[0]={root,true};
  for(std::size_t n=1;n<m_nodes.size();++n){const auto& node=m_nodes[n];auto current=multiply(m_candidate[node.parent].transform,scaled(node.base,scale));bool visible=true;
    if(a){
      auto motion=a->sample(n,frameA,mode);if(b)motion=NativeW3DAnimation::blend(motion,b->sample(n,frameB,mode),percentage);
      auto local=rotation(motion.rotation);for(unsigned axis=0;axis<3;++axis)local[axis*4+3]=motion.translation[axis]*scale;
      finite(local);current=multiply(current,local);visible=motion.visible;}
    if(node.captured){auto control=node.capture;if(node.world){control[3]=control[7]=control[11]=0;current=multiply(current,control);
        for(unsigned axis=0;axis<3;++axis)current[axis*4+3]+=node.capture[axis*4+3];
        finite(current);}
      else current=multiply(current,control);
      visible=true;}
    m_candidate[n]={current,visible};}
  m_pose.swap(m_candidate); // A late failure leaves the previous whole pose visible.
}
