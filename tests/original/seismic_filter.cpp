// SPDX-License-Identifier: GPL-3.0-or-later
// Actual source filter against a generated grid oracle, not terrain acceptance.
#include "AllocationFault.h"
#include "GameClient/TerrainVisual.h"
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string_view>
namespace {
void require(bool value,const char* label) { if (!value) throw std::runtime_error(label); }
struct CallbackFailure {};
struct Grid : WorldHeightMapInterfaceClass {
    std::array<Real,121> values{};
    unsigned reads=0,writes=0;
    int failRead=-1,failWrite=-1;
    static Real sample(Int x,Int y) { return Real(x)*0.25f-Real(y)*0.1f; }
    Int getBorderSize() override { return 3; }
    Real getSeismicZVelocity(Int x,Int y) const override { return values.at(std::size_t(x+11*y)); }
    Real getBilinearSampleSeismicZVelocity(Int x,Int y) override {
        if (int(reads++)==failRead) throw CallbackFailure{};
        require(x>=0 && x<11 && y>=0 && y<11,"bounded sample"); return sample(x,y);
    }
    void setSeismicZVelocity(Int x,Int y,Real value) override {
        if (int(writes++)==failWrite) throw CallbackFailure{};
        require(x>=0 && x<11 && y>=0 && y<11,"bounded write"); values.at(std::size_t(x+11*y))=value;
    }
};
SeismicSimulationNode node(unsigned radius=3,unsigned life=1) {
    SeismicSimulationNode result; result.m_center={2,2}; result.m_radius=radius;
    result.m_life=life; result.m_magnitude=20; return result;
}
void oracle() {
    DomeStyleSeismicFilter actual;
    SeismicSimulationFilterBase& filter=actual;
    for (unsigned radius=1;radius<=3;++radius) for (unsigned life=1;life<15;++life) {
        Grid grid; const auto input=node(radius,life);
        require(filter.filterCallback(&grid,&input)==SeismicSimulationFilterBase::SEISMIC_STATUS_ACTIVE,"active source filter");
        require(grid.writes==4*radius*radius,"complete source square writes");
        for (int x=0;x<11;++x) for (int y=0;y<11;++y) {
            Real expected=0;
            const int dx=x-5,dy=y-5;
            if (dx>=-int(radius) && dx<int(radius) && dy>=-int(radius) && dy<int(radius)) {
                const Real distance=std::sqrt(Real(dx*dx+dy*dy));
                if (distance<Real(radius)) expected=std::min(9.0f,20.0f/Real(life)*
                    std::cos(distance/Real(radius)*(PI/2))+Grid::sample(x,y));
            }
            require(std::abs(grid.getSeismicZVelocity(x,y)-expected)<0.00001f,"independent dome and cap oracle");
        }
    }
    require(filter.applyGravityCallback(4)==2.5f && filter.applyGravityCallback(-2)==-3.5f,"source gravity");
    Grid grid; auto input=node(3,0);
    require(filter.filterCallback(&grid,&input)==SeismicSimulationFilterBase::SEISMIC_STATUS_ACTIVE && !grid.reads && !grid.writes,"zero-life omission");
    input.m_life=15;
    require(filter.filterCallback(&grid,&input)==SeismicSimulationFilterBase::SEISMIC_STATUS_ZERO_ENERGY && !grid.writes,"expired source energy");
    input.m_life=std::numeric_limits<UnsignedInt>::max();
    require(filter.filterCallback(&grid,&input)==SeismicSimulationFilterBase::SEISMIC_STATUS_ZERO_ENERGY,"large life cannot narrow to active negative");
    input=node(0,1); require(filter.filterCallback(&grid,&input)==SeismicSimulationFilterBase::SEISMIC_STATUS_ACTIVE && !grid.writes,"empty radius source policy");
}
void failures() {
    DomeStyleSeismicFilter actual; SeismicSimulationFilterBase& filter=actual;
    Grid grid; auto input=node();
    require(filter.filterCallback(nullptr,&input)==SeismicSimulationFilterBase::SEISMIC_STATUS_INVALID &&
        filter.filterCallback(&grid,nullptr)==SeismicSimulationFilterBase::SEISMIC_STATUS_INVALID,"missing input rejection");
    for (auto magnitude:{std::numeric_limits<Real>::infinity(),std::numeric_limits<Real>::quiet_NaN()}) {
        input=node(); input.m_magnitude=magnitude;
        require(filter.filterCallback(&grid,&input)==SeismicSimulationFilterBase::SEISMIC_STATUS_INVALID && !grid.writes,"nonfinite admission");
    }
    input=node(std::numeric_limits<UnsignedInt>::max());
    require(filter.filterCallback(&grid,&input)==SeismicSimulationFilterBase::SEISMIC_STATUS_INVALID,"radius width rejection");
    input=node(); input.m_center.x=std::numeric_limits<Int>::max();
    require(filter.filterCallback(&grid,&input)==SeismicSimulationFilterBase::SEISMIC_STATUS_INVALID,"coordinate addition rejection");
    input=node(); input.m_center.y=std::numeric_limits<Int>::min(); input.m_radius=4;
    require(filter.filterCallback(&grid,&input)==SeismicSimulationFilterBase::SEISMIC_STATUS_INVALID,"coordinate subtraction rejection");
    input=node(); const auto baseline=AllocationFault::live();
    for (bool reading:{true,false}) {
        for (int ordinal=0;ordinal<(reading?25:36);++ordinal) {
            Grid candidate; if (reading) candidate.failRead=ordinal; else candidate.failWrite=ordinal;
            bool failed=false; try { filter.filterCallback(&candidate,&input); } catch(CallbackFailure) {failed=true;}
            require(failed && AllocationFault::live()==baseline,"every sample/write exception retires workspace");
            candidate=Grid{}; require(filter.filterCallback(&candidate,&input)==SeismicSimulationFilterBase::SEISMIC_STATUS_ACTIVE,"corrected callback retry");
        }
    }
    AllocationFault::arm(0); bool failed=false;
    try { filter.filterCallback(&grid,&input); } catch(const std::bad_alloc&) { failed=true; }
    const auto count=AllocationFault::attempts(); AllocationFault::disarm();
    require(failed && count==1 && !grid.writes && AllocationFault::live()==baseline,"workspace failure before effects");
    require(filter.filterCallback(&grid,&input)==SeismicSimulationFilterBase::SEISMIC_STATUS_ACTIVE,"same-owner allocation retry");
    grid=Grid{}; AllocationFault::arm(1); filter.filterCallback(&grid,&input);
    const auto terminal=AllocationFault::attempts(); const auto hit=AllocationFault::triggered(); AllocationFault::disarm();
    require(terminal==1 && !hit && AllocationFault::live()==baseline,"exact allocation terminal and retirement");
}
}
int main(int argc,char** argv) {
    try {
        require(argc==2,"family"); const std::string_view family=argv[1];
        for (int repeat=0;repeat<3;++repeat) {
            const auto baseline=AllocationFault::live();
            if (family=="oracle") oracle(); else if (family=="failures") failures(); else require(false,"unknown family");
            require(AllocationFault::live()==baseline,"whole repeated filter lifetime");
        }
        return 0;
    } catch (...) { AllocationFault::disarm(); return 1; }
}
