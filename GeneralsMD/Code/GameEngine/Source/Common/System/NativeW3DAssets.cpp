// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/NativeW3DAssets.h"
#include "Common/Errors.h"
#include "Common/FileSystem.h"
#include "Common/FileOwner.h"
#include <algorithm>
#include <map>

namespace {
using Failure=NativeW3DAssetFailure;
unsigned char lower(unsigned char value){return value>='A' && value<='Z'?value+('a'-'A'):value;}
struct CaseLess {
  using is_transparent=void;
  bool operator()(std::string_view a,std::string_view b)const noexcept {
    for(std::size_t n=0;n<std::min(a.size(),b.size());++n){auto x=lower(a[n]),y=lower(b[n]);if(x!=y)return x<y;}
    return a.size()<b.size();
  }
};
bool equal(std::string_view a,std::string_view b){return !CaseLess{}(a,b) && !CaseLess{}(b,a);}
void identifier(std::string_view name){
  if(name.empty() || name.size()>255 || name=="." || name=="..")throw ERROR_BAD_ARG;
  for(unsigned char c:name)if(c<32 || c==127 || c=='/' || c=='\\' || c==':')throw ERROR_BAD_ARG;
}
std::string filename(std::string_view stem){identifier(stem);return std::string(stem)+".w3d";}
std::string meshName(const NativeW3DDocument::Mesh& mesh){return mesh.container.empty()?mesh.name:mesh.container+"."+mesh.name;}
NativeW3DAssets::Hierarchy defaultHierarchy(){auto tree=std::make_shared<NativeW3DDocument::Hierarchy>();
  NativeW3DDocument::Pivot root;root.name="RootTransform";root.rotation[3]=1;tree->pivots.push_back(std::move(root));return tree;}
void relationship(bool valid){if(!valid)throw Failure(Failure::Code::InvalidRelationship);}
std::string nullName(std::span<const std::uint8_t> bytes){if(bytes.size()<48)throw ERROR_CORRUPT_FILE_FORMAT;
  auto start=bytes.begin()+16,end=std::find(start,start+32,0);if(end==start || end==start+32)throw ERROR_CORRUPT_FILE_FORMAT;
  return {reinterpret_cast<const char*>(&*start),std::size_t(end-start)};}
void skinRelationship(const NativeW3DAssets::Model& model,std::size_t bones){
  if(model->kind!=NativeW3DAssets::Prototype::Kind::Mesh)return;
  const auto& mesh=model->document->meshes()[model->metadataIndex];
  if(!(mesh.attributes&0x00020000) && !(mesh.vertexChannels&0x10))return;
  bool found=false;
  for(auto child:model->document->chunks()[mesh.chunk].children)if(model->document->chunks()[child].type==0xe){
    found=true;auto bytes=model->document->payload(child);relationship(bytes.size()/8==mesh.vertices);
    for(std::size_t at=0;at<bytes.size();at+=8)relationship((unsigned(bytes[at])|(unsigned(bytes[at+1])<<8))<bones);
  }
  relationship(found || mesh.vertices==0);
}
}

