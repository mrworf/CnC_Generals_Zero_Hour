// SPDX-License-Identifier: GPL-3.0-or-later
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

namespace {
using Data=std::vector<std::uint8_t>;
using Assets=NativeW3DAssets;
using Failure=NativeW3DAssetFailure;
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void word(Data& out,std::uint32_t value){for(unsigned n=0;n<4;++n)out.push_back(std::uint8_t(value>>(8*n)));}
void bigWord(Data& out,std::uint32_t value){for(int n=3;n>=0;--n)out.push_back(std::uint8_t(value>>(8*n)));}
void half(Data& out,unsigned value){out.push_back(std::uint8_t(value));out.push_back(std::uint8_t(value>>8));}
void real(Data& out,float value){word(out,std::bit_cast<std::uint32_t>(value));}
void text(Data& out,std::string_view value,std::size_t width=16){require(value.size()<width,"generated fixed name");out.insert(out.end(),value.begin(),value.end());out.resize(out.size()+width-value.size(),0);}
Data join(std::initializer_list<Data> entries){Data out;for(const auto& entry:entries)out.insert(out.end(),entry.begin(),entry.end());return out;}
Data chunk(unsigned type,const Data& bytes,bool nested=false){Data out;word(out,type);word(out,std::uint32_t(bytes.size())|(nested?0x80000000:0));out.insert(out.end(),bytes.begin(),bytes.end());return out;}
Data tree(std::string_view name="Tree",float base=10){Data header,pivots;word(header,0x40001);text(header,name);word(header,3);for(unsigned n=0;n<3;++n)real(header,0);
  for(unsigned n=0;n<3;++n){text(pivots,n==0?"Root":n==1?"Arm":"Muzzle");word(pivots,n==0?UINT32_MAX:n-1);
    real(pivots,n==0?99:n==1?base:4);for(unsigned i=0;i<8;++i)real(pivots,0);real(pivots,1);}
  return chunk(0x100,join({chunk(0x101,header),chunk(0x102,pivots)}),true);}
Data mesh(std::string_view container,std::string_view name,bool skin=false,unsigned lastBone=2,float radius=3){Data header,vertices,triangles,influences;
  word(header,0x40002);word(header,skin?0x00020000:0x1234);text(header,name);text(header,container);
  for(auto value:{1u,3u,0u,0u,0u,0u,0u,skin?0x10u:0u,1u})word(header,value);
  for(float value:{0.f,0.f,0.f,2.f,2.f,2.f,1.f,1.f,1.f,radius})real(header,value);
  for(unsigned n=0;n<3;++n){real(vertices,float(n));real(vertices,0);real(vertices,0);half(influences,n==2?lastBone:n);for(unsigned i=0;i<6;++i)influences.push_back(0xa5);}
  for(auto value:{0u,1u,2u,0xf1234567u})word(triangles,value);for(float value:{0.f,0.f,1.f,0.f})real(triangles,value);
  return chunk(0,join({chunk(0x1f,header),chunk(2,vertices),chunk(0x20,triangles),skin?chunk(0xe,influences):Data{},chunk(0xfefefefe,{9,8,7})}),true);}
Data null(std::string_view name){Data out;word(out,0x10000);word(out,0xf1234567);word(out,0xabcdef01);word(out,0x98765432);text(out,name,32);return chunk(0x750,out);}
Data objects(unsigned type,const std::vector<std::pair<std::string,unsigned>>& entries,float maximum){Data header,out;word(header,entries.size());real(header,maximum);out=chunk(0x703,header);
  for(const auto& [name,bone]:entries){Data object;word(object,bone);text(object,name,32);auto bytes=chunk(0x704,object);out.insert(out.end(),bytes.begin(),bytes.end());}return chunk(type,out,true);}
Data hlod(std::string_view name,std::string_view hierarchy,const std::vector<std::pair<std::string,unsigned>>& entries){Data header;word(header,0x10000);word(header,1);text(header,name);text(header,hierarchy);
  return chunk(0x700,join({chunk(0x701,header),objects(0x702,entries,100)}),true);}
Data outer(){Data header;word(header,0x10000);word(header,2);text(header,"Outer");text(header,"Tree");
  return join({chunk(0x700,join({chunk(0x701,header),objects(0x702,{{"Inner",1}},5),objects(0x702,{{"Body.Part",2},{"NULL",0}},std::numeric_limits<float>::max()),
    objects(0x705,{{"Marker.Null",2}},0),objects(0x706,{{"NotAFile",2}},0)}),true),chunk(0x12345678,{1,2,3})});}
Data animation(std::string_view hierarchy="Tree",std::string_view name="Walk"){Data header,channel;word(header,0x40001);text(header,name);text(header,hierarchy);word(header,2);word(header,30);
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
std::vector<std::pair<std::string,Data>> entries(){return {
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
Data big(const std::vector<std::pair<std::string,Data>>& entries){std::uint32_t offset=16;for(const auto& [name,bytes]:entries){(void)bytes;offset+=9+name.size();}
  Data out{'B','I','G','F'};bigWord(out,0);bigWord(out,entries.size());bigWord(out,offset);
  for(const auto& [name,bytes]:entries){bigWord(out,offset);bigWord(out,bytes.size());out.insert(out.end(),name.begin(),name.end());out.push_back(0);offset+=bytes.size();}
  for(const auto& [name,bytes]:entries){(void)name;out.insert(out.end(),bytes.begin(),bytes.end());}return out;}
struct Context {
  Root root,user;
  FileSystem files;
  std::unique_ptr<NativeUserStorage> storage;
  Context(bool archived=false){const auto data=entries();if(archived)root.write("scene.big",big(data));else for(const auto& [name,bytes]:data)root.write(name,bytes);
    files.mountReadOnly({root.path.string()});user.write("data/W3D/UserOnly.w3d",mesh("","UserOnly",false,0,11));
    storage=std::make_unique<NativeUserStorage>(NativeUserPaths{(user.path/"data").string(),(user.path/"cache").string()},files);files.attachUserStorage(storage.get());}
  std::string userModels()const{return (user.path/"data/W3D").string();}
};
template<class F>void rejects(F action){bool failed=false;try{action();}catch(const Failure&){failed=true;}catch(ErrorCode){failed=true;}require(failed,"invalid asset graph rejected");}
std::size_t descriptors(){DIR* directory=::opendir("/proc/self/fd");require(directory,"descriptor count");std::size_t count=0;
  while(auto* entry=::readdir(directory))if(entry->d_name[0]!='.')++count;::closedir(directory);return count;}
struct NativeUnits {
  std::size_t files=descriptors();Int pooled=TheMemoryPoolFactory->findMemoryPool("NativeDataFile")->getUsedBlockCount();
  void check()const{require(descriptors()==files && TheMemoryPoolFactory->findMemoryPool("NativeDataFile")->getUsedBlockCount()==pooled,"admission retires exact descriptors and pooled File units");}
};
void graph(Context& context){Assets owner(context.files,"English");auto model=owner.model("Outer");
  require(owner.counts()==Assets::Counts{5,1,4,0,5,0},"complete external HLOD dependency graph cardinality");
  require(model->kind==Assets::Prototype::Kind::Hlod && model->levels.size()==2 && model->levels[0].objects.size()==1 && model->levels[1].objects.size()==2,"real LOD cardinality and ordering");
  require(model->levels[0].maxScreenSize==5 && model->levels[1].maxScreenSize==std::numeric_limits<float>::max() && model->aggregates.size()==1 && model->proxies.size()==1,"authored thresholds and distinct aggregate/proxy cohorts");
  auto inner=model->levels[0].objects[0].model,body=model->levels[1].objects[0].model;
  require(inner->levels[0].objects[0].model==body && inner->hierarchy==model->hierarchy && model->proxies[0].name=="NotAFile","shared dependency identity; proxy is metadata not a loaded file");
  const auto& metadata=body->document->meshes()[body->metadataIndex];require(metadata.attributes==0x20000 && metadata.vertices==3 && metadata.triangles==1,"authored skin geometry metadata retained");
  auto clip=owner.animation("Tree.Walk");Assets::validateMotionBinding(model,clip);require(clip->hierarchy==model->hierarchy && clip->document->animations().size()==1,"clip owns exact admitted hierarchy and full input");
  auto pose=Assets::createPose(model);pose.evaluate(NativeW3DPose::identity(),clip->motion.get(),1);require(pose.bone(2).transform[3]==20,"external graph resolves actual animated muzzle transform");
  require(owner.counts()==Assets::Counts{6,1,4,1,5,1},"clip suffix file loaded once without duplicate hierarchy admission");
}
void routing(Context& context){Assets owner(context.files,"English",context.userModels());
  auto localized=owner.model("Localized");require(localized->document->meshes()[0].sphereRadius==7,"localized W3D precedence");
  Assets base(context.files,"German",context.userModels());require(base.model("Localized")->document->meshes()[0].sphereRadius==3,"base art fallback");
  require(owner.model("UserOnly")->document->meshes()[0].sphereRadius==11,"explicit separate user-storage read fallback");
  auto tree=owner.hierarchy("Tree");auto duplicate=owner.model("DupTree");require(duplicate->hierarchy==tree && tree->pivots[1].translation[0]==10,"first accepted hierarchy collision wins");
  require(owner.model("DupWinner")->kind==Assets::Prototype::Kind::Hlod && owner.model("DupMesh")->kind==Assets::Prototype::Kind::Mesh,"prototype collisions preserve authored root order across types");
  auto defaultModel=owner.model("Default");auto pose=Assets::createPose(defaultModel);require(defaultModel->hierarchy->name.empty() && pose.findBone("RootTransform")==0,"empty hierarchy legitimately uses exact source default tree");
  auto builtin=owner.model("nUlL");require(builtin->kind==Assets::Prototype::Kind::Null && !builtin->document && !builtin->hierarchy,"source explicit NULL requires no missing-file success");
  auto marker=owner.model("Marker.Null");require(marker->document->payload(marker->metadataIndex).size()==48,"named null retains both pad words and authored attributes");
}
void negative(Context& context){context.root.write("Art/W3D/Broken.w3d",hlod("Broken","Tree",{{"Missing.Part",1}}));
  Assets owner(context.files,"English");auto accepted=owner.model("Outer");auto clip=owner.animation("Tree.Walk");const auto counts=owner.counts();
  auto bad=[&](auto action){const auto live=AllocationFault::live();NativeUnits units;rejects(action);
    require(owner.counts()==counts && owner.model("outer")==accepted && owner.animation("tree.walk")==clip && AllocationFault::live()==live,"rejected graph preserves every accepted cache identity/count and retires candidate backing");units.check();};
  for(const auto* name:{"CycleA","Broken","BadBone","BadSkin","BadTree","WrongName","Malformed","NoFile"})bad([&]{(void)owner.model(name);});
  for(const auto* name:{"../escape","C:\\escape","a/b","","..","bad\nkey"})bad([&]{(void)owner.model(name);});
  bad([&]{(void)owner.animation("NoDot");});bad([&]{(void)owner.animation("Tree.NoFile");});bad([&]{(void)owner.hierarchy("AbsentTree");});
  bad([&]{(void)owner.model(std::string_view("bad\0key",7));});bad([&]{(void)Assets::createPose(owner.model("NULL"));});
  // The generated loose provider already indexed this file. Corrected content
  // is read at current length; no retail content or mounts are changed.
  context.root.write("Art/W3D/Broken.w3d",hlod("Broken","Tree",{{"Body.Part",1}}));
  require(owner.model("Broken")->levels[0].objects.size()==1,"same-owner corrected missing-child retry");
  try{(void)owner.model("SecretSelector");throw std::runtime_error("missing error expected");}catch(const Failure& error){require(error.code()==Failure::Code::MissingFile && std::string_view(error.what()).find("SecretSelector")==std::string_view::npos,"missing-data diagnostic is actionable and redacted");}
}
void cache(Context& context){Assets owner(context.files,"English");auto model=owner.model("Outer");auto clip=owner.animation("Tree.Walk");auto tree=owner.hierarchy("Tree");const auto counts=owner.counts();
  const auto live=AllocationFault::live();AllocationFault::arm(0);try{require(owner.model("oUtEr")==model && owner.animation("tReE.wAlK")==clip && owner.hierarchy("tReE")==tree,"case-insensitive cached identities");}
  catch(...){AllocationFault::disarm();throw;}const auto attempts=AllocationFault::attempts();const bool hit=AllocationFault::triggered();AllocationFault::disarm();
  require(attempts==0 && !hit && AllocationFault::live()==live && owner.counts()==counts,"cache-hit acquisition allocates nothing");
  owner.clear();require(owner.counts()==Assets::Counts{},"complete cache retirement");Assets::validateMotionBinding(model,clip);
  auto rebuilt=owner.model("Outer");auto newClip=owner.animation("Tree.Walk");Assets::validateMotionBinding(rebuilt,newClip);
  rejects([&]{Assets::validateMotionBinding(model,newClip);});rejects([&]{Assets::validateMotionBinding(rebuilt,clip);});
  require(model->hierarchy!=rebuilt->hierarchy && owner.counts()==counts,"old handles retain exact hierarchy, not a same-name new generation");
  AllocationFault::arm(0);owner.clear();const auto clearing=AllocationFault::attempts();const bool clearedHit=AllocationFault::triggered();AllocationFault::disarm();require(clearing==0 && !clearedHit,"cache clear allocates nothing");
}
void lifetime(){Assets::Model model;Assets::Animation clip;NativeW3DPose pose;
  {Context context;Assets owner(context.files,"English");model=owner.model("Outer");clip=owner.animation("Tree.Walk");pose=Assets::createPose(model);owner.clear();}
  Assets::validateMotionBinding(model,clip);pose.evaluate(NativeW3DPose::identity(),clip->motion.get(),1);
  require(pose.bone(2).transform[3]==20 && model->levels[0].objects[0].model->levels[0].objects[0].model->document->meshes()[0].vertices==3,"entire admitted model/motion graph survives VFS/asset/input retirement");
}
void faults(Context& context,bool animation,bool seeded){auto operation=[&](Assets& owner){if(animation)(void)owner.animation("Tree.Walk");else(void)owner.model("Outer");};
  std::size_t terminal;Assets::Counts expected;
  {Assets measured(context.files,"English");if(seeded)(void)measured.model("NULL");AllocationFault::arm(std::numeric_limits<std::size_t>::max());
    try{operation(measured);}catch(...){AllocationFault::disarm();throw;}terminal=AllocationFault::attempts();AllocationFault::disarm();expected=measured.counts();}
  require(terminal>0 && terminal<512,"bounded complete asset graph standard-allocation census");
  for(std::size_t ordinal=0;ordinal<=terminal;++ordinal){const auto outerLive=AllocationFault::live();NativeUnits units;
    {Assets owner(context.files,"English");Assets::Model retained;if(seeded)retained=owner.model("NULL");const auto counts=owner.counts();const auto live=AllocationFault::live();bool failed=false;
      AllocationFault::arm(ordinal);try{operation(owner);}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
      const auto hit=AllocationFault::triggered();const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
      require(failed==(ordinal<terminal) && failed==hit && attempts==(failed?ordinal+1:terminal),"every asset graph prefix and exact terminal");
      if(failed){require(owner.counts()==counts && AllocationFault::live()==live,"failed cold/warm admission retains exact cache counts and retires candidate storage");if(seeded)require(owner.model("NULL")==retained,"failed warm admission preserves accepted handle identity");}
      units.check();operation(owner);require(owner.counts()==expected,"every failure/terminal has corrected same-owner identical graph retry");units.check();
    }require(AllocationFault::live()==outerLive,"complete fault/retry asset owner retires all standard backing");units.check();
  }std::cout<<"assets "<<(animation?"animation":"model")<<(seeded?" seeded":" cold")<<" prefixes [0,"<<terminal<<"); terminal "<<terminal<<"; every retry\n";
}
ModelConditionInfo condition(){ModelConditionInfo info;info.m_modelName="Outer";
  info.m_conditionsYesVec.emplace_back();info.m_animations.emplace_back(AsciiString("Tree.Walk"),false,0);
  info.m_animations.emplace_back(AsciiString("Tree.MissingSecond"),false,0);return info;}
void conditions(Context& context){Assets owner(context.files,"English");auto info=condition();
  auto first=info.prepareNativeBinding(owner,1);require(first.pose.bone(2).transform[3]==16,"original condition chooses first animation/frame0");
  info.m_flags=1<<4;auto last=info.prepareNativeBinding(owner,1);require(last.pose.bone(2).transform[3]==20,"original pristine final-frame flag");
  auto scaled=info.prepareNativeBinding(owner,2);require(scaled.pose.bone(2).transform[3]==80,"source tree scale and external root scale both retained");
  auto near=info.prepareNativeBinding(owner,1.005f);require(std::fabs(near.pose.bone(2).transform[3]-20.1f)<.00001f,"source asset scale epsilon does not remove external root scale");
  info.addPublicBone(AsciiString("Muzzle"));info.addPublicBone(AsciiString("mUzZlE"));info.addPublicBone(AsciiString("NONE"));
  require(info.m_publicBones.size()==1 && info.m_publicBones[0]==AsciiString("muzzle"),"original public bone normalization and deduplication");
  require(info.m_pristineBones.empty() && info.m_validStuff==0,"admission does not partially publish original bone caches");
  const auto counts=owner.counts();info.m_animations.front()=W3DAnimationInfo(AsciiString("Tree.Absent"),false,0);
  rejects([&]{(void)info.prepareNativeBinding(owner,1);});require(owner.counts()==counts,"missing first clip preserves complete accepted cache");
  Assets cold(context.files,"English");rejects([&]{(void)info.prepareNativeBinding(cold,1);});require(cold.counts()==Assets::Counts{},"missing clip retires newly acquired cold model graph");
  info.m_animations.front()=W3DAnimationInfo(AsciiString("OtherTree.OtherWalk"),false,0);
  rejects([&]{(void)info.prepareNativeBinding(cold,1);});require(cold.counts()==Assets::Counts{},"incompatible named hierarchy retires both graphs");
  info.m_animations.front()=W3DAnimationInfo(AsciiString(),false,0);rejects([&]{(void)info.prepareNativeBinding(cold,1);});
  require(cold.counts()==Assets::Counts{},"an authored empty first clip is not silent no-animation success");
  info=condition();rejects([&]{(void)info.prepareNativeBinding(owner,std::numeric_limits<float>::quiet_NaN());});
  ModelConditionInfo empty;empty.m_animations=info.m_animations;require(!empty.prepareNativeBinding(owner,1).model,"source no-model state does not load incidental clips");
  info.m_modelName="NULL";info.m_animations.clear();require(info.prepareNativeBinding(owner,1).model->kind==Assets::Prototype::Kind::Null,"explicit null condition is valid without a fake hierarchy");
  std::vector<ModelConditionInfo> states(3);for(auto& state:states)state.m_conditionsYesVec.emplace_back();
  states[1].m_conditionsYesVec[0].set(MODELCONDITION_MOVING);
  ModelConditionFlags alias;alias.set(MODELCONDITION_ATTACKING);states[1].m_conditionsYesVec.push_back(alias);
  states[2].m_conditionsYesVec[0].set(MODELCONDITION_MOVING);states[2].m_conditionsYesVec[0].set(MODELCONDITION_DAMAGED);
  SparseMatchFinder<ModelConditionInfo,ModelConditionFlags> finder;ModelConditionFlags bits;
  require(finder.findBestInfo(states,bits)==&states[0],"source empty condition winner");bits.set(MODELCONDITION_MOVING);
  require(finder.findBestInfo(states,bits)==&states[1],"source fewest extraneous bits tie-break");bits.set(MODELCONDITION_DAMAGED);
  require(finder.findBestInfo(states,bits)==&states[2],"source most intersecting conditions");bits.clear();bits.set(MODELCONDITION_ATTACKING);
  require(finder.findBestInfo(states,bits)==&states[1],"source aliases resolve to same original condition");
  auto copied=states;auto copiedFinder=finder;require(copiedFinder.findBestInfo(copied,bits)==&copied[1],"copied matcher resolves copied condition storage");
  states[1].m_conditionsYesVec[0].set(MODELCONDITION_NIGHT);states[1].m_conditionsYesVec[1].set(MODELCONDITION_SNOW);
  require(states[1].matchesMode(true,false) && states[1].matchesMode(false,true) && !states[1].matchesMode(true,true),"source night/snow mode uses each actual alias");
  owner.clear();last.pose.evaluate(NativeW3DPose::identity(),last.animation->motion.get(),1);require(last.pose.bone(2).transform[3]==20,"condition binding survives cache retirement");
}
void conditionFaults(Context& context,bool seeded=false){auto info=condition();info.m_flags=1<<4;std::size_t terminal;
  {Assets measured(context.files,"English");if(seeded)(void)measured.model("Static.Mesh");AllocationFault::arm(std::numeric_limits<std::size_t>::max());
    try{auto binding=info.prepareNativeBinding(measured,1);terminal=AllocationFault::attempts();AllocationFault::disarm();require(binding.pose.bone(2).transform[3]==20,"complete condition census");}
    catch(...){AllocationFault::disarm();throw;}}
  require(terminal>0 && terminal<512,"bounded condition binding census");
  for(std::size_t ordinal=0;ordinal<=terminal;++ordinal){const auto outer=AllocationFault::live();NativeUnits units;
    {Assets owner(context.files,"English");Assets::Model retained;if(seeded)retained=owner.model("Static.Mesh");const auto counts=owner.counts();const auto live=AllocationFault::live();bool failed=false;
      AllocationFault::arm(ordinal);try{(void)info.prepareNativeBinding(owner,1);}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
      const auto hit=AllocationFault::triggered();const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
      require(failed==(ordinal<terminal) && failed==hit && attempts==(failed?ordinal+1:terminal),"exact condition failure prefixes and terminal");
      if(failed){require(owner.counts()==counts && AllocationFault::live()==live,"late condition failure retains whole accepted cache and retires candidate");
        if(seeded)require(owner.model("Static.Mesh")==retained,"failed coupled condition admission retains accepted populated model identity");}
      require(info.m_pristineBones.empty() && info.m_validStuff==0,"condition failure cannot publish original bone cache");units.check();
      auto retry=info.prepareNativeBinding(owner,1);require(retry.pose.bone(2).transform[3]==20,"every condition failure/terminal has exact same-owner retry");units.check();
    }require(AllocationFault::live()==outer,"whole condition binding owner retirement");units.check();
  }std::cout<<"condition binding "<<(seeded?"seeded":"cold")<<" prefixes [0,"<<terminal<<"); terminal "<<terminal<<"; every retry\n";
}
void warm(Context& context){graph(context);routing(context);cache(context);conditions(context);}
}
int main(int argc,char** argv){bool initialized=false;try{require(argc==2,"asset family");initMemoryManager();initialized=true;
  {Context context;warm(context);for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();NativeUnits units;const std::string_view family=argv[1];
    if(family=="graph")graph(context);else if(family=="routing")routing(context);else if(family=="negative")negative(context);else if(family=="cache")cache(context);
    else if(family=="lifetime")lifetime();else if(family=="archives"){Context archived(true);warm(archived);graph(archived);routing(archived);}
    else if(family=="cold_model")faults(context,false,false);else if(family=="cold_animation")faults(context,true,false);
    else if(family=="seeded_model")faults(context,false,true);else if(family=="seeded_animation")faults(context,true,true);
    else if(family=="conditions")conditions(context);else if(family=="condition_faults")conditionFaults(context);
    else if(family=="condition_seeded_faults")conditionFaults(context,true);else throw std::runtime_error("unknown asset family");
    require(AllocationFault::live()==live,"repeated entire source asset cohort retires standard ownership");units.check();}}
  shutdownMemoryManager();initialized=false;std::cout<<"PASS W3D asset graph; original draw consumers and N2 pending\n";return 0;
 }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode){std::cerr<<"FAIL source error\n";}
 AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
