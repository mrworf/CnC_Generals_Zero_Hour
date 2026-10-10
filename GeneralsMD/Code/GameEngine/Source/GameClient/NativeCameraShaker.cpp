// SPDX-License-Identifier: GPL-3.0-or-later
// C3 camera effect formula adapted from original camerashakesystem.cpp.
#include "PreRTS.h"
#include "GameClient/NativeCameraShaker.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace {
void finiteValue(Real value){if(!std::isfinite(value))throw ERROR_BAD_ARG;}
void position(const Coord3D& value){finiteValue(value.x);finiteValue(value.y);finiteValue(value.z);}
Real randomVisual(Real lo,Real hi){return (Real(std::rand() & 0xFFF)/Real(0xFFF))*(hi-lo)+lo;}
constexpr Real minOmega=(12.5f*360)*PI/180.0f,maxOmega=(15.0f*360)*PI/180.0f,endOmega=360*PI/180.0f;
constexpr Real minPhase=0,maxPhase=360*PI/180.0f;
}
void NativeCameraShaker::add(const Coord3D& point,Real radius,Real duration,Real power){
  position(point);finiteValue(radius);finiteValue(duration);finiteValue(power);
  if(radius<=0 || duration<=0 || power<0)throw ERROR_BAD_ARG;
  // Original Real squared-distance/divisor arithmetic must be defined/finite.
  if(radius>std::sqrt(std::numeric_limits<Real>::max()))throw ERROR_BAD_ARG;
  const Real radians=power*PI/180.0f;finiteValue(radians);
  const double maxPhaseArgument=maxOmega*double(duration)+(maxOmega-endOmega)*double(duration)*duration+maxPhase;
  if(maxPhaseArgument>std::numeric_limits<Real>::max())throw ERROR_BAD_ARG;
  double totalIntensity=radians;for(const auto& record:m_records)totalIntensity+=record.intensity;
  if(totalIntensity>double(std::numeric_limits<Real>::max())/64)throw ERROR_BAD_ARG;
  // The only fallible acquisition precedes the six original RNG draws.
  if(m_records.size()==m_records.max_size())throw ERROR_OUT_OF_MEMORY;
  if(m_records.size()==m_records.capacity()){
    const auto capacity=m_records.capacity();
    const auto next=capacity==0?std::size_t(1):(capacity>m_records.max_size()/2?m_records.max_size():capacity*2);
    m_records.reserve(next);
  }
  Record record;record.position=point;record.radius=radius;record.duration=duration;record.intensity=radians;
  record.omega={randomVisual(minOmega,maxOmega),randomVisual(minOmega,maxOmega),randomVisual(minOmega,maxOmega)};
  record.phase={randomVisual(minPhase,maxPhase),randomVisual(minPhase,maxPhase),randomVisual(minPhase,maxPhase)};
  // Store amortized at the end and accumulate in reverse: equivalent to the
  // original MultiList head admission without shifting every accepted record.
  m_records.push_back(record);
}
void NativeCameraShaker::timestep(Real seconds){
  finiteValue(seconds);if(seconds<0)throw ERROR_BAD_ARG;
  for(auto& record:m_records){
    // Avoid overflowing elapsed when a finite large timestep expires a record.
    if(seconds>=record.duration-record.elapsed)record.elapsed=record.duration;
    else record.elapsed+=seconds;
  }
  std::erase_if(m_records,[](const Record& record){return record.elapsed>=record.duration;});
}
Coord3D NativeCameraShaker::angles(const Coord3D& eye){
  position(eye);Coord3D result{};
  const Real axis[3]={7.5f*PI/180.0f,15.0f*PI/180.0f,5.0f*PI/180.0f};
  for(auto it=m_records.rbegin();it!=m_records.rend();++it){const auto& record=*it;
    const Real dx=eye.x-record.position.x,dy=eye.y-record.position.y,dz=eye.z-record.position.z;
    const Real len2=dx*dx+dy*dy+dz*dz;
    if(len2>record.radius*record.radius)continue;
    const Real intensity=record.intensity*(1-std::sqrt(len2)/record.radius)*(1-record.elapsed/record.duration);
    const Real omega[3]={record.omega.x,record.omega.y,record.omega.z};
    const Real phase[3]={record.phase.x,record.phase.y,record.phase.z};
    Real* angles[3]={&result.x,&result.y,&result.z};
    for(unsigned n=0;n<3;++n){
      const Real frequency=omega[n]+(endOmega-omega[n])*record.elapsed;
      *angles[n]+=axis[n]*intensity*std::sin(frequency*record.elapsed+phase[n]);
      // Preserve the secondary three-vector INSIDE each axis loop (nine draws).
      const Real minor=intensity*.5f;
      result.x+=randomVisual(-minor,minor);result.y+=randomVisual(-minor,minor);result.z+=randomVisual(-minor,minor);
    }
  }
  // Source ignores WWMath::Clamp's return value. No silent visual redesign.
  return result;
}
