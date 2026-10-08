// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/NativeSourceStrings.h"
#include "Common/NativeCellSpan.h"
#include "Common/DiscreteCircle.h"
#include "Common/Errors.h"
#include "Common/NativeSourceMath.h"
#include <array>
#include <bit>
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace {
void require(bool yes,const char* message) { if (!yes) throw std::runtime_error(message); }
void sourceMath() {
    // Reference operation sequence from unchanged WWMath/matrix3d.cpp's
    // buildTransformMatrix, Rotate_Vector and Get_Z_Rotation. No library member
    // replacement, D3DX implementation or physical renderer is linked here.
    const std::array<Vector3,10> directions{{
        {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1},
        {0.6f,0.8f,0},{-0.6f,0,0.8f},{0,-0.6f,-0.8f},{0,0,0}}};
    const std::array<Vector3,4> positions{{{0,0,0},{13,-17,23},{-1.25f,7.5f,31},{-0.0f,0,-0.0f}}};
    const std::array<Vector3,4> vectors{{{1,0,0},{0,1,0},{0,0,1},{2.5f,-3.25f,7.5f}}};
    std::uint64_t checkpoint=14695981039346656037ull;
    auto append=[&](float value) {
        checkpoint^=std::bit_cast<std::uint32_t>(value);checkpoint*=1099511628211ull;
    };
    auto same=[](float a,float b) {return std::bit_cast<std::uint32_t>(a)==std::bit_cast<std::uint32_t>(b);};
    AllocationFault::arm(0);
    for(const auto& direction:directions) for(const auto& position:positions) {
        Matrix3D actual;
        for(int row=0;row<3;++row) for(int column=0;column<4;++column)
            actual[row][column]=13.25f+float(row*4+column);
        nativeSourceBuildTransformMatrix(actual,position,direction);
        Matrix3D expected(true);
        const float len2=(float)sqrt(direction.X*direction.X + direction.Y*direction.Y);
        const float siny=len2!=0.0f?direction.Y/len2:0.0f;
        const float cosy=len2!=0.0f?direction.X/len2:1.0f;
        expected.Translate(position);expected.Rotate_Z(siny,cosy);expected.Rotate_Y(-direction.Z,len2);
        for(int row=0;row<3;++row) for(int column=0;column<4;++column) {
            require(same(actual[row][column],expected[row][column]),"source-exact complete transform from dirty backing");
            append(actual[row][column]);
        }
        require(actual[0][3]==position.X && actual[1][3]==position.Y && actual[2][3]==position.Z,
            "source yaw/pitch retain authored translation");
        const float yaw=nativeSourceYaw(actual);
        require(same(yaw,WWMath::Atan2(actual[1][0],actual[0][0])),"source yaw double atan2 boundary");append(yaw);
        for(const auto& vector:vectors) {
            const auto rotated=nativeSourceRotateVector(actual,vector);
            const Vector3 expectedVector(
                (expected[0][0]*vector[0]+expected[0][1]*vector[1]+expected[0][2]*vector[2]),
                (expected[1][0]*vector[0]+expected[1][1]*vector[1]+expected[1][2]*vector[2]),
                (expected[2][0]*vector[0]+expected[2][1]*vector[1]+expected[2][2]*vector[2]));
            for(int component=0;component<3;++component) {
                require(same(rotated[component],expectedVector[component]),"source-exact rotation excludes translation");
                append(rotated[component]);
            }
        }
    }
    Matrix3D upward;
    nativeSourceBuildTransformMatrix(upward,Vector3(13,-17,23),Vector3(0,0,1));
    const auto forward=nativeSourceRotateVector(upward,Vector3(1,0,0));
    require(forward.X==0 && forward.Y==0 && forward.Z==1,"known vertical zero-projection fallback");
    Matrix3D zero;
    nativeSourceBuildTransformMatrix(zero,Vector3(1,2,3),Vector3(0,0,0));
    const auto collapsed=nativeSourceRotateVector(zero,Vector3(1,0,0));
    require(collapsed.X==0 && collapsed.Y==0 && collapsed.Z==0,
        "source degenerate direction is not silently reset to identity");
    require(!AllocationFault::triggered() && AllocationFault::attempts()==0,
        "complete source-math operations are allocation free");
    AllocationFault::disarm();
    std::printf("source math checkpoint %016llx\n",static_cast<unsigned long long>(checkpoint));
}
template<class F> void badArgument(F action) {
    bool rejected=false;
    try { action(); } catch (ErrorCode error) { rejected=error==ERROR_BAD_ARG; }
    require(rejected,"defined original error admission");
}
void strings() {
    for (auto name : {"Arial","Times New Roman","Font-With-Hyphen","Name:Colon"}) {
        for (bool bold : {false,true}) {
            const auto label=std::string(name)+" - Size:14"+(bold ? " [Bold]" : "");
            const auto font=parseNativeCinematicFont(label);
            require(font.name==name && font.pointSize==14 && font.bold==bold,"actual editor font encoding");
        }
    }
    const auto max=parseNativeCinematicFont("Font - Size:2147483647");
    require(max.pointSize==std::numeric_limits<Int>::max(),"encoded font Int maximum, not backing capacity");
    require(parseNativeCinematicFont("Font - Size:+14").pointSize==14,"source atoi positive sign");
    const std::string longName(4096,'N');
    require(parseNativeCinematicFont(longName+" - Size:14 [Bold]").name==longName,"no legacy256-byte font buffer");
    for (auto label : {"", "Font", " - Size:14", "Font - Size:", "Font - Size:0", "Font - Size:-1",
                       "Font - Size:2147483648", "Font - Size:14bad", "Font - Size:14 [Unknown]", "Font - Size:14 [bold]"})
        badArgument([&]{ (void)parseNativeCinematicFont(label); });
    const std::string nul("Font - Size:14\0junk",19);
    badArgument([&]{ (void)parseNativeCinematicFont(nul); });
    require(nativeScriptFrames(10)==300 && nativeScriptFrames(-10)==-300,"defined source frame duration");
    require(nativeScriptFrames(71582788)==2147483640 && nativeScriptFrames(-71582788)==-2147483640,
            "exact signed multiplication boundaries");
    for (Int seconds : {71582789,-71582789,std::numeric_limits<Int>::min(),std::numeric_limits<Int>::max()})
        badArgument([&]{ (void)nativeScriptFrames(seconds); });
    for (auto leaf : {"map.ini","solo.ini","map.str","AssetUsage.txt"}) {
        for (auto map : {"Maps/Generated/Generated.map","Maps\\Generated\\Generated.map"})
            require(nativeMapCompanionPath(map,leaf)==std::string("Maps")+map[4]+"Generated\\"+leaf,
                    "source parent separators and companion identity");
        require(nativeMapCompanionPath("root.map",leaf)==leaf,"rooted companion without absolute leading separator");
        const auto path=nativeMapCompanionPath(std::string(4096,'M')+"/map.map",leaf);
        require(path==std::string(4096,'M')+"\\"+leaf,"no MAX_PATH truncation");
    }
    for (auto map : {"", "x", "/map.map", "\\map.map", "C:/map.map"})
        badArgument([&]{ (void)nativeMapCompanionPath(map,"map.ini"); });
    for (auto leaf : {"", ".", "..", "a/b", "a\\b", "a:b"})
        badArgument([&]{ (void)nativeMapCompanionPath("map.map",leaf); });
}
void spans() {
    NativeCellSpan span{91,92,93};
    for (const auto& values : std::array<std::array<Int,5>,8>{{
         {0,3,0,0,0},{3,0,0,0,0},{3,3,0,0,-1},{3,3,0,0,3},
         {3,3,2,1,0},{3,3,3,4,0},{3,3,-4,-1,0},{-1,3,0,0,0}}}) {
        require(!nativeCellSpan(values[0],values[1],values[2],values[3],values[4],span),"invalid source cell span");
        require(span.first==91 && span.last==92 && span.offset==93,"rejection preserves accepted span");
    }
    require(nativeCellSpan(3,3,-5,9,2,span) && span.first==0 && span.last==2 && span.offset==6,
            "clip both sides before pointer arithmetic");
    require(nativeCellSpan(3,3,2,2,2,span) && span.first==2 && span.last==2 && span.offset==8,"last exact cell");
    const auto limit=std::numeric_limits<Int>::max();
    require(nativeCellSpan(limit,limit,limit-1,limit,limit-1,span),"wide index arithmetic admitted");
    require(span.offset==static_cast<std::size_t>(limit)*static_cast<std::size_t>(limit)-1,
            "logical maximum index; no physical backing claim");
}
using Scan=std::array<Int,3>;
struct DrawLog { std::vector<Scan> scans; Int borrowed; };
void collect(Int x1,Int x2,Int y,void* data) {
    auto& log=*static_cast<DrawLog*>(data);
    require(log.borrowed==17,"synchronous borrowed payload preserved");
    log.scans.push_back({x1,x2,y});
}
void circles() {
    for (auto center : {0,-37,std::numeric_limits<Int>::max()-2,std::numeric_limits<Int>::min()+2}) {
        DiscreteCircle circle(center,center,2); DrawLog log{{},17};
        circle.drawCircle(collect,&log);
        const std::vector<Scan> expected{{center-1,center+1,center+2},{center-1,center+1,center-2},
              {center-2,center+2,center+1},{center-2,center+2,center-1},{center-2,center+2,center}};
        require(log.scans==expected,"original discrete scanlines and top/mirror order");
        badArgument([&]{ circle.drawCircle(nullptr,&log); });
        log.scans.clear();circle.drawCircle(collect,&log);require(log.scans==expected,"same-owner callback admission retry");
    }
    DiscreteCircle zero(-3,-4,0); DrawLog log{{},17}; zero.drawCircle(collect,&log);
    require(log.scans==std::vector<Scan>{{-3,-3,-4}},"zero-radius authored center");
    for (const auto& values : std::array<std::array<Int,3>,5>{{{0,0,-1},
         {std::numeric_limits<Int>::min(),0,1},{std::numeric_limits<Int>::max(),0,1},
         {0,std::numeric_limits<Int>::min(),1},{0,std::numeric_limits<Int>::max(),1}}})
        badArgument([&]{ DiscreteCircle invalid(values[0],values[1],values[2]); });
}
void faults() {
    const std::string name(4096,'N'),label=name+" - Size:14 [Bold]",map=name+"/map.map";
    const std::string expectedPath=name+"\\AssetUsage.txt";
    constexpr std::array<std::size_t,3> terminals{1,2,1};
    for (int operation=0;operation<3;++operation) {
        bool terminal=false;
        for (std::size_t ordinal=0;ordinal<16;++ordinal) {
            const auto live=AllocationFault::live();bool rejected=false;
            AllocationFault::arm(ordinal);
            try {
                if (operation==0) { auto font=parseNativeCinematicFont(label);require(font.name==name,"candidate font identity"); }
                else if (operation==1) { auto path=nativeMapCompanionPath(map,"AssetUsage.txt");require(path==expectedPath,"candidate map identity"); }
                else { DiscreteCircle circle(-3,-4,8);require(circle.getEdgeCount()>0,"candidate scanlines"); }
            } catch (const std::bad_alloc&) { rejected=true; }
            catch (...) { AllocationFault::disarm();throw; }
            AllocationFault::disarm();
            require(AllocationFault::live()==live,"all failed/accepted temporary owner storage released");
            if (!AllocationFault::triggered()) {
                require(!rejected && ordinal==terminals[operation],"exact untriggered allocation terminal accepts");
                terminal=true;break;
            }
            require(rejected,"every injected allocation rejects without retained candidate backing");
            if (operation==0) require(parseNativeCinematicFont(label).name==name,"same-input font retry");
            else if (operation==1) require(nativeMapCompanionPath(map,"AssetUsage.txt")==expectedPath,"same-input map retry");
            else { DiscreteCircle circle(-3,-4,8);require(circle.getEdgeCount()>0,"same-input circle retry"); }
        }
        require(terminal,"bounded exact allocation terminal reached");
    }
}
}
int main(int argc,char** argv) {
    try {
        require(argc==2,"select bounded source family");
        for (int repeat=0;repeat<3;++repeat) {
            const auto live=AllocationFault::live();
            if (!std::strcmp(argv[1],"strings")) strings();
            else if (!std::strcmp(argv[1],"spans")) spans();
            else if (!std::strcmp(argv[1],"circles")) circles();
            else if (!std::strcmp(argv[1],"faults")) faults();
            else if (!std::strcmp(argv[1],"math")) sourceMath();
            else throw std::runtime_error("unknown source family");
            require(AllocationFault::live()==live,"same-process source owner teardown");
        }
        std::puts("PASS original source boundaries");return 0;
    } catch (const std::exception& error) { AllocationFault::disarm();std::fprintf(stderr,"FAIL %s\n",error.what()); }
    catch (...) { AllocationFault::disarm();std::fputs("FAIL source boundary exception\n",stderr); }
    return 1;
}
