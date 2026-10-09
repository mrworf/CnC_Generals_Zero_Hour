// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/AsciiString.h"
#include "Common/Debug.h"
#include <array>
#include <cerrno>
#include <cstdlib>
#include <dirent.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/wait.h>
#include <unistd.h>

namespace {
void require(bool value,const char* message) {if(!value) throw std::runtime_error(message);}
struct Descriptor {
  int value=-1;
  ~Descriptor(){if(value>=0) ::close(value);}
  void close(){if(value>=0) ::close(value);value=-1;}
};
enum class Output {Capture,Closed,Broken};
struct Directory {
  char path[40]="/tmp/zh-fatal-XXXXXX";
  Directory(){require(::mkdtemp(path)!=nullptr,"generated fatal working directory");}
  ~Directory(){::rmdir(path);}
  void checkEmpty() const {
    DIR* directory=::opendir(path);require(directory,"generated output directory read");
    bool empty=true;
    while(const auto* entry=::readdir(directory))
      if(std::string_view(entry->d_name)!="." && std::string_view(entry->d_name)!="..") empty=false;
    ::closedir(directory);require(empty,"fatal paths never create raw reports or mutate input files");
  }
};
void callback() {::_exit(91);}
void run(bool localized,bool exhausted,Output mode,bool nullReason=false) {
  Directory directory;
  int descriptors[2];require(::pipe(descriptors)==0,"generated diagnostic pipe");
  Descriptor input{descriptors[0]},output{descriptors[1]};
  if(mode==Output::Broken) input.close();
  // Allocate caller strings before fault arming/fork. No retail paths are used.
  AsciiString prompt("generated-private-selector"),reason("generated/private/root: secret-details\n");
  const auto child=::fork();require(child>=0,"generated fatal child");
  if(child==0) {
    if(::chdir(directory.path)!=0) ::_exit(96);
    input.close();
    if(mode==Output::Closed) ::close(STDERR_FILENO);
    else if(::dup2(output.value,STDERR_FILENO)<0) ::_exit(92);
    output.close();
    if(std::atexit(callback)!=0) ::_exit(93);
    if(exhausted) AllocationFault::arm(0);
    try {
      if(localized) ReleaseCrashLocalized(prompt,reason);
      else ReleaseCrash(nullReason?nullptr:reason.str());
    }catch(...) {::_exit(94);}
    ::_exit(95);
  }
  output.close();
  if(mode==Output::Broken) input.close();
  std::string report;
  if(mode==Output::Capture) {
    std::array<char,256> bytes{};
    for(;;) {
      const auto count=::read(input.value,bytes.data(),bytes.size());
      if(count>0) report.append(bytes.data(),static_cast<std::size_t>(count));
      else if(count<0 && errno==EINTR) continue;
      else {require(count==0,"complete fatal report read");break;}
    }
  }
  int status=0;pid_t waited;
  do {waited=::waitpid(child,&status,0);}while(waited<0 && errno==EINTR);
  require(waited==child && WIFEXITED(status) && WEXITSTATUS(status)==EXIT_FAILURE,
      "fatal entry exits failure without callbacks or allocation escape");
  directory.checkEmpty();
  if(mode==Output::Capture)
    require(report=="Zero Hour: fatal engine error. Runtime state is invalid; restart the game. Private error details withheld.\n",
        "bounded redacted diagnostic never leaks caller data");
}
}
int main(int argc,char** argv) {
  try {
    require(argc==2,"fatal family required");
    const std::string family=argv[1];
    const auto live=AllocationFault::live();
    for(unsigned repeat=0;repeat<3;++repeat) {
      if(family=="reporting") {
        run(false,false,Output::Capture);run(false,false,Output::Capture,true);
        run(true,false,Output::Capture);
      }else if(family=="failures") {
        for(bool localized:{false,true}) {
          run(localized,true,Output::Capture);run(localized,true,Output::Closed);
          run(localized,true,Output::Broken);
        }
      }else throw std::runtime_error("unknown fatal family");
    }
    require(AllocationFault::live()==live,"caller-owned fatal fixture backing retires exactly");
    std::cout<<"PASS native fatal boundary, not original process acceptance\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';return 1;}
}
