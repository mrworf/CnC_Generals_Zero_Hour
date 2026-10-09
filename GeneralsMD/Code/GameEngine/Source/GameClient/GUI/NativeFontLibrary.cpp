// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeFontLibrary.h"
#include "GameClient/GlobalLanguage.h"
#include "Common/FileOwner.h"
#include "Common/FileSystem.h"
#include "Common/INI.h"
#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SYNTHESIS_H
#include FT_TRUETYPE_TABLES_H
#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <string>

namespace {
void valid(bool value) { if(!value) throw ERROR_BAD_ARG; }
using Bytes=std::vector<FT_Byte>;
struct Library {
  FT_Library value=nullptr;
  Library() { valid(FT_Init_FreeType(&value)==0); }
  ~Library() { if(value)FT_Done_FreeType(value); }
  Library(const Library&)=delete;
};
struct Face {
  // The handle dies before its borrowed memory, including moved local records.
  std::shared_ptr<const Bytes> bytes;
  FT_Face value=nullptr;
  Bool local=FALSE, embolden=FALSE;
  Face()=default;
  Face(Face&& other) noexcept:bytes(std::move(other.bytes)),value(other.value),local(other.local),embolden(other.embolden) { other.value=nullptr; }
  Face& operator=(Face&& other) noexcept {
    if(this!=&other) { if(value)FT_Done_Face(value);bytes=std::move(other.bytes);value=other.value;local=other.local;embolden=other.embolden;other.value=nullptr; }
    return *this;
  }
  ~Face() { if(value)FT_Done_Face(value); }
  Face(const Face&)=delete;
};
using Config=std::unique_ptr<FcConfig,decltype(&FcConfigDestroy)>;
using Pattern=std::unique_ptr<FcPattern,decltype(&FcPatternDestroy)>;
using Set=std::unique_ptr<FcFontSet,decltype(&FcFontSetDestroy)>;
struct FontData { Face primary,alternate;Int ascent=0; };
Int pixels(FT_Pos value) {
  const auto result=std::llround(static_cast<double>(value)/64.0);
  valid(result>=std::numeric_limits<Int>::min() && result<=std::numeric_limits<Int>::max());
  return static_cast<Int>(result);
}
std::shared_ptr<const Bytes> suppliedBytes(const AsciiString& path) {
  FileCloseOwner file(TheFileSystem->openFile(path.str(),File::READ|File::BINARY));
  if(!file)throw INI_CANT_OPEN_FILE;
  const auto length=file->size();valid(length>0);
  auto bytes=std::make_shared<Bytes>(static_cast<std::size_t>(length));
  valid(file->read(bytes->data(),length)==length);return bytes;
}
}

