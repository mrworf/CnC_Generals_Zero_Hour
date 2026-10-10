// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/NativeW3DSubobjects.h"
#include "Common/Errors.h"
#include <algorithm>
#include <limits>

namespace {
void validName(std::string_view name){
  if(name.empty()||name.size()>255||name.find('\0')!=name.npos)throw ERROR_BAD_ARG;
}
bool equal(std::string_view a,std::string_view b)noexcept{
  if(a.size()!=b.size())return false;
  auto lower=[](unsigned char c){return c>='A'&&c<='Z'?c+('a'-'A'):c;};
  for(std::size_t n=0;n<a.size();++n)if(lower(a[n])!=lower(b[n]))return false;
  return true;
}
bool authoredHidden(const NativeW3DAssets::Model& model){
  if(model->kind!=NativeW3DAssets::Prototype::Kind::Mesh)return false;
  if(!model->document||model->metadataIndex>=model->document->meshes().size())throw ERROR_BAD_ARG;
  // Public W3D format W3D_MESH_FLAG_HIDDEN; source mesh.cpp::Load_W3D.
  return (model->document->meshes()[model->metadataIndex].attributes&0x00001000u)!=0;
}
}
NativeW3DSubobjects NativeW3DSubobjects::admit(const NativeW3DAssets::Model& model){
  if(!model)throw ERROR_BAD_ARG;
  NativeW3DSubobjects out;out.m_model=model;out.m_hidden=authoredHidden(model);
  if(model->kind!=NativeW3DAssets::Prototype::Kind::Hlod)return out;
  if(!model->hierarchy||model->hierarchy->pivots.empty())throw ERROR_BAD_ARG;
  const auto& pivots=model->hierarchy->pivots;
  if(pivots[0].parent!=UINT32_MAX)throw ERROR_BAD_ARG;
  for(std::size_t n=1;n<pivots.size();++n){
    auto cursor=std::uint32_t(n);std::size_t steps=0;
    while(cursor!=0){
      if(cursor>=pivots.size()||++steps>=pivots.size())throw ERROR_BAD_ARG;
      cursor=pivots[cursor].parent;
    }
  }
  std::size_t count=model->aggregates.size();
  for(const auto& lod:model->levels){
    if(lod.objects.size()>std::numeric_limits<std::size_t>::max()-count)throw ERROR_BAD_ARG;
    count+=lod.objects.size();
  }
  out.m_children.reserve(count);
  auto append=[&](const NativeW3DAssets::Attachment& child){
    if(!child.model||child.bone>=pivots.size())throw ERROR_BAD_ARG;
    out.m_children.push_back({child.model,child.bone,authoredHidden(child.model)});
  };
  for(const auto& lod:model->levels)for(const auto& child:lod.objects)append(child);
  for(const auto& child:model->aggregates)append(child);
  return out;
}
void NativeW3DSubobjects::swap(NativeW3DSubobjects& other)noexcept{
  m_model.swap(other.m_model);m_children.swap(other.m_children);std::swap(m_hidden,other.m_hidden);
}
std::optional<std::size_t> NativeW3DSubobjects::find(std::string_view name)const{
  validName(name);
  for(std::size_t n=0;n<m_children.size();++n)if(equal(m_children[n].model->name,name))return n;
  for(std::size_t n=0;n<m_children.size();++n){
    std::string_view child=m_children[n].model->name;const auto dot=child.find('.');
    if(dot!=child.npos)child.remove_prefix(dot+1);
    if(equal(child,name))return n;
  }
  return {};
}
bool NativeW3DSubobjects::descendant(std::uint32_t child,std::uint32_t parent)const noexcept{
  if(!m_model->hierarchy)return false;
  const auto& pivots=m_model->hierarchy->pivots;
  while(child!=0){child=pivots[child].parent;if(child==parent)return true;}
  return false;
}
void NativeW3DSubobjects::apply(std::span<const Rule> rules){
  for(const auto& rule:rules)validName(rule.name);
  for(const auto& rule:rules)if(const auto index=find(rule.name)){
    const auto bone=m_children[*index].bone;m_children[*index].hidden=rule.hidden;
    for(auto& child:m_children)if(descendant(child.bone,bone))child.hidden=rule.hidden;
  }
}
bool NativeW3DSubobjects::setFirstOnBone(std::uint32_t bone,bool hidden){
  for(auto& child:m_children)if(child.bone==bone){child.hidden=hidden;return true;}
  return false;
}
