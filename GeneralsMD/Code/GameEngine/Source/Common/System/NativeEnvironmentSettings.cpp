// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "Common/NativeEnvironmentSettings.h"
#include "GameClient/TerrainVisual.h"
#include <type_traits>

NativeEnvironmentSettings::NativeEnvironmentSettings():
  m_priorWater(TheWaterTransparency.getNonOverloadedPointer()),
  m_priorWeather(TheWeatherSetting.getNonOverloadedPointer()) {
  if(TheSnowManager || TheTerrainVisual)throw ERROR_BAD_ARG;
  for(unsigned n=0;n<TIME_OF_DAY_COUNT;++n)m_saved[n]=WaterSettings[n];
  TheWaterTransparency=nullptr;TheWeatherSetting=nullptr;
}
void NativeEnvironmentSettings::capture() noexcept {
  m_water=const_cast<WaterTransparencySetting*>(TheWaterTransparency.getNonOverloadedPointer());
  m_weather=const_cast<WeatherSetting*>(TheWeatherSetting.getNonOverloadedPointer());
}
void NativeEnvironmentSettings::activate() noexcept {
  TheWaterTransparency=m_water;TheWeatherSetting=m_weather;
}
NativeEnvironmentSettings::~NativeEnvironmentSettings() noexcept {
  activate();
  auto retire=[](auto* head)noexcept{
    while(head){
      auto* next=static_cast<std::remove_pointer_t<decltype(head)>*>(head->friend_getNextOverride());
      head->friend_restoreOverrideOwnership(nullptr,head->friend_isOverride());
      head->deleteInstance();head=next;
    }
  };
  retire(m_water);retire(m_weather);
  TheWaterTransparency=m_priorWater;TheWeatherSetting=m_priorWeather;
  for(unsigned n=0;n<TIME_OF_DAY_COUNT;++n)nativeExchangeWaterSetting(WaterSettings[n],m_saved[n]);
}
