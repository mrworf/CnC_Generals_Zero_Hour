// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeHeadlessGameClient.h"
#include "GameClient/Drawable.h"
#include "GameClient/RayEffect.h"
#include <memory>

Drawable* NativeHeadlessGameClient::friend_createDrawable(const ThingTemplate* definition,DrawableStatus status)
{
  if(TheGameClient!=this || !TheRayEffects)throw ERROR_BAD_ARG;
  if(!definition)return nullptr; // Original source factory's null-template contract.
  return newInstance(Drawable)(definition,status);
}
void NativeHeadlessGameClient::createRayEffectByTemplate(const Coord3D* start,const Coord3D* end,const ThingTemplate* definition)
{
  if(!start || !end || !definition)throw ERROR_BAD_ARG;
  Drawable* drawable=friend_createDrawable(definition,DRAWABLE_STATUS_NONE);
  auto retire=[this](Drawable* value){destroyDrawable(value);};
  std::unique_ptr<Drawable,decltype(retire)> candidate(drawable,retire);
  const Coord3D midpoint{(end->x-start->x)*0.5f+start->x,(end->y-start->y)*0.5f+start->y,(end->z-start->z)*0.5f+start->z};
  drawable->setPosition(&midpoint);
  TheRayEffects->addRayEffect(drawable,start,end);
  candidate.release();
}
