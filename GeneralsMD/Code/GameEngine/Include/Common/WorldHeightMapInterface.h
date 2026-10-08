// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Lib/BaseType.h"
// Original source height-velocity contract, shared without MapObject/gameplay
// ownership imports. Actual WorldHeightMap and the source filter use this type.
class WorldHeightMapInterfaceClass {
public:
    virtual Int getBorderSize() = 0;
    virtual Real getSeismicZVelocity(Int xIndex,Int yIndex) const = 0;
    virtual void setSeismicZVelocity(Int xIndex,Int yIndex,Real value) = 0;
    virtual Real getBilinearSampleSeismicZVelocity(Int x,Int y) = 0;
};