struct NativeFontLibrary::Impl {
  AcquisitionObserver observer;void* context;
  Library library;
  Config config{nullptr,FcConfigDestroy};
  std::vector<Face> locals;
  // Font-set patterns may borrow metadata faces; retire the set first.
  Set localFonts{nullptr,FcFontSetDestroy};
  AsciiString alternateName;
  explicit Impl(AcquisitionObserver callback,void* data):observer(callback),context(data) {
    acquired();
    config.reset(FcInitLoadConfigAndFonts());valid(bool(config));acquired();
    localFonts.reset(FcFontSetCreate());valid(bool(localFonts));acquired();
    alternateName=TheGlobalLanguageData->m_unicodeFontName;
    for(const auto& name:TheGlobalLanguageData->m_localFonts) {
      auto bytes=suppliedBytes(name);FT_Long count=1;
      for(FT_Long index=0;index<count;++index) {
        Face face;face.bytes=bytes;face.local=TRUE;
        valid(FT_New_Memory_Face(library.value,bytes->data(),bytes->size(),index,&face.value)==0);acquired();
        if(!index) { count=face.value->num_faces;valid(count>0 && count<=65536); }
        // This metadata is from retained actual VFS bytes, not an invented
        // host path. The public FT_FACE pattern property identifies its owner.
        Pattern pattern(FcFreeTypeQueryFace(face.value,reinterpret_cast<const FcChar8*>("supplied-memory-font"),index,nullptr),FcPatternDestroy);
        valid(bool(pattern));acquired();
        valid(FcPatternAddFTFace(pattern.get(),FC_FT_FACE,face.value));
        // Retain the face before the set admits its borrowed identity.
        locals.push_back(std::move(face));
        valid(FcFontSetAdd(localFonts.get(),pattern.get()));pattern.release();
      }
    }
  }
  void acquired() { if(observer)observer(context); }
  Face match(const AsciiString& name,Int point,Bool bold,Bool condensed) {
    Pattern query(FcPatternCreate(),FcPatternDestroy);valid(bool(query));acquired();
    valid(FcPatternAddString(query.get(),FC_FAMILY,reinterpret_cast<const FcChar8*>(name.str())));
    valid(FcPatternAddInteger(query.get(),FC_WEIGHT,bold?FC_WEIGHT_BOLD:FC_WEIGHT_NORMAL));
    valid(FcPatternAddDouble(query.get(),FC_PIXEL_SIZE,point*96.0/72.0));
    valid(FcConfigSubstitute(config.get(),query.get(),FcMatchPattern));FcDefaultSubstitute(query.get());
    FcFontSet* sets[]{localFonts.get(),FcConfigGetFonts(config.get(),FcSetSystem)};
    FcResult result=FcResultNoMatch;
    Pattern selected(FcFontSetMatch(config.get(),sets,sets[1]?2:1,query.get(),&result),FcPatternDestroy);
    valid(bool(selected) && result==FcResultMatch);acquired();
    Face output;FT_Face metadata=nullptr;
    if(FcPatternGetFTFace(selected.get(),FC_FT_FACE,0,&metadata)==FcResultMatch) {
      const auto source=std::find_if(locals.begin(),locals.end(),[&](const Face& face){return face.value==metadata;});
      valid(source!=locals.end());output.bytes=source->bytes;output.local=TRUE;
      valid(FT_New_Memory_Face(library.value,output.bytes->data(),output.bytes->size(),metadata->face_index,&output.value)==0);
    } else {
      FcChar8* file=nullptr;int index=0;
      valid(FcPatternGetString(selected.get(),FC_FILE,0,&file)==FcResultMatch);
      valid(FcPatternGetInteger(selected.get(),FC_INDEX,0,&index)==FcResultMatch && index>=0);
      // Host fallback is stock matching only. Private selectors never escape.
      valid(FT_New_Face(library.value,reinterpret_cast<const char*>(file),index,&output.value)==0);
    }
    acquired();
    const auto emHeight=static_cast<FT_UInt>(std::lround(point*96.0/72.0));
    if(FT_Set_Pixel_Sizes(output.value,0,emHeight)!=0) {
      valid(output.value->num_fixed_sizes>0);Int closest=0;
      for(Int i=1;i<output.value->num_fixed_sizes;++i)
        if(std::abs(output.value->available_sizes[i].height-Int(emHeight))<std::abs(output.value->available_sizes[closest].height-Int(emHeight)))closest=i;
      valid(FT_Select_Size(output.value,closest)==0);
    }
    if(condensed) {
      const auto* os2=static_cast<const TT_OS2*>(FT_Get_Sfnt_Table(output.value,ft_sfnt_os2));
      valid(os2 && os2->xAvgCharWidth>0 && output.value->units_per_EM>0 && FT_IS_SCALABLE(output.value));
      const auto requestedWidth=static_cast<FT_UInt>(emHeight*0.40);
      valid(requestedWidth>0);
      const auto emWidth=static_cast<FT_UInt>(std::lround(double(requestedWidth)*output.value->units_per_EM/os2->xAvgCharWidth));
      valid(emWidth>0 && FT_Set_Pixel_Sizes(output.value,emWidth,emHeight)==0);
    }
    output.embolden=bold && !(output.value->style_flags&FT_STYLE_FLAG_BOLD);
    return output;
  }
};

NativeFontLibrary::NativeFontLibrary(AcquisitionObserver callback,void* context):m_observer(callback),m_context(context) {}
NativeFontLibrary::~NativeFontLibrary() { deleteAllFonts();m_impl.reset(); }
void NativeFontLibrary::init() {
  if(m_impl)return;
  if(!TheFileSystem || !TheGlobalLanguageData)throw ERROR_BAD_ARG;
  auto candidate=std::make_unique<Impl>(m_observer,m_context);
  m_impl=std::move(candidate);
}
void NativeFontLibrary::reset() { deleteAllFonts(); }
bool NativeFontLibrary::initialized() const noexcept { return bool(m_impl); }
std::size_t NativeFontLibrary::localFaceCount() const noexcept { return m_impl?m_impl->locals.size():0; }
bool NativeFontLibrary::owns(const GameFont* font) const noexcept {
  for(auto* owned=m_fontList;owned;owned=owned->next)if(owned==font)return true;
  return false;
}
Int NativeFontLibrary::extraOverlap(const GameFont* font) const {
  valid(m_impl && owns(font));
  return std::clamp(Int(std::lround(font->pointSize*96.0/72.0))/8,0,4);
}
Bool NativeFontLibrary::loadFontData(GameFont* font) {
  if(!m_impl || !font || font->nameString.isEmpty() || font->pointSize<=0 || font->pointSize>100)throw ERROR_BAD_ARG;
  auto data=std::make_unique<FontData>();
  const Bool condensed=font->nameString==AsciiString("Generals");
  data->primary=m_impl->match(condensed?AsciiString("Arial"):font->nameString,font->pointSize,font->bold,condensed);
  if(m_impl->alternateName.isNotEmpty())data->alternate=m_impl->match(m_impl->alternateName,font->pointSize,font->bold,FALSE);
  const auto& metrics=data->primary.value->size->metrics;
  const auto height=std::int64_t(pixels(metrics.ascender))-pixels(metrics.descender);
  valid(height>0 && height<=std::numeric_limits<Int>::max());
  data->ascent=pixels(metrics.ascender);
  font->height=height; font->fontData=data.release();return TRUE;
}
void NativeFontLibrary::releaseFontData(GameFont* font) { delete static_cast<FontData*>(font->fontData);font->fontData=nullptr; }

