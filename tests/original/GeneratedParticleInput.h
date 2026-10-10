// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <string>

namespace GeneratedParticleInput {
inline std::string definition(const char* name,const char* slave="",const char* delay="0"){
  std::string out="ParticleSystem ";out+=name;out+="\nShader = ADDITIVE\nType = PARTICLE\nParticleName = generated.dds\nVelocityType = ORTHO\nVolumeType = LINE\nVolLineStart = X:0 Y:0 Z:0\nVolLineEnd = X:0 Y:0 Z:0\n";
  for(const char* field:{"AngleZ","AngularRateZ","AngularDamping","VelocityDamping","Lifetime","Size","StartSizeRate","SizeRate","SizeRateDamping","ColorScale","BurstDelay","BurstCount","VelOrthoX","VelOrthoY","VelOrthoZ"})out+=std::string(field)+" = 0 0 CONSTANT\n";
  out+="InitialDelay = ";out+=delay;out+=' ';out+=delay;out+=" CONSTANT\n";
  for(unsigned i=1;i<=8;++i){out+="Alpha"+std::to_string(i)+" = 0 0 0\n";out+="Color"+std::to_string(i)+" = R:0 G:0 B:0 0\n";}
  if(*slave)out+=std::string("SlaveSystem = ")+slave+'\n';out+="End\n";return out;
}
}
