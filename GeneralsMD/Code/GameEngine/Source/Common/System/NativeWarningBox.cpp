// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "Common/NativeWarningBox.h"
#include "Common/AsciiString.h"
#include "GameClient/GameText.h"
#include <SDL3/SDL.h>
#include <cstdint>

namespace {
std::string utf8(const UnicodeString& text) {
  std::string result;
  for(Int i=0;i<text.getLength();++i){
    const auto value=static_cast<std::uint32_t>(text.getCharAt(i));
    if(value==0 || value>0x10ffff || (value>=0xd800 && value<=0xdfff))throw ERROR_BAD_ARG;
    if(value<=0x7f)result.push_back(static_cast<char>(value));
    else if(value<=0x7ff){result.push_back(static_cast<char>(0xc0|(value>>6)));result.push_back(static_cast<char>(0x80|(value&63)));}
    else if(value<=0xffff){result.push_back(static_cast<char>(0xe0|(value>>12)));result.push_back(static_cast<char>(0x80|((value>>6)&63)));result.push_back(static_cast<char>(0x80|(value&63)));}
    else {result.push_back(static_cast<char>(0xf0|(value>>18)));result.push_back(static_cast<char>(0x80|((value>>12)&63)));result.push_back(static_cast<char>(0x80|((value>>6)&63)));result.push_back(static_cast<char>(0x80|(value&63)));}
  }return result;
}
OSDisplayButtonType sdlWarning(const NativeWarningRequest& request,void*) {
  if(!SDL_IsMainThread())return OSDBT_ERROR;
  SDL_MessageBoxButtonData buttons[2]{};int count=0;
  if(request.buttons&OSDBT_OK)buttons[count++]={SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,OSDBT_OK,"OK"};
  if(request.buttons&OSDBT_CANCEL)buttons[count++]={SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,OSDBT_CANCEL,"Cancel"};
  SDL_MessageBoxData data{};data.title=request.title.c_str();data.message=request.message.c_str();
  data.numbuttons=count;data.buttons=buttons;
  // Original icon constants overlap; retain the original error/stop precedence
  // over information, with warning otherwise. Modal scope is platform-managed.
  data.flags=(request.options&0x10)?SDL_MESSAGEBOX_ERROR:
      (request.options&OSDOF_EXCLAMATIONICON)?SDL_MESSAGEBOX_WARNING:SDL_MESSAGEBOX_INFORMATION;
  int selected=-1;
  if(!SDL_ShowMessageBox(&data,&selected))return OSDBT_ERROR;
  if(selected==-1 && (request.buttons&OSDBT_CANCEL))return OSDBT_CANCEL;
  if(selected==OSDBT_OK && (request.buttons&OSDBT_OK))return OSDBT_OK;
  if(selected==OSDBT_CANCEL && (request.buttons&OSDBT_CANCEL))return OSDBT_CANCEL;
  return OSDBT_ERROR;
}
}
OSDisplayButtonType nativeDisplayWarning(GameTextInterface* text,const AsciiString& title,
    const AsciiString& message,UnsignedInt buttons,UnsignedInt options,
    NativeWarningPresenter presenter,void* context) {
  if(!text || !presenter || !buttons || (buttons&~UnsignedInt(OSDBT_OK|OSDBT_CANCEL)) ||
      (options&~UnsignedInt(0x1f)))return OSDBT_ERROR;
  Bool titleExists=FALSE,messageExists=FALSE;
  const auto titleText=text->fetch(title,&titleExists);const auto messageText=text->fetch(message,&messageExists);
  if(!titleExists || !messageExists)return OSDBT_ERROR;
  NativeWarningRequest request{utf8(titleText),utf8(messageText),buttons,options};
  const auto result=presenter(request,context);
  if(result==OSDBT_OK || result==OSDBT_CANCEL){if(!(buttons&UnsignedInt(result)))return OSDBT_ERROR;}
  else return OSDBT_ERROR;
  return result;
}
OSDisplayButtonType OSDisplayWarningBox(AsciiString title,AsciiString message,
    UnsignedInt buttons,UnsignedInt options) {
  return nativeDisplayWarning(TheGameText,title,message,buttons,options,sdlWarning,nullptr);
}