NativeFontLibrary::Glyph NativeFontLibrary::readGlyph(const GameFont* font,WideChar character) {
  valid(m_impl && font);
  auto* owned=m_fontList;while(owned && owned!=font)owned=owned->next;valid(owned && owned->fontData);
  const auto code=static_cast<std::int64_t>(character);
  valid(code>=0 && code<=0x10ffff && !(code>=0xd800 && code<=0xdfff));
  auto& data=*static_cast<FontData*>(owned->fontData);
  const Bool alternate=code>=256 && data.alternate.value;
  auto& source=alternate?data.alternate:data.primary;
  valid(FT_Load_Char(source.value,code,FT_LOAD_DEFAULT)==0);m_impl->acquired();
  if(source.embolden)FT_GlyphSlot_Embolden(source.value->glyph);
  valid(FT_Render_Glyph(source.value->glyph,FT_RENDER_MODE_NORMAL)==0);
  const auto slot=source.value->glyph;const auto& bitmap=slot->bitmap;
  valid(bitmap.width<=UnsignedInt(std::numeric_limits<Int>::max()) && bitmap.rows<=UnsignedInt(std::numeric_limits<Int>::max()));
  const auto advance=std::int64_t(pixels(slot->advance.x))+(code=='W'?1:0);
  valid(advance>=std::numeric_limits<Int>::min() && advance<=std::numeric_limits<Int>::max());
  Glyph glyph;glyph.advance=static_cast<Int>(advance);
  glyph.width=bitmap.width;glyph.height=bitmap.rows;glyph.left=slot->bitmap_left;glyph.top=slot->bitmap_top;
  glyph.lineHeight=font->height;glyph.ascent=data.ascent;glyph.alternate=alternate;glyph.local=source.local;
  if(!bitmap.width || !bitmap.rows)return glyph;
  valid(bitmap.buffer && (bitmap.pixel_mode==FT_PIXEL_MODE_GRAY || bitmap.pixel_mode==FT_PIXEL_MODE_MONO || bitmap.pixel_mode==FT_PIXEL_MODE_BGRA));
  const auto rowBytes=bitmap.pixel_mode==FT_PIXEL_MODE_MONO?(std::uint64_t(bitmap.width)+7)/8:std::uint64_t(bitmap.width)*(bitmap.pixel_mode==FT_PIXEL_MODE_BGRA?4:1);
  const auto stride=bitmap.pitch<0?-std::int64_t(bitmap.pitch):std::int64_t(bitmap.pitch);
  valid(stride>=std::int64_t(rowBytes) && std::uint64_t(stride)*bitmap.rows<=std::uint64_t(std::numeric_limits<std::ptrdiff_t>::max()));
  const auto count=std::uint64_t(bitmap.width)*bitmap.rows;valid(count<=glyph.coverage.max_size());
  glyph.coverage.resize(static_cast<std::size_t>(count));
  for(unsigned y=0;y<bitmap.rows;++y) {
    const auto* row=bitmap.buffer+std::ptrdiff_t(y)*bitmap.pitch;
    for(unsigned x=0;x<bitmap.width;++x) {
      auto& pixel=glyph.coverage[std::size_t(y)*bitmap.width+x];
      if(bitmap.pixel_mode==FT_PIXEL_MODE_MONO)pixel=(row[x/8]&(0x80>>(x%8)))?255:0;
      else if(bitmap.pixel_mode==FT_PIXEL_MODE_BGRA)pixel=row[std::size_t(x)*4+3];
      else { valid(bitmap.num_grays>1);pixel=std::uint32_t(row[x])*255/(bitmap.num_grays-1); }
    }
  }
  return glyph;
}
