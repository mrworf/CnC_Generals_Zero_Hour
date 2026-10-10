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

#include "W3DAssetInput.h"
namespace {
using namespace W3DAssetInput;
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
  auto stale=owner.model("Stale");require(stale->levels[0].objects[0].bone==0,"source stale positive LOD bone clamps to root");
  auto stalePose=Assets::createPose(stale);require(stalePose.bone(0).transform[3]==0,"stale attachment uses the real root transform");
  auto aggregate=owner.model("AggReject");require(aggregate->aggregates.size()==1&&aggregate->aggregates[0].bone==2&&aggregate->aggregates[0].model==aggregate->levels[0].objects[0].model,"source aggregate rejects stale/negative nodes while retaining valid child identity");
}
void negative(Context& context){context.root.write("Art/W3D/Broken.w3d",hlod("Broken","Tree",{{"Missing.Part",1}}));
  Assets owner(context.files,"English");auto accepted=owner.model("Outer");auto clip=owner.animation("Tree.Walk");const auto counts=owner.counts();
  auto bad=[&](auto action){const auto live=AllocationFault::live();NativeUnits units;rejects(action);
    require(owner.counts()==counts && owner.model("outer")==accepted && owner.animation("tree.walk")==clip && AllocationFault::live()==live,"rejected graph preserves every accepted cache identity/count and retires candidate backing");units.check();};
  for(const auto* name:{"CycleA","Broken","BadBone","BadProxy","BadSkin","BadTree","WrongName","Malformed","NoFile"})bad([&]{(void)owner.model(name);});
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
