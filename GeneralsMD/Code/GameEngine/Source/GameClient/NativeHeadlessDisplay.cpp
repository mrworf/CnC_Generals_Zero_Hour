// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeHeadlessDisplay.h"
#include "GameClient/GameClient.h"
#include "GameClient/ScriptedView.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/ScriptEngine.h"

Bool NativeHeadlessDisplay::advanceLogicalFrame(){
  if(std::this_thread::get_id()!=m_thread)throw ERROR_BAD_ARG;
  if(TheDisplay!=this || !TheGameClient || !TheGameLogic || !TheScriptEngine ||
      TheGameLogic->isInGameLogicUpdate())throw ERROR_BAD_ARG;
  auto* primary=dynamic_cast<ScriptedView*>(getFirstView());
  if(!primary || primary!=TheTacticalView || primary->frameLengthMilliseconds()<=0)throw ERROR_BAD_ARG;
  const Bool frozen=(primary->isTimeFrozen() && !primary->isCameraMovementFinished()) ||
    TheScriptEngine->isTimeFrozenDebug() || TheScriptEngine->isTimeFrozenScript() || TheGameLogic->isGamePaused();
  auto candidate=m_clock;
  const auto step=candidate.advance(TheGameClient->getFrame(),primary->frameLengthMilliseconds(),frozen,TheScriptEngine->isTimeFast());
  if(step.cameraOnly)primary->updateCameraMovements(primary->frameLengthMilliseconds());
  m_clock=candidate;return step.cameraOnly;
}
