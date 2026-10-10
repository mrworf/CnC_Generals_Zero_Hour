// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/GameClient.h"

// Original logical client, with explicitly absent physical output. This is not
// an interactive renderer and must never be selected for player-facing mode.
class NativeHeadlessGameClient : public GameClient {
public:
  enum class Composition { LogicalServices, DrawablesOnly };
  explicit NativeHeadlessGameClient(Composition mode=Composition::LogicalServices):GameClient(TRUE),m_composition(mode){}
  void init() override;
  Drawable* friend_createDrawable(const ThingTemplate*,DrawableStatus) override;
  void createRayEffectByTemplate(const Coord3D*,const Coord3D*,const ThingTemplate*) override;
  void addScorch(const Coord3D*,Real,Scorches) override {}
  void setTeamColor(Int,Int,Int) override {}
  void adjustLOD(Int) override {}
  void notifyTerrainObjectMoved(Object*) override {}
private:
  Composition m_composition;
  Display* createGameDisplay() override;
  InGameUI* createInGameUI() override;
  GameWindowManager* createWindowManager() override {throw ERROR_BAD_ARG;}
protected:
  FontLibrary* createFontLibrary() override;
  DisplayStringManager* createDisplayStringManager() override;
private:
  VideoPlayerInterface* createVideoPlayer() override {throw ERROR_BAD_ARG;}
  IMEManagerInterface* createIMEManager() override {throw ERROR_BAD_ARG;}
  TerrainVisual* createTerrainVisual() override;
  Keyboard* createKeyboard() override {throw ERROR_BAD_ARG;}
  Mouse* createMouse() override {throw ERROR_BAD_ARG;}
  SnowManager* createSnowManager() override {throw ERROR_BAD_ARG;}
  void setFrameRate(Real) override {}
};
