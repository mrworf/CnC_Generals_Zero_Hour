// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/GlobalData.h"
#include <array>

// Original authored lighting and the selected active terrain-light row. Captures
// scalar configuration without borrowing a renderer or allocating backing.
struct NativeTerrainLightingState {
  TimeOfDay time;
  std::array<std::array<GlobalData::TerrainLighting,MAX_GLOBAL_LIGHTS>,TIME_OF_DAY_COUNT> terrain,objects;
  std::array<RGBColor,MAX_GLOBAL_LIGHTS> ambient,diffuse;
  std::array<Coord3D,MAX_GLOBAL_LIGHTS> direction;
  explicit NativeTerrainLightingState(const GlobalData& source):time(source.m_timeOfDay) {
    for(unsigned t=0;t<TIME_OF_DAY_COUNT;++t)for(unsigned l=0;l<MAX_GLOBAL_LIGHTS;++l){
      terrain[t][l]=source.m_terrainLighting[t][l];objects[t][l]=source.m_terrainObjectsLighting[t][l];}
    for(unsigned l=0;l<MAX_GLOBAL_LIGHTS;++l){ambient[l]=source.m_terrainAmbient[l];diffuse[l]=source.m_terrainDiffuse[l];direction[l]=source.m_terrainLightPos[l];}
  }
  void publish(GlobalData& target)const noexcept {
    target.m_timeOfDay=time;
    for(unsigned t=0;t<TIME_OF_DAY_COUNT;++t)for(unsigned l=0;l<MAX_GLOBAL_LIGHTS;++l){
      target.m_terrainLighting[t][l]=terrain[t][l];target.m_terrainObjectsLighting[t][l]=objects[t][l];}
    for(unsigned l=0;l<MAX_GLOBAL_LIGHTS;++l){target.m_terrainAmbient[l]=ambient[l];target.m_terrainDiffuse[l]=diffuse[l];target.m_terrainLightPos[l]=direction[l];}
  }
};
