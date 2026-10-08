// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only container/consumer audit. No private names, paths or payload output.
#include "Common/FileSystem.h"
#include "Common/FileOwner.h"
#include "GameClient/GameText.h"
#include <cstdio>
#include <memory>
#include <vector>
int main(int argc,char** argv) {
    bool initialized=false;unsigned stage=0,mask=0;
    try {
        if(argc<2)throw ERROR_BAD_ARG;
        initMemoryManager();initialized=true;
        {
            FileSystem files;TheFileSystem=&files;std::vector<std::string> roots;
            for(int index=1;index<argc;++index)roots.emplace_back(argv[index]);
            stage=1;files.mountReadOnly(roots);mask|=1u;
            stage=2;
            if(files.doesFileExist("Data/INI/GameData.ini") || files.doesFileExist("Data/INI/Default/GameData.ini"))mask|=2u;
            for(const auto& query:std::vector<std::pair<const char*,unsigned>>{
                {"Data/INI/Object",4u},{"Art/W3D",8u},{"Maps",16u}}) {
                FilenameList names;files.getFileListInDirectory(query.first,"*",names,TRUE);
                if(!names.empty())mask|=query.second;
            }
            stage=3;std::unique_ptr<GameTextInterface> text(CreateGameTextInterface());text->init();
            // Initialization alone is not proof of a populated catalog.
            if(!text->getStringsWithLabelPrefix("").empty())mask|=32u;
            stage=4;
        }
        TheFileSystem=nullptr;shutdownMemoryManager();initialized=false;
        std::printf("READ_ONLY_DATA_AUDIT stage=%u mask=%u status=%s\n",stage,mask,mask==63u?"PASS":"INCOMPLETE");
        return mask==63u?0:2;
    }catch(...){TheFileSystem=nullptr;if(initialized)shutdownMemoryManager();
        std::printf("READ_ONLY_DATA_AUDIT stage=%u mask=%u status=REJECTED\n",stage,mask);return 1;}
}
