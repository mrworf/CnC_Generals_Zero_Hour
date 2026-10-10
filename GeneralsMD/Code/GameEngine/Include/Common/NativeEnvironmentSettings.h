// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/Errors.h"
#include "GameClient/Water.h"
#include "GameClient/Snow.h"
#include <array>
#include <utility>

inline void nativeExchangeWaterSetting(WaterSetting& first,WaterSetting& second) noexcept {
  first.m_skyTextureFile.swap(second.m_skyTextureFile);
  first.m_waterTextureFile.swap(second.m_waterTextureFile);
  std::swap(first.m_waterRepeatCount,second.m_waterRepeatCount);
  std::swap(first.m_skyTexelsPerUnit,second.m_skyTexelsPerUnit);
  std::swap(first.m_vertex00Diffuse,second.m_vertex00Diffuse);
  std::swap(first.m_vertex10Diffuse,second.m_vertex10Diffuse);
  std::swap(first.m_vertex11Diffuse,second.m_vertex11Diffuse);
  std::swap(first.m_vertex01Diffuse,second.m_vertex01Diffuse);
  std::swap(first.m_waterDiffuseColor,second.m_waterDiffuseColor);
  std::swap(first.m_transparentWaterDiffuse,second.m_transparentWaterDiffuse);
  std::swap(first.m_uScrollPerMs,second.m_uScrollPerMs);
  std::swap(first.m_vScrollPerMs,second.m_vScrollPerMs);
}

// Actual GameEngine-owned configuration lifetime. Device owners must retire
// before this owner; borrowed prefixes remain isolated through source parsing.
class NativeEnvironmentSettings final {
  const WaterTransparencySetting* m_priorWater;
  const WeatherSetting* m_priorWeather;
  WaterTransparencySetting* m_water=nullptr;
  WeatherSetting* m_weather=nullptr;
  std::array<WaterSetting,TIME_OF_DAY_COUNT> m_saved;
  void capture() noexcept;
public:
  NativeEnvironmentSettings();
  ~NativeEnvironmentSettings() noexcept;
  NativeEnvironmentSettings(const NativeEnvironmentSettings&)=delete;
  NativeEnvironmentSettings& operator=(const NativeEnvironmentSettings&)=delete;
  void activate() noexcept;
  template<class Load>void load(Load action) {
    if(TheWaterTransparency.getNonOverloadedPointer()!=m_water ||
       TheWeatherSetting.getNonOverloadedPointer()!=m_weather)throw ERROR_BAD_ARG;
    try{action();}catch(...){capture();throw;}
    capture();
  }
};
