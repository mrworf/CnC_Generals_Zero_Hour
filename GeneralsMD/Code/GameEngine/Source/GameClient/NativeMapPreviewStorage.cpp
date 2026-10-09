// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeMapPreviewLayout.h"
#include "Common/FileSystem.h"
#include "Common/FileOwner.h"
#include "Common/NativeUserStorage.h"
void nativeCopyMapPreview(FileSystem& files,const NativeUserStorage& storage,
    const AsciiString& input,std::string_view destination){
  FileCloseOwner source(files.openFile(input.str(),File::READ|File::BINARY));
  if(!source)throw ERROR_BAD_ARG;
  const Int length=source->size();if(length<=0)throw ERROR_BAD_ARG;
  auto output=storage.beginWrite(NativeUserArea::Data,destination);
  std::array<unsigned char,65536> bytes{};
  Int remaining=length;
  while(remaining){
    const Int count=std::min(remaining,Int(bytes.size()));
    if(source->read(bytes.data(),count)!=count)throw ERROR_BAD_ARG;
    output->write(bytes.data(),count);remaining-=count;
  }
  (void)output->commit();
}
