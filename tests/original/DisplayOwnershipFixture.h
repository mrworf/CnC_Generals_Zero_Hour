// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/Display.h"
// Real Display View ownership only. Every absent physical provider rejects.
struct DisplayOwnershipFixture:Display {
  using Display::deleteViews;
  void doSmartAssetPurgeAndPreload(const char*)override{throw ERROR_BAD_ARG;}
#if defined(_DEBUG) || defined(_INTERNAL)
  void dumpAssetUsage(const char*)override{throw ERROR_BAD_ARG;}
  void dumpModelAssets(const char*)override{throw ERROR_BAD_ARG;}
#endif
  VideoBuffer* createVideoBuffer()override{throw ERROR_BAD_ARG;}
  void setClipRegion(IRegion2D*)override{throw ERROR_BAD_ARG;}
  Bool isClippingEnabled()override{throw ERROR_BAD_ARG;}
  void enableClipping(Bool)override{throw ERROR_BAD_ARG;}
  void setTimeOfDay(TimeOfDay)override{throw ERROR_BAD_ARG;}
  void createLightPulse(const Coord3D*,const RGBColor*,Real,Real,UnsignedInt,UnsignedInt)override{throw ERROR_BAD_ARG;}
  void drawLine(Int,Int,Int,Int,Real,UnsignedInt)override{throw ERROR_BAD_ARG;}
  void drawLine(Int,Int,Int,Int,Real,UnsignedInt,UnsignedInt)override{throw ERROR_BAD_ARG;}
  void drawOpenRect(Int,Int,Int,Int,Real,UnsignedInt)override{throw ERROR_BAD_ARG;}
  void drawFillRect(Int,Int,Int,Int,UnsignedInt)override{throw ERROR_BAD_ARG;}
  void drawRectClock(Int,Int,Int,Int,Int,UnsignedInt)override{throw ERROR_BAD_ARG;}
  void drawRemainingRectClock(Int,Int,Int,Int,Int,UnsignedInt)override{throw ERROR_BAD_ARG;}
  void drawImage(const Image*,Int,Int,Int,Int,Color,DrawImageMode)override{throw ERROR_BAD_ARG;}
  void drawVideoBuffer(VideoBuffer*,Int,Int,Int,Int)override{throw ERROR_BAD_ARG;}
  void setShroudLevel(Int,Int,CellShroudStatus)override{throw ERROR_BAD_ARG;}
  void clearShroud()override{throw ERROR_BAD_ARG;}
  void setBorderShroudLevel(UnsignedByte)override{throw ERROR_BAD_ARG;}
  void preloadModelAssets(AsciiString)override{throw ERROR_BAD_ARG;}
  void preloadTextureAssets(AsciiString)override{throw ERROR_BAD_ARG;}
  void takeScreenShot()override{throw ERROR_BAD_ARG;}
  void toggleMovieCapture()override{throw ERROR_BAD_ARG;}
  void toggleLetterBox()override{throw ERROR_BAD_ARG;}
  void enableLetterBox(Bool)override{throw ERROR_BAD_ARG;}
  Real getAverageFPS()override{throw ERROR_BAD_ARG;}
  Int getLastFrameDrawCalls()override{throw ERROR_BAD_ARG;}
};
