// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeDisplayString.h"
#include "GameClient/GlobalLanguage.h"
#include <algorithm>
#include <limits>

namespace {
void valid(bool value) { if(!value)throw ERROR_BAD_ARG; }
Int checked(std::int64_t value) {
  valid(value>=std::numeric_limits<Int>::min() && value<=std::numeric_limits<Int>::max());
  return Int(value);
}
}
NativeDisplayString::NativeDisplayString(NativeFontLibrary& fonts):m_fonts(fonts) {}
NativeDisplayString::~NativeDisplayString()=default;
NativeDisplayString::Layout NativeDisplayString::prepare(const UnicodeString& text,GameFont* font,Int wrap,Bool centered,Bool hotkeys) {
  Layout result;if(font)valid(m_fonts.owns(font));
  if(!font || text.isEmpty())return result;
  const auto* characters=text.str();const auto count=text.getLength();
  std::vector<NativeFontLibrary::Glyph> glyphs;
  glyphs.reserve(count);result.advances.reserve(count);result.placements.reserve(count);
  for(Int n=0;n<count;++n) {
    if(characters[n]==L'\n') { glyphs.emplace_back();result.advances.push_back(0); }
    else { glyphs.push_back(m_fonts.readGlyph(font,characters[n]));result.advances.push_back(glyphs.back().advance); }
  }
  const Bool hard=TheGlobalLanguageData && TheGlobalLanguageData->m_useHardWrap;
  valid(font->height>0);
  std::vector<Int> lineWidths(std::size_t(count)+2,0);
  std::int64_t x=0,y=0,maximum=0;
  Int marker=-1,markerX=0,markerY=0;
  auto markerAt=[&](Int n) { return hotkeys && characters[n]==L'&' && n+1<count && characters[n+1]>L' '; };
  for(Int n=0;n<count;++n) {
    Bool hidden=FALSE;
    if(markerAt(n)) { marker=n+1;markerX=checked(x);markerY=checked(y);++n;hidden=TRUE; }
    const auto character=characters[n];const auto advance=result.advances[n];
    const Bool breakWord=hard && wrap>0 && x+advance>=wrap;
    if(character==L' ' || character==L'\n' || breakWord) {
      maximum=std::max(maximum,x);
      if(character==L' ') {
        if(wrap>0) {
          std::int64_t word=advance;
          for(Int next=n+1;next<count && characters[next]>L' ';++next) {
            if(markerAt(next))++next;
            word+=result.advances[next];checked(word);
          }
          if(x+word>=wrap) { lineWidths[std::size_t(y/font->height)]=checked(x);x=0;y=checked(y+font->height); }
        }
      }else { lineWidths[std::size_t(y/font->height)]=checked(x);x=0;y=checked(y+font->height); }
    }
    if(character!=L'\n') {
      if(!hidden)result.placements.push_back({checked(x),checked(y),std::move(glyphs[n]),FALSE});
      x=checked(x+advance);
    }
  }
  maximum=std::max(maximum,x);
  lineWidths[std::size_t(y/font->height)]=checked(x);
  result.width=checked(maximum+m_fonts.extraOverlap(font));result.height=checked(y+font->height);
  if(marker>=0) {
    auto* bold=m_fonts.getFont(font->nameString,font->pointSize,TRUE);valid(bold);
    // Original searchHotKey picks the first marker's following character;
    // sentence traversal records the final marker position.
    Int first=0;while(first<count && characters[first]!=L'&')++first;
    if(first+1<count)result.placements.push_back({markerX,markerY,m_fonts.readGlyph(bold,characters[first+1]),TRUE});
  }
  if(centered && (wrap>0 || std::find(characters,characters+count,L'\n')!=characters+count)) {
    for(auto& placement:result.placements) {
      const auto lineWidth=lineWidths[std::size_t(placement.y/font->height)];
      placement.x=checked(placement.x+std::max<std::int64_t>(0,(std::int64_t(result.width)-lineWidth)/2));
    }
  }
  return result;
}
void NativeDisplayString::adopt(UnicodeString text,GameFont* font,Int wrap,Bool centered,Bool hotkeys,Color color) {
  auto candidate=prepare(text,font,wrap,centered,hotkeys);
  m_textString.swap(text);m_font=font;m_wrap=wrap;m_centered=centered;m_hotkeys=hotkeys;m_hotkeyColor=color;
  std::swap(m_layout,candidate);
}
void NativeDisplayString::setText(UnicodeString text) { if(text!=m_textString)adopt(text,m_font,m_wrap,m_centered,m_hotkeys,m_hotkeyColor); }
void NativeDisplayString::setFont(GameFont* font) { if(font && font!=m_font)adopt(m_textString,font,m_wrap,m_centered,m_hotkeys,m_hotkeyColor); }
void NativeDisplayString::appendChar(WideChar character) { auto text=m_textString;text.concat(character);adopt(text,m_font,m_wrap,m_centered,m_hotkeys,m_hotkeyColor); }
void NativeDisplayString::removeLastChar() { auto text=m_textString;text.removeLastChar();adopt(text,m_font,m_wrap,m_centered,m_hotkeys,m_hotkeyColor); }
void NativeDisplayString::notifyTextChanged() { adopt(m_textString,m_font,m_wrap,m_centered,m_hotkeys,m_hotkeyColor); }
void NativeDisplayString::reset() { m_textString.clear();m_font=nullptr;Layout empty;std::swap(m_layout,empty); }
void NativeDisplayString::setWordWrap(Int width) { valid(width>=0);if(width!=m_wrap)adopt(m_textString,m_font,width,m_centered,m_hotkeys,m_hotkeyColor); }
void NativeDisplayString::setWordWrapCentered(Bool centered) { if(centered!=m_centered)adopt(m_textString,m_font,m_wrap,centered,m_hotkeys,m_hotkeyColor); }
void NativeDisplayString::setUseHotkey(Bool enabled,Color color) { adopt(m_textString,m_font,m_wrap,m_centered,enabled,color); }
void NativeDisplayString::setClipRegion(IRegion2D* region) { valid(region && region->lo.x<=region->hi.x && region->lo.y<=region->hi.y);m_clip=*region; }
void NativeDisplayString::getSize(Int* width,Int* height) { if(width)*width=m_layout.width;if(height)*height=m_layout.height; }
Int NativeDisplayString::getWidth(Int position) {
  valid(position>=-1);std::int64_t width=0;
  const auto count=position==-1?m_layout.advances.size():std::min(m_layout.advances.size(),std::size_t(position));
  for(std::size_t n=0;n<count;++n)width=checked(width+m_layout.advances[n]);return checked(width);
}
void NativeDisplayString::draw(Int x,Int y,Color color,Color shadow) { draw(x,y,color,shadow,1,1); }
void NativeDisplayString::draw(Int,Int,Color,Color,Int,Int) { if(!m_textString.isEmpty())throw ERROR_BAD_ARG; }
