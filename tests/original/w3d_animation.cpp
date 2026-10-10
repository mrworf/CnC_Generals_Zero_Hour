// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/NativeW3DAnimation.h"
#include "Common/NativeW3DPose.h"
#include "Common/Errors.h"
#include "Common/GameMemory.h"
#include "Common/FileSystem.h"
#include "Common/FileOwner.h"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string_view>
#include <unistd.h>
namespace {
using Data=std::vector<std::uint8_t>;
using Animation=NativeW3DAnimation;
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void close(float a,float b,const char* message){require(std::fabs(a-b)<0.0001f,message);}
void word(Data& out,std::uint32_t value){for(unsigned n=0;n<4;++n)out.push_back(std::uint8_t(value>>(8*n)));}
void half(Data& out,unsigned value){out.push_back(std::uint8_t(value));out.push_back(std::uint8_t(value>>8));}
void real(Data& out,float value){word(out,std::bit_cast<std::uint32_t>(value));}
void name(Data& out,std::string_view value,std::size_t size=16){require(value.size()<size,"generated name");out.insert(out.end(),value.begin(),value.end());out.resize(out.size()+size-value.size(),0);}
Data join(std::initializer_list<Data> items){Data out;for(const auto& item:items)out.insert(out.end(),item.begin(),item.end());return out;}
Data chunk(unsigned type,const Data& input,bool nested=false){Data out;word(out,type);word(out,std::uint32_t(input.size())|(nested?0x80000000:0));out.insert(out.end(),input.begin(),input.end());return out;}
void set(Data& out,std::size_t at,std::uint32_t value){require(at+4<=out.size(),"generated extent");for(unsigned n=0;n<4;++n)out[at+n]=std::uint8_t(value>>(8*n));}
Data raw(unsigned type,unsigned pivot,unsigned first,unsigned last,std::initializer_list<float> samples){Data out;
  half(out,first);half(out,last);half(out,type==6?4:1);half(out,type);half(out,pivot);half(out,0);for(float value:samples)real(out,value);return chunk(0x202,out);}
Data rawBits(unsigned pivot,unsigned first,unsigned last,bool defaultValue,std::initializer_list<unsigned> bits){Data out;
  half(out,first);half(out,last);half(out,0);half(out,pivot);out.push_back(defaultValue);for(auto value:bits)out.push_back(std::uint8_t(value));return chunk(0x203,out);}
Data sparse(unsigned type,unsigned pivot,std::initializer_list<unsigned> times,std::initializer_list<float> samples){Data out;
  word(out,times.size());half(out,pivot);const unsigned width=type==6?4:1;out.push_back(width);out.push_back(type);
  auto value=samples.begin();require(samples.size()==times.size()*width,"generated sparse backing");for(auto time:times){word(out,time);for(unsigned n=0;n<width;++n)real(out,*value++);}return chunk(0x282,out);}
Data sparseBits(unsigned pivot){Data out;word(out,3);half(out,pivot);out.push_back(0);out.push_back(0);for(auto time:{0x80000000u,2u,0x80000003u})word(out,time);return chunk(0x283,out);}
Data delta(unsigned type,unsigned pivot,unsigned frames,float scale,unsigned filter,std::uint8_t nibbles){Data out;
  word(out,frames);half(out,pivot);unsigned width=type==6?4:1;out.push_back(width);out.push_back(type);real(out,scale);
  for(unsigned component=0;component<width;++component)real(out,type==6?(component==3?1.f:0.f):10.f);
  for(unsigned packet=0;packet<(frames-1+15)/16;++packet)for(unsigned component=0;component<width;++component){out.push_back(filter);
    for(unsigned n=0;n<8;++n)out.push_back(type==6 && component==3?0:nibbles);}
  return chunk(0x282,out);}
Data file(bool compressed,unsigned flavor,unsigned frames,const Data& channels,bool legacy=false){Data header;
  word(header,legacy?0x20000:0x40001);name(header,"GeneratedMotion");name(header,"GeneratedTree");word(header,frames);
  if(compressed){half(header,30);half(header,flavor);}else word(header,30);
  return chunk(compressed?0x280:0x200,join({chunk(compressed?0x281:0x201,header),channels,chunk(0xeeee,{5,4,3})}),true);}
NativeW3DDocument::Hierarchy hierarchy(){NativeW3DDocument::Hierarchy out;out.name="generatedtree";out.version=0x40001;out.pivots.resize(3);
  for(auto& pivot:out.pivots)pivot.rotation[3]=1;
  out.pivots[0].name="Root";out.pivots[0].translation[0]=99;
  out.pivots[1].name="Arm";out.pivots[1].parent=0;out.pivots[1].translation[0]=10;
  out.pivots[1].rotation={0,0,std::sqrt(0.5f),std::sqrt(0.5f)};
  out.pivots[2].name="Muzzle";out.pivots[2].parent=1;out.pivots[2].translation[0]=4;return out;}
Data rawFile(){return file(false,0,4,join({raw(0,1,1,2,{10,20}),raw(1,1,0,3,{0,2,4,6}),
  raw(6,1,0,3,{0,0,0,1, 0,0,0,1, 0,0,1,0, 0,0,1,0}),rawBits(1,1,2,true,{1}),raw(0,8,0,0,{999})}));}
Data sparseFile(){return file(true,0,4,join({sparse(0,1,{0,2,0x80000003u},{0,20,30}),
  sparse(6,1,{0,2,3},{0,0,0,1, 0,0,1,0, 0,0,-1,0}),sparseBits(1)}));}
Data deltaFile(){return file(true,1,34,join({delta(0,1,34,0.5f,8,0xf1),delta(1,1,34,2.f,16,0x11),
  delta(6,1,34,0.01f,8,0x11),sparseBits(1)}));}
Animation admit(const Data& input){auto document=NativeW3DDocument::decode(input);return Animation::decode(document,0,hierarchy());}
template<class F>void rejects(F operation){bool caught=false;try{operation();}catch(ErrorCode){caught=true;}require(caught,"invalid motion rejected");}
void rawTests(){auto animation=admit(rawFile());require(animation.pivots()==3 && animation.frames()==4 && animation.frameRate()==30,"resolved owned animation metadata");
  close(animation.sample(1,0.5f).translation[0],0,"retail nearest/even lower tie");close(animation.sample(1,1.5f).translation[0],20,"retail nearest/even upper tie");
  close(animation.sample(1,0.75f).translation[0],10,"Generals raw rounded path is not interpolated");
  close(animation.sample(1,0.75f,Animation::RawSampling::Interpolated).translation[0],7.5f,"generic raw interpolated path");
  close(animation.sample(1,3.75f,Animation::RawSampling::Interpolated).translation[1],1.5f,"raw last-to-first interpolation");
  close(animation.sample(1,3.75f).translation[1],0,"rounded terminal wraps root frame");
  require(animation.sample(1,0).visible && animation.sample(1,1).visible && !animation.sample(1,2).visible && animation.sample(1,3).visible,"raw visibility/default boundaries");
  require(animation.sample(2,2).rotation==std::array<float,4>{0,0,0,1},"absent quaternion is identity");
  auto interpolated=animation.sample(1,1.5f,Animation::RawSampling::Interpolated);close(interpolated.rotation[2],std::sqrt(0.5f),"raw quaternion source slerp");
  close(interpolated.rotation[3],std::sqrt(0.5f),"raw quaternion source slerp w");
  auto legacy=admit(file(false,0,4,join({raw(0,0,0,3,{3,4,5,6}),rawBits(0,0,3,false,{15})}),true));
  close(legacy.sample(1,0).translation[0],3,"pre3.0 raw pivot shift");close(legacy.sample(0,0).translation[0],0,"source root index remains distinct");
  auto duplicate=admit(file(false,0,1,join({raw(0,1,0,0,{4}),raw(0,1,0,0,{9})})));close(duplicate.sample(1,0).translation[0],9,"source last channel wins without leaking prior storage");
  auto trailing=raw(0,1,0,0,{12});set(trailing,4,20);trailing.resize(28,0xa5);auto extra=admit(file(false,0,1,trailing));close(extra.sample(1,0).translation[0],12,"raw exporter trailing payload accepted");
  auto bits=admit(file(false,0,12,rawBits(1,1,10,false,{0x81,2})));
  require(!bits.sample(1,0).visible && bits.sample(1,1).visible && bits.sample(1,8).visible && !bits.sample(1,9).visible && bits.sample(1,10).visible && !bits.sample(1,11).visible,"packed visibility crosses byte boundary and retains defaults");
  auto bitReplacement=admit(file(false,0,1,join({rawBits(1,0,0,false,{0}),rawBits(1,0,0,false,{1})})));
  require(bitReplacement.sample(1,0).visible,"duplicate visibility retains last source value and retires prior owner");
}
void sparseTests(){auto animation=admit(sparseFile());close(animation.sample(1,1).translation[0],10,"sparse linear interpolation");
  close(animation.sample(1,2.75f).translation[0],20,"next-key binary flag holds earlier sample");close(animation.sample(1,3.75f).translation[0],30,"sparse terminal holds last key");
  require(animation.sample(1,0).visible && !animation.sample(1,2).visible && animation.sample(1,3).visible,"time-coded visibility high bits");
  close(animation.sample(1,1).rotation[2],std::sqrt(0.5f),"sparse quaternion slerp");close(animation.sample(1,2.5f).rotation[2],1,"quaternion hemisphere flip");
  for(float frame:{3.5f,0.5f,2.5f,1.5f,0.f}){const float expected=frame>=3?30:frame>=2?20:frame*10;close(animation.sample(1,frame).translation[0],expected,"random sparse seek is independent of cache history");}
  auto binary=admit(file(true,0,4,sparse(6,1,{0,0x80000002u},{0,0,0,1,0,0,1,0})));
  require(binary.sample(1,1.75f).rotation==std::array<float,4>{0,0,0,1},"next-key binary flag also holds quaternion motion");
  auto one=admit(file(true,0,4,sparse(0,1,{0},{42})));close(one.sample(1,3.75f).translation[0],42,"single sparse key remains usable at terminal");
  auto extended=admit(file(true,0,4,sparse(0,1,{0,4},{0,40})));close(extended.sample(1,3).translation[0],30,"source sparse channel can extend beyond clip duration");
}
void deltaTests(){auto animation=admit(deltaFile());for(unsigned frame=0;frame<34;++frame){auto sample=animation.sample(1,float(frame));
    close(sample.translation[0],frame%2?10.5f:10.f,"low/high signed delta nibbles across packets");
    close(sample.translation[1],10.f+2.f*frame,"filter16 and multiple delta packets");
    close(sample.rotation[0],0.01f*frame,"all quaternion components reconstructed across packets");close(sample.rotation[1],0.01f*frame,"delta vector component1 independent reconstruction");
    close(sample.rotation[3],1,"delta quaternion w backing");}
  close(animation.sample(1,32.5f).translation[0],10.25f,"delta fractional translation interpolation");
  close(animation.sample(1,33.75f).translation[0],10.5f,"delta terminal clamps rather than wraps");
  for(unsigned frame:{32u,1u,33u,16u,0u,17u})close(animation.sample(1,float(frame)).translation[1],10.f+2.f*frame,"delta random seek independent of source cache defects");
  auto filter=admit(file(true,1,2,delta(0,1,2,1.f,136,0x11)));close(filter.sample(1,1).translation[0],10.f+(1.f-std::sin(3.141592654f/4)),"source generated filter-table selection");
  auto shortened=admit(file(true,1,4,delta(0,1,2,1.f,8,0x11)));close(shortened.sample(1,3).translation[0],11,"source adaptive channel owns its own frame count and clamps independently");
}
void negativeTests(){auto accepted=admit(rawFile());const auto name=accepted.name();
  auto bad=[&](const Data& input){auto document=NativeW3DDocument::decode(input);const auto live=AllocationFault::live();rejects([&]{accepted.replace(document,0,hierarchy());});
    require(accepted.name()==name && AllocationFault::live()==live,"malformed channel retires candidate/preserves accepted owner");close(accepted.sample(1,1).translation[0],10,"old accepted motion still usable");};
  bad(file(false,0,4,raw(0,1,0,3,{1})));bad(file(false,0,4,raw(0,1,2,1,{1})));bad(file(false,0,4,raw(6,1,0,0,{1})));
  bad(file(true,0,4,sparse(0,1,{1,2},{1,2})));bad(file(true,0,4,sparse(0,1,{0,0},{1,2})));bad(file(true,0,4,sparse(0,1,{0,2,1},{1,2,3})));
  auto nonfinite=raw(0,1,0,0,{0});set(nonfinite,8+12,0x7fc00000);bad(file(false,0,4,nonfinite));
  auto truncated=delta(6,1,34,1,8,0x11);truncated.resize(truncated.size()-1);set(truncated,4,truncated.size()-8);bad(file(true,1,34,truncated));
  auto overflow=delta(0,1,34,std::numeric_limits<float>::max(),15,0x77);bad(file(true,1,34,overflow));
  for(const auto& complete:{raw(0,1,0,2,{1,2,3}),sparse(0,1,{0,2},{1,2}),delta(0,1,4,1,8,0x11),rawBits(1,0,3,false,{15})}){
    const unsigned type=unsigned(complete[0])|(unsigned(complete[1])<<8);const bool compressed=type==0x282;
    for(std::size_t length=0;length<complete.size()-8;++length){Data prefix(complete.begin()+8,complete.begin()+8+length);
      bad(file(compressed,type==0x282 && complete[8]==4?1:0,4,chunk(type,prefix)));}}
  auto wrong=hierarchy();wrong.name="unrelated";auto document=NativeW3DDocument::decode(rawFile());rejects([&]{accepted.replace(document,0,wrong);});
  for(float frame:{-1.f,4.f,std::numeric_limits<float>::infinity(),std::bit_cast<float>(0x7fc00000u)})rejects([&]{(void)accepted.sample(1,frame);});
  rejects([&]{(void)accepted.sample(3,0);});rejects([&]{(void)Animation::decode(document,99,hierarchy());});
  accepted.replace(document,0,hierarchy());close(accepted.sample(1,1).translation[0],10,"corrected same-owner admission retry");
}
void poseTests(){using Pose=NativeW3DPose;auto pose=Pose::decode(hierarchy());auto root=Pose::identity();root[3]=100;root[7]=200;
  require(pose.findBone("MUZZLE")==2 && pose.findBone("root")==0 && !pose.findBone("missing") && pose.sourceBoneIndex("missing")==0,"case-insensitive lookup distinguishes real root from source miss0");
  auto motion=admit(rawFile());pose.evaluate(root);
  close(pose.bone(0).transform[3],100,"external root replaces authored pivot0");close(pose.bone(2).transform[3],110,"rotated parent/base composition x");close(pose.bone(2).transform[7],204,"rotated parent/base composition y");
  pose.evaluate(root,&motion,1);close(pose.bone(2).transform[3],108,"animation translation uses base local orientation");close(pose.bone(2).transform[7],214,"child inherits actual animated parent");
  pose.evaluate(root,&motion,1,2);close(pose.bone(2).transform[3],116,"authored and animated translations share scale");close(pose.bone(2).transform[7],228,"scaled muzzle inherits full parent transform");
  pose.capture(1);auto control=Pose::identity();control[3]=3;pose.control(1,control,false);pose.evaluate(root);
  close(pose.bone(2).transform[3],110,"captured local translation rotates with bone");close(pose.bone(2).transform[7],207,"capture applied before child composition");
  pose.control(1,control,true);pose.evaluate(root);close(pose.bone(2).transform[3],113,"world capture applies unrotated translation");close(pose.bone(2).transform[7],204,"world capture preserves base child rotation");
  auto pristine=pose.pristineBone(2,root);close(pristine[3],110,"pristine query ignores capture");close(pristine[7],204,"pristine query retains authored chain");
  pose.evaluate(root,&motion,2);require(pose.bone(1).visible,"captured bone forces animated visibility");pose.release(1);pose.evaluate(root,&motion,2);require(!pose.bone(1).visible && pose.bone(2).visible,"source bone visibility is not inherited from parent");
  pose.evaluateBlend(root,motion,0.75f,motion,0.75f,0.5f);close(pose.bone(1).transform[3],108.5f,"blend uses generic raw interpolation rather than rounded path");close(pose.bone(1).transform[7],207.5f,"blend local translation source order");
  pose.evaluateBlend(root,motion,1,motion,2,0);require(pose.bone(1).visible,"blend visibility OR includes both motions even at percentage0");
  const auto prior=pose.bone(2);const auto* view=&pose.bone(2);auto explosive=root;explosive[0]=std::numeric_limits<float>::max();rejects([&]{pose.evaluate(explosive);});
  require(&pose.bone(2)==view && pose.bone(2).transform==prior.transform && pose.bone(2).visible==prior.visible,"late numeric rejection preserves entire accepted pose/view");
  rejects([&]{pose.evaluate(root,&motion,4);});rejects([&]{pose.control(1,control,false);});rejects([&]{pose.capture(99);});
  rejects([&]{(void)pose.pristineBone(99,root);});rejects([&]{pose.evaluateBlend(root,motion,0,motion,0,1.1f);});
  auto wrong=hierarchy();wrong.name="other";auto incompatible=Pose::decode(wrong);rejects([&]{incompatible.evaluate(root,&motion,1);});
  auto cyclic=hierarchy();cyclic.pivots[2].parent=2;rejects([&]{pose.replace(cyclic);});
  const auto live=AllocationFault::live();AllocationFault::arm(0);try{for(float frame:{0.f,1.f,2.f,3.75f,1.5f}){pose.evaluate(root,&motion,frame);(void)pose.pristineBone(2,root);}
    pose.evaluateBlend(root,motion,1,motion,2,0.5f);}catch(...){AllocationFault::disarm();throw;}
  const auto attempts=AllocationFault::attempts();const bool hit=AllocationFault::triggered();AllocationFault::disarm();require(attempts==0 && !hit && AllocationFault::live()==live,"post-admission pose evaluation/blend/pristine query allocate nothing");
  auto moved=std::move(pose);moved.evaluate(root);close(moved.bone(2).transform[7],204,"whole pose move owns hierarchy after borrowed source retirement");
}
void poseFaults(){using Pose=NativeW3DPose;const auto tree=hierarchy();auto pose=Pose::decode(tree);
  AllocationFault::arm(std::numeric_limits<std::size_t>::max());try{pose.replace(tree);}catch(...){AllocationFault::disarm();throw;}
  const auto terminal=AllocationFault::attempts();AllocationFault::disarm();require(terminal>0 && terminal<32,"complete pose owner allocation census");
  for(std::size_t ordinal=0;ordinal<=terminal;++ordinal){const auto live=AllocationFault::live();const auto* view=&pose.bone(2);const auto expected=pose.bone(2);bool failed=false;
    AllocationFault::arm(ordinal);try{pose.replace(tree);}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
    const auto attempts=AllocationFault::attempts();const bool hit=AllocationFault::triggered();AllocationFault::disarm();
    require(failed==(ordinal<terminal) && failed==hit && attempts==(failed?ordinal+1:terminal),"pose exact terminal and every failure prefix");
    if(failed)require(&pose.bone(2)==view && pose.bone(2).transform==expected.transform,"failed whole pose replacement preserves accepted view");
    require(AllocationFault::live()==live,"pose candidate or replaced backing retires exactly");pose.replace(tree);require(AllocationFault::live()==live,"every pose failure/terminal has same-owner corrected retry");
  }std::cout<<"pose prefixes [0,"<<terminal<<"); terminal "<<terminal<<"; every retry\n";
}
void integratedTests(){Animation motion;NativeW3DPose pose;
  {const auto tree=hierarchy();Data header,pivots;word(header,tree.version);name(header,tree.name);word(header,tree.pivots.size());
    for(unsigned n=0;n<3;++n)real(header,0);
    for(const auto& pivot:tree.pivots){name(pivots,pivot.name);word(pivots,pivot.parent);for(float value:pivot.translation)real(pivots,value);
      for(float value:pivot.euler)real(pivots,value);for(float value:pivot.rotation)real(pivots,value);}
    const auto bytes=join({chunk(0x100,join({chunk(0x101,header),chunk(0x102,pivots)}),true),rawFile()});
    struct Root {char path[40]="/tmp/zh-motion-input-XXXXXX";Root(){require(::mkdtemp(path),"generated model mount");}
      ~Root(){char file[80];std::snprintf(file,sizeof(file),"%s/model.w3d",path);::unlink(file);::rmdir(path);}}root;
    {std::ofstream output(std::filesystem::path(root.path)/"model.w3d",std::ios::binary);output.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());require(bool(output),"generated whole W3D file");}
    FileSystem files;files.mountReadOnly({root.path});FileCloseOwner file(files.openFile("model.w3d",File::READ|File::BINARY|File::STREAMING));require(bool(file),"source File admission");
    auto document=NativeW3DDocument::read(*file);require(document.hierarchies().size()==1 && document.animations().size()==1,"actual file parses typed hierarchy and animation");
    motion=Animation::decode(document,0,document.hierarchies()[0]);pose=NativeW3DPose::decode(document.hierarchies()[0]);
  }
  auto root=NativeW3DPose::identity();root[3]=100;root[7]=200;pose.evaluate(root,&motion,1);
  const auto muzzle=pose.findBone("muzzle");require(muzzle==2,"named query after complete borrowed input retirement");
  close(pose.bone(*muzzle).transform[3],108,"actual File-to-owned-model muzzle x");close(pose.bone(*muzzle).transform[7],214,"actual File-to-owned-model muzzle y");
  close(pose.pristineBone(*muzzle,root)[7],204,"pristine query survives original File/document retirement");
}
void faults(){for(const auto& input:{rawFile(),sparseFile(),deltaFile()}){auto document=NativeW3DDocument::decode(input);const auto tree=hierarchy();auto accepted=Animation::decode(document,0,tree);
    AllocationFault::arm(std::numeric_limits<std::size_t>::max());try{accepted.replace(document,0,tree);}catch(...){AllocationFault::disarm();throw;}
    const auto terminal=AllocationFault::attempts();AllocationFault::disarm();require(terminal>0 && terminal<128,"bounded complete animation allocation census");
    for(std::size_t ordinal=0;ordinal<=terminal;++ordinal){const auto live=AllocationFault::live();const auto* name=accepted.name().data();const auto expected=accepted.sample(1,1);bool failed=false;
      AllocationFault::arm(ordinal);try{accepted.replace(document,0,tree);}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
      const auto triggered=AllocationFault::triggered();const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
      require(failed==(ordinal<terminal) && failed==triggered && attempts==(failed?ordinal+1:terminal),"exact animation terminal and every failure prefix");
      if(failed)require(accepted.name().data()==name,"failed replacement preserves borrowed accepted name identity");
      const auto observed=accepted.sample(1,1);require(observed.translation==expected.translation && observed.rotation==expected.rotation && observed.visible==expected.visible && AllocationFault::live()==live,"failed/success replacement preserves motion and retires storage");
      accepted.replace(document,0,tree);require(AllocationFault::live()==live,"every prefix/terminal has exact same-owner corrected retry");
    }std::cout<<"animation prefixes [0,"<<terminal<<"); terminal "<<terminal<<"; every retry\n";}
}
}
int main(int argc,char** argv){bool initialized=false;try{require(argc==2,"animation family");initMemoryManager();initialized=true;
  {auto warm=admit(deltaFile());(void)warm.sample(1,1.5f);integratedTests();}
  for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();const std::string_view family=argv[1];
    if(family=="raw")rawTests();else if(family=="sparse")sparseTests();else if(family=="delta")deltaTests();else if(family=="negative")negativeTests();else if(family=="faults")faults();else if(family=="pose")poseTests();else if(family=="pose_faults")poseFaults();else if(family=="integrated")integratedTests();else throw std::runtime_error("unknown animation family");
    require(AllocationFault::live()==live,"repeated complete animation lifecycle retires all standard storage");}
  shutdownMemoryManager();initialized=false;std::cout<<"PASS CPU motion; model consumers and N2 pending\n";return 0;
 }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode){std::cerr<<"FAIL source error\n";}
 AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
