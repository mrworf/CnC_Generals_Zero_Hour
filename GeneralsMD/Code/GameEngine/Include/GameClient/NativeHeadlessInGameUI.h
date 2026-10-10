// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/InGameUI.h"
#include "GameClient/NativeFrameCameraView.h"

// Actual source UI configuration/state and camera phase without physical menus.
// The client/root owns context and must outlive every Display-owned camera.
class NativeHeadlessInGameUI : public InGameUI {
public:
  explicit NativeHeadlessInGameUI(NativeCameraShaker& context):InGameUI(TRUE),m_context(context){}
  void draw()override{throw ERROR_BAD_ARG;}
protected:
  View* createView()override{return new Camera(m_context);}
private:
  class Camera final:public NativeFrameCameraView {
  public:
    using NativeFrameCameraView::NativeFrameCameraView;
    Drawable* pickDrawable(const ICoord2D*,Bool,PickType)override{throw ERROR_BAD_ARG;}
    Int iterateDrawablesInRegion(IRegion2D*,Bool(*)(Drawable*,void*),void*)override{throw ERROR_BAD_ARG;}
    void drawView()override{throw ERROR_BAD_ARG;}
  };
  NativeCameraShaker& m_context;
};
