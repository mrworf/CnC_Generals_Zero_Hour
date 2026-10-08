// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/BezierSegment.h"
#include "Common/BezFwdIterator.h"
#include "Common/GameMemory.h"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace {
void require(bool yes, const char* message) {
    if (!yes) throw std::runtime_error(message);
}
void near(const Coord3D& actual, const Coord3D& expected, float tolerance = 0.0002f) {
    require(std::abs(actual.x-expected.x) <= tolerance &&
            std::abs(actual.y-expected.y) <= tolerance &&
            std::abs(actual.z-expected.z) <= tolerance, "source cubic coordinates");
}
const std::array<Coord3D,4> points{{{2,4,-6},{5,-3,1},{-2,7,3},{9,2,-1}}};
Coord3D oracle(const std::array<Coord3D,4>& p, double t) {
    // Independent Bernstein form in double, not the implementation's matrix.
    const double u = 1-t;
    const std::array<double,4> w{u*u*u,3*u*u*t,3*u*t*t,t*t*t};
    double x=0,y=0,z=0;
    for (std::size_t i=0; i<4; ++i) { x+=p[i].x*w[i]; y+=p[i].y*w[i]; z+=p[i].z*w[i]; }
    return {static_cast<float>(x),static_cast<float>(y),static_cast<float>(z)};
}
BezierSegment curve() { return {points[0],points[1],points[2],points[3]}; }
void evaluate() {
    auto segment=curve();
    std::array<Real,12> packed{2,4,-6,5,-3,1,-2,7,3,9,2,-1};
    BezierSegment fromPacked(packed.data());
    auto coordinates=points;
    BezierSegment fromCoordinates(coordinates.data());
    BezierSegment fromScalars(2,4,-6,5,-3,1,-2,7,3,9,2,-1);
    BezierSegment zero;
    for (Real t : {-1.0f,0.0f,0.125f,0.5f,0.875f,1.0f,2.0f}) {
        for (const auto* owner : {&segment,&fromPacked,&fromCoordinates,&fromScalars}) {
            Coord3D value{99,99,99}; owner->evaluateBezSegmentAtT(t,&value);
            near(value,oracle(points,t));
        }
        Coord3D value{99,99,99}; zero.evaluateBezSegmentAtT(t,&value); near(value,{0,0,0});
    }
    segment.evaluateBezSegmentAtT(0.5f,nullptr); // Original explicit no-op.
    BezierSegment line({0,0,0},{1,2,2},{2,4,4},{3,6,6});
    require(line.getApproximateLength() == 9.0f, "straight source length");
    require(zero.getApproximateLength() == 0.0f, "degenerate source length");
    const Real rough=segment.getApproximateLength(1.0f), fine=segment.getApproximateLength(0.01f);
    require(std::isfinite(rough) && std::isfinite(fine) && fine>0 && std::abs(rough-fine)<1,
            "source recursive length refinement");
}
void subdivision() {
    auto segment=curve();
    for (Real split : {0.0f,0.25f,0.5f,0.75f,1.0f}) {
        BezierSegment left,right; segment.splitSegmentAtT(split,left,right);
        for (Real t : {0.0f,0.25f,0.5f,0.75f,1.0f}) {
            Coord3D actual{}; left.evaluateBezSegmentAtT(t,&actual); near(actual,oracle(points,t*split));
            right.evaluateBezSegmentAtT(t,&actual); near(actual,oracle(points,split+t*(1-split)));
        }
    }
}
void sampling() {
    auto segment=curve();
    for (Int count : {0,1,2,3,8,33}) {
        VecCoord3D values{{99,99,99}};
        segment.getSegmentPoints(count,&values);
        require(values.size() == static_cast<std::size_t>(count), "exact source sample count");
        for (Int i=0;i<count;++i) near(values[i],oracle(points,count==1 ? 0 : double(i)/(count-1)),0.005f);
        BezFwdIterator iterator(count,&segment);
        for (Int repeat=0;repeat<3;++repeat) {
            iterator.start(); Int seen=0;
            while (!iterator.done()) { near(iterator.getCurrent(),values[seen++]); iterator.next(); }
            require(seen==count,"same-owner iterator restart");
        }
    }
    VecCoord3D prior{{91,92,93}}; auto* backing=prior.data();
    bool rejected=false;
    try { segment.getSegmentPoints(-1,&prior); } catch (ErrorCode error) { rejected=error==ERROR_BAD_ARG; }
    require(rejected && prior.data()==backing && prior.size()==1,"negative count before publication");
    near(prior.front(),{91,92,93},0);
    for (Int count : {-1,0,1}) {
        bool bad=false;
        try { BezFwdIterator invalid(count,nullptr); } catch (ErrorCode error) { bad=error==ERROR_BAD_ARG; }
        require(bad,"null curve admission");
    }
    segment.getSegmentPoints(-1,nullptr); // Preserve original absent-output no-op.
}
void faults() {
    auto segment=curve();
    for (std::size_t ordinal=0;ordinal<=1;++ordinal) {
        VecCoord3D values{{91,92,93}}; auto* backing=values.data();
        const auto live=AllocationFault::live(); bool rejected=false;
        AllocationFault::arm(ordinal);
        try { segment.getSegmentPoints(33,&values); }
        catch (const std::bad_alloc&) { rejected=true; }
        catch (...) { AllocationFault::disarm(); throw; }
        AllocationFault::disarm();
        if (ordinal==0) {
            require(rejected && AllocationFault::triggered() && AllocationFault::live()==live,
                    "failed sample allocation releases all candidate storage");
            require(values.data()==backing && values.size()==1,"accepted samples retain backing");
            near(values.front(),{91,92,93},0);
            segment.getSegmentPoints(33,&values);
        } else require(!rejected && !AllocationFault::triggered(),"exact one-allocation terminal");
        require(values.size()==33,"corrected same-owner sampling retry");
        near(values.front(),points.front()); near(values.back(),points.back(),0.005f);
    }
}
}
int main(int argc,char** argv) {
    try {
        require(argc==2,"select bounded math family");
        for (int repeat=0;repeat<3;++repeat) {
            const auto live=AllocationFault::live();
            if (!std::strcmp(argv[1],"evaluate")) evaluate();
            else if (!std::strcmp(argv[1],"subdivision")) subdivision();
            else if (!std::strcmp(argv[1],"sampling")) sampling();
            else if (!std::strcmp(argv[1],"faults")) faults();
            else throw std::runtime_error("unknown math family");
            require(AllocationFault::live()==live,"complete same-process math teardown");
        }
        std::puts("PASS original cubic owner"); return 0;
    } catch (const std::exception& error) { AllocationFault::disarm(); std::fprintf(stderr,"FAIL %s\n",error.what()); }
    catch (...) { AllocationFault::disarm(); std::fputs("FAIL original math exception\n",stderr); }
    return 1;
}