struct NativeW3DAssets::State {
  struct Definition {Prototype::Kind kind;Document document;std::size_t index;};
  struct AnimDefinition {Document document;std::size_t index;};
  std::map<std::string,Document,CaseLess> documents;
  std::map<std::string,Hierarchy,CaseLess> hierarchies;
  std::map<std::string,Definition,CaseLess> definitions;
  std::map<std::string,AnimDefinition,CaseLess> animationDefinitions;
  std::map<std::string,Model,CaseLess> models;
  std::map<std::string,Animation,CaseLess> animations;
};
const char* NativeW3DAssetFailure::what()const noexcept{
  switch(m_code){case Code::MissingFile:return "Required W3D input is missing from the configured read-only namespaces";
    case Code::MissingDefinition:return "Required W3D definition is unavailable in the selected input";
    case Code::DependencyCycle:return "W3D prototype dependencies are cyclic or too deep";
    case Code::InvalidRelationship:return "W3D prototype references an invalid hierarchy or geometry relationship";}
  return "W3D admission failed";
}
NativeW3DAssets::NativeW3DAssets(FileSystem& files,std::string language,std::string userDirectory)
  :m_files(files),m_language(std::move(language)),m_userDirectory(std::move(userDirectory)){
  identifier(m_language);if(m_language.find('.')!=std::string::npos)throw ERROR_BAD_ARG;
  if(!m_userDirectory.empty() && m_userDirectory.back()!='/')m_userDirectory+='/';
}
void NativeW3DAssets::load(State& state,const std::string& name){
  if(state.documents.contains(name))return;
  const std::array prefixes={"Data/"+m_language+"/Art/W3D/",std::string("Art/W3D/"),m_userDirectory};
  FileCloseOwner file;
  for(const auto& prefix:prefixes){if(prefix.empty())continue;const auto path=prefix+name;
    file.reset(m_files.openFile(path.c_str(),File::READ|File::BINARY|File::STREAMING));if(file)break;}
  if(!file)throw Failure(Failure::Code::MissingFile);
  auto document=std::make_shared<const NativeW3DDocument>(NativeW3DDocument::read(*file));
  state.documents.emplace(name,document);
  // Document metadata tables each preserve the order of their typed roots.
  // Traverse the actual root order, not grouped types: first prototype wins.
  std::size_t treeIndex=0,meshIndex=0,hlodIndex=0,animationIndex=0;
  for(auto root:document->roots())switch(document->chunks()[root].type){
    case 0x100:{const auto& tree=document->hierarchies()[treeIndex++];identifier(tree.name);
      state.hierarchies.emplace(tree.name,Hierarchy(document,&tree));break;}
    case 0:{const auto index=meshIndex++;auto key=meshName(document->meshes()[index]);identifier(key);
      state.definitions.emplace(std::move(key),State::Definition{Prototype::Kind::Mesh,document,index});break;}
    case 0x700:{const auto index=hlodIndex++;const auto& model=document->hlods()[index];identifier(model.name);
      state.definitions.emplace(model.name,State::Definition{Prototype::Kind::Hlod,document,index});break;}
    case 0x750:{auto key=nullName(document->payload(root));identifier(key);
      state.definitions.emplace(std::move(key),State::Definition{Prototype::Kind::Null,document,root});break;}
    case 0x200:case 0x280:{const auto index=animationIndex++;const auto& animation=document->animations()[index];auto key=animation.hierarchy+"."+animation.name;identifier(key);
      state.animationDefinitions.emplace(std::move(key),State::AnimDefinition{document,index});break;}
    default:break;
  }
}
NativeW3DAssets::Hierarchy NativeW3DAssets::resolveHierarchy(State& state,std::string_view name){
  if(auto found=state.hierarchies.find(name);found!=state.hierarchies.end())return found->second;
  load(state,filename(name));auto found=state.hierarchies.find(name);
  if(found==state.hierarchies.end())throw Failure(Failure::Code::MissingDefinition);
  return found->second;
}
NativeW3DAssets::Model NativeW3DAssets::resolveModel(State& state,std::string_view name,std::vector<std::string_view>& ancestors){
  identifier(name);if(auto found=state.models.find(name);found!=state.models.end())return found->second;
  if(ancestors.size()>=128 || std::any_of(ancestors.begin(),ancestors.end(),[&](auto ancestor){return equal(ancestor,name);}))throw Failure(Failure::Code::DependencyCycle);
  ancestors.push_back(name);struct Pop {std::vector<std::string_view>& values;~Pop(){values.pop_back();}}pop{ancestors};
  auto prototype=std::make_shared<Prototype>();prototype->name=name;
  if(equal(name,"NULL")){prototype->kind=Prototype::Kind::Null;state.models.emplace(prototype->name,prototype);return prototype;}
  auto definition=state.definitions.find(name);
  if(definition==state.definitions.end()){load(state,filename(name.substr(0,name.find('.'))));definition=state.definitions.find(name);}
  if(definition==state.definitions.end())throw Failure(Failure::Code::MissingDefinition);
  prototype->kind=definition->second.kind;prototype->document=definition->second.document;prototype->metadataIndex=definition->second.index;
  if(prototype->kind==Prototype::Kind::Hlod){const auto& data=prototype->document->hlods()[prototype->metadataIndex];relationship(!data.levels.empty());
    prototype->hierarchy=data.hierarchy.empty()?defaultHierarchy():resolveHierarchy(state,data.hierarchy);
    auto attachment=[&](const NativeW3DDocument::Subobject& object){relationship(object.bone<prototype->hierarchy->pivots.size());
      auto child=resolveModel(state,object.name,ancestors);skinRelationship(child,prototype->hierarchy->pivots.size());return Attachment{object.bone,std::move(child)};};
    prototype->levels.reserve(data.levels.size());
    for(const auto& source:data.levels){Lod lod{source.maxScreenSize,{}};lod.objects.reserve(source.objects.size());for(const auto& object:source.objects)lod.objects.push_back(attachment(object));prototype->levels.push_back(std::move(lod));}
    prototype->aggregates.reserve(data.aggregates.size());for(const auto& object:data.aggregates)prototype->aggregates.push_back(attachment(object));
    prototype->proxies=data.proxies;for(const auto& proxy:prototype->proxies)relationship(proxy.bone<prototype->hierarchy->pivots.size());
  }
  state.models.emplace(prototype->name,prototype);return prototype;
}
NativeW3DAssets::Animation NativeW3DAssets::resolveAnimation(State& state,std::string_view name){
  if(auto found=state.animations.find(name);found!=state.animations.end())return found->second;
  auto definition=state.animationDefinitions.find(name);
  if(definition==state.animationDefinitions.end()){auto separator=name.find('.');if(separator==std::string_view::npos || separator+1==name.size())throw ERROR_BAD_ARG;
    load(state,filename(name.substr(separator+1)));definition=state.animationDefinitions.find(name);}
  if(definition==state.animationDefinitions.end())throw Failure(Failure::Code::MissingDefinition);
  const auto& data=definition->second.document->animations()[definition->second.index];auto tree=resolveHierarchy(state,data.hierarchy);
  auto motion=std::make_shared<const NativeW3DAnimation>(NativeW3DAnimation::decode(*definition->second.document,definition->second.index,*tree));
  auto animation=std::make_shared<const AnimationAsset>(AnimationAsset{std::move(motion),tree,definition->second.document});
  state.animations.emplace(std::string(name),animation);return animation;
}
NativeW3DAssets::Model NativeW3DAssets::model(std::string_view name){
  identifier(name);if(m_state)if(auto found=m_state->models.find(name);found!=m_state->models.end())return found->second;
  auto candidate=m_state?std::make_shared<State>(*m_state):std::make_shared<State>();std::vector<std::string_view> ancestors;
  auto model=resolveModel(*candidate,name,ancestors);m_state=std::move(candidate);return model;
}
NativeW3DAssets::Hierarchy NativeW3DAssets::hierarchy(std::string_view name){
  identifier(name);if(m_state)if(auto found=m_state->hierarchies.find(name);found!=m_state->hierarchies.end())return found->second;
  auto candidate=m_state?std::make_shared<State>(*m_state):std::make_shared<State>();auto tree=resolveHierarchy(*candidate,name);m_state=std::move(candidate);return tree;
}
NativeW3DAssets::Animation NativeW3DAssets::animation(std::string_view name){
  identifier(name);if(m_state)if(auto found=m_state->animations.find(name);found!=m_state->animations.end())return found->second;
  auto candidate=m_state?std::make_shared<State>(*m_state):std::make_shared<State>();auto animation=resolveAnimation(*candidate,name);m_state=std::move(candidate);return animation;
}
NativeW3DAssets::Counts NativeW3DAssets::counts()const noexcept{
  if(!m_state)return {};
  return {m_state->documents.size(),m_state->hierarchies.size(),m_state->definitions.size(),m_state->animationDefinitions.size(),m_state->models.size(),m_state->animations.size()};
}
void NativeW3DAssets::clear()noexcept{m_state.reset();}
NativeW3DPose NativeW3DAssets::createPose(const Model& model){if(!model || !model->hierarchy)throw ERROR_BAD_ARG;return NativeW3DPose::decode(*model->hierarchy);}
void NativeW3DAssets::validateMotionBinding(const Model& model,const Animation& animation){
  relationship(model && animation && model->hierarchy && animation->motion && model->hierarchy==animation->hierarchy);
}
