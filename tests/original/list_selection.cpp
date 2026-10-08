// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "GameClient/NativeListBoxSelection.h"
#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
namespace {
void require(bool value,const char* text) { if (!value) throw std::runtime_error(text); }
struct Fixture {
    std::array<ListEntryCell,4> cells{};
    std::array<ListEntryRow,4> rows{};
    std::array<Int,5> selections{-1,-1,-1,-1,-1};
    ListboxData list{};
    Fixture() {
        for (Int i=0;i<4;++i) rows[i].cell=&cells[i];
        list.listLength=4; list.endPos=4; list.columns=1;
        list.listData=rows.data(); list.selections=selections.data();
        list.multiSelect=true; list.selectPos=-1;
    }
};
void functional() {
    Fixture f;
    const Int all[]{0,1,2,3};
    require(nativeListBoxSetSelection(f.list,all,4),"full capacity selection");
    require(f.selections==std::array<Int,5>{0,1,2,3,-1},"full capacity terminal");
    Int* borrowed=nullptr;
    require(nativeListBoxGetSelection(f.list,1,&borrowed) && borrowed==f.selections.data(),"exact borrowed backing");
    nativeListBoxRemoveSelection(f.list,1);
    require(f.selections==std::array<Int,5>{0,2,3,-1,-1},"overlap-safe bounded removal");
    require(nativeListBoxToggleSelection(f.list,1) && f.selections[4]==-1,"toggle reaches full capacity with terminal");
    nativeListBoxScrollSelection(f.list,2);
    require(f.selections[0]==0 && f.selections[1]==1 && f.selections[2]==-1,"scroll drops retired rows and remaps survivors");
    nativeListBoxClearSelection(f.list);
    for (Int value:f.selections) require(value==-1,"complete clear including terminal");
    f.list.multiSelect=false;
    const Int one=3;
    require(nativeListBoxSetSelection(f.list,&one,1),"single scalar selection");
    struct { Int value; Int canary; } output{-1,1234};
    require(nativeListBoxGetSelection(f.list,0,&output.value) && output.value==3 && output.canary==1234,"scalar width and canary");
    nativeListBoxScrollSelection(f.list,4);
    require(f.list.selectPos==-1,"retired scalar clears");
    f.list.multiSelect=true;
    NativeListBoxBacking grown(f.list,6);
    require(grown.rows[3].cell==&f.cells[3] && !grown.rows[4].cell && !grown.rows[5].cell,"retained owners borrowed and new rows initialized");
    require(grown.selections[6]==-1,"grown terminal initialized");
    NativeListBoxBacking empty(f.list,0);
    require(empty.selections[0]==-1,"zero capacity has terminal");
    NativeListBoxBacking maximum(f.list,std::numeric_limits<Short>::max());
    require(maximum.selections[std::numeric_limits<Short>::max()]==-1
        && !maximum.rows[std::numeric_limits<Short>::max()-1].cell,"exact source capacity maximum");
    Fixture insert;
    insert.list.endPos=3; insert.rows[3]={};
    const Int selected[]{0,2};
    require(nativeListBoxSetSelection(insert.list,selected,2),"insert selection baseline");
    AllocationFault::arm(0);
    const bool inserted=nativeListBoxInsertRow(insert.list,1);
    const auto attempts=AllocationFault::attempts(); AllocationFault::disarm();
    require(inserted && attempts==0 && !insert.rows[1].cell
        && insert.rows[2].cell==&insert.cells[1] && insert.rows[3].cell==&insert.cells[2]
        && insert.selections[1]==3 && insert.list.endPos==4,"in-place row insertion preserves each owner and remaps selections without allocation");
    require(!nativeListBoxInsertRow(insert.list,1) && !nativeListBoxInsertRow(insert.list,-1)
        && insert.rows[3].cell==&insert.cells[2],"full/negative insertion rejects unchanged");
}
void negative() {
    Fixture f;
    const Int all[]{0,1,2,3};
    require(nativeListBoxSetSelection(f.list,all,4),"commit-ready selection baseline");
    const auto before=f.selections;
    const Int lateNegative[]{1,-2},lateBoundary[]{1,4},lateHole[]{1,2};
    require(!nativeListBoxSetSelection(f.list,lateNegative,2),"late negative rejected");
    require(!nativeListBoxSetSelection(f.list,lateBoundary,2),"late bound plus one rejected");
    f.rows[2].cell=nullptr;
    require(!nativeListBoxSetSelection(f.list,lateHole,2),"late uninitialized row rejected");
    f.rows[2].cell=&f.cells[2];
    require(!nativeListBoxSetSelection(f.list,nullptr,1)
        && !nativeListBoxSetSelection(f.list,all,0)
        && !nativeListBoxSetSelection(f.list,all,5)
        && !nativeListBoxSetSelection(f.list,all,WindowMsgData{1}<<32),"count/pointer admission before read or narrowing");
    require(f.selections==before,"all rejections preserve commit-ready backing");
    struct {Int value; Int canary;} output{17,23};
    require(!nativeListBoxGetSelection(f.list,0,&output.value) && output.value==17 && output.canary==23,"multi state cannot widen scalar output");
    require(!nativeListBoxGetSelection(f.list,2,&output) && !nativeListBoxGetSelection(f.list,1,nullptr),"unknown/null output rejected");
    require(!nativeListBoxToggleSelection(f.list,-1) && !nativeListBoxToggleSelection(f.list,4),"toggle bounds");
    for (Int length:{-1,32768}) {
        bool rejected=false;
        try { NativeListBoxBacking candidate(f.list,length); }
        catch(const std::invalid_argument&) { rejected=true; }
        require(rejected && f.selections==before,"capacity rejected before allocation or accepted mutation");
    }
    require(nativeListBoxSetSelection(f.list,all,4),"same-owner corrected selection retry");
    AllocationFault::arm(0);
    nativeListBoxRemoveSelection(f.list,0);
    const bool toggled=nativeListBoxToggleSelection(f.list,0);
    nativeListBoxScrollSelection(f.list,1);
    const auto attempts=AllocationFault::attempts();
    AllocationFault::disarm();
    require(toggled && attempts==0,"selection ordering and shifts allocate nothing");
}
void faults() {
    Fixture f;
    constexpr std::size_t census=2; // rows + capacity/terminal selection backing
    for (std::size_t ordinal=0;ordinal<=census;++ordinal) {
        const auto live=AllocationFault::live();
        bool failed=false;
        AllocationFault::arm(ordinal);
        try { NativeListBoxBacking candidate(f.list,6); }
        catch(const std::bad_alloc&) { failed=true; }
        catch(...) { AllocationFault::disarm(); throw; }
        const auto attempts=AllocationFault::attempts();
        const auto triggered=AllocationFault::triggered();
        AllocationFault::disarm();
        require(AllocationFault::live()==live && f.rows[3].cell==&f.cells[3]
            && f.selections[0]==-1,"failed constructor preserves actual accepted ownership");
        require(ordinal<census ? failed && triggered : !failed && !triggered && attempts==census,"complete two-allocation manifest and exact terminal");
        { NativeListBoxBacking retry(f.list,6); require(retry.rows[3].cell==&f.cells[3],"same-owner backing retry"); }
        require(AllocationFault::live()==live,"retry teardown exact");
    }
}
}
int main(int argc,char** argv) {
    try {
        require(argc==2,"widget selection family");
        const std::string family(argv[1]);
        for (int repeat=0;repeat<3;++repeat) {
            const auto live=AllocationFault::live();
            if (family=="functional") functional();
            else if (family=="negative") negative();
            else if (family=="faults") faults();
            else throw std::runtime_error("unknown widget selection family");
            require(AllocationFault::live()==live,"three complete selection-owner lifetimes");
        }
        std::cout<<"PASS actual list metadata/selection ownership (window callbacks pending)\n";
        return 0;
    } catch(const std::exception& error) { std::cerr<<"FAIL "<<error.what()<<'\n'; }
    AllocationFault::disarm(); return 1;
}
