#pragma once

#include "Lib/BaseType.h"

// Source IDs are runtime-assigned UInt32 protocol values, not just named enums.
// Explicit representation makes raw decoding defined before owner admission.
enum ObjectID : UnsignedInt {
    INVALID_ID = 0,
    FORCE_OBJECTID_TO_LONG_SIZE = 0x7ffffff
};
enum DrawableID : UnsignedInt {
    INVALID_DRAWABLE_ID = 0,
    FORCE_DRAWABLEID_TO_LONG_SIZE = 0x7ffffff
};
enum FormationID : UnsignedInt {
    NO_FORMATION_ID = 0,
    FORCE_FORMATIONID_TO_LONG_SIZE = 0x7ffffff
};
enum ParticleSystemID : UnsignedInt { INVALID_PARTICLE_SYSTEM_ID = 0 };
enum ProductionID : UnsignedInt { PRODUCTIONID_INVALID = 0 };
enum WaypointID : UnsignedInt { INVALID_WAYPOINT_ID = 0x7FFFFFFF };
