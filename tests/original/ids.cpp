#include "Common/EngineIDs.h"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <limits>
#include <cstdio>

// Prior declarations, with only enumerator names changed to avoid collisions.
namespace prior {
enum Object { ObjectInvalid = 0, ObjectForce = 0x7ffffff };
enum Drawable { DrawableInvalid = 0, DrawableForce = 0x7ffffff };
enum Formation { FormationInvalid = 0, FormationForce = 0x7ffffff };
enum Particle { ParticleInvalid = 0 };
enum Production { ProductionInvalid = 0 };
enum Waypoint { WaypointInvalid = 0x7FFFFFFF };
}
template<class ID> struct Layout {
    unsigned char prefix;
    ID id;
    UnsignedInt generation;
    void* owner;
};
template<class ID, class Prior> void prove()
{
    static_assert(sizeof(ID) == 4 && sizeof(ID) == sizeof(Prior));
    static_assert(alignof(ID) == alignof(Prior));
    static_assert(sizeof(Layout<ID>) == sizeof(Layout<Prior>));
    static_assert(alignof(Layout<ID>) == alignof(Layout<Prior>));
    static_assert(offsetof(Layout<ID>, id) == offsetof(Layout<Prior>, id));
    static_assert(offsetof(Layout<ID>, generation) == offsetof(Layout<Prior>, generation));
    static_assert(offsetof(Layout<ID>, owner) == offsetof(Layout<Prior>, owner));
    constexpr std::array<UnsignedInt, 8> values{0,1,0x7ffffff,0x8000000,
        0x7fffffff,0x80000000,0xfffffffe,0xffffffff};
    for (UnsignedInt raw : values) {
        const ID id = std::bit_cast<ID>(raw);
        if (static_cast<UnsignedInt>(id) != raw || std::bit_cast<UnsignedInt>(id) != raw)
            std::terminate();
        Layout<ID> record{0xa5, id, 19, nullptr};
        if (std::bit_cast<UnsignedInt>(record.id) != raw) std::terminate();
    }
}
int main()
{
    static_assert(INVALID_ID == 0 && INVALID_DRAWABLE_ID == 0 && NO_FORMATION_ID == 0);
    static_assert(INVALID_PARTICLE_SYSTEM_ID == 0 && PRODUCTIONID_INVALID == 0);
    static_assert(INVALID_WAYPOINT_ID == 0x7fffffff);
    static_assert(FORCE_OBJECTID_TO_LONG_SIZE == 0x7ffffff);
    static_assert(FORCE_DRAWABLEID_TO_LONG_SIZE == 0x7ffffff);
    static_assert(FORCE_FORMATIONID_TO_LONG_SIZE == 0x7ffffff);
    prove<ObjectID,prior::Object>(); prove<DrawableID,prior::Drawable>();
    prove<FormationID,prior::Formation>(); prove<ParticleSystemID,prior::Particle>();
    prove<ProductionID,prior::Production>(); prove<WaypointID,prior::Waypoint>();
    std::puts("original-id-representations: PASS");
}
