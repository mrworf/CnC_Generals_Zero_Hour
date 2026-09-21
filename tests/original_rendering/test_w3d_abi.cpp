#include "PreRTS.h"
#include "GameClient/Shadow.h"
#include "W3DDevice/GameClient/Module/W3DDefaultDraw.h"
#include "W3DDevice/GameClient/Module/W3DModelDraw.h"
#include "W3DDevice/GameClient/Module/W3DTankDraw.h"
#include "W3DDevice/GameClient/Module/W3DTankTruckDraw.h"
#include "W3DDevice/GameClient/Module/W3DTruckDraw.h"
#include "W3DDevice/GameClient/Module/W3DSupplyDraw.h"
#include "W3DDevice/GameClient/Module/W3DOverlordTankDraw.h"
#include "W3DDevice/GameClient/Module/W3DDependencyModelDraw.h"
#include "W3DDevice/GameClient/Module/W3DOverlordAircraftDraw.h"
#include "W3DDevice/GameClient/Module/W3DOverlordTruckDraw.h"
#include <cstdio>

#define PRINT_LAYOUT(T) std::printf("%s %zu %zu\n", #T, sizeof(T), alignof(T))
int main()
{
    PRINT_LAYOUT(W3DDefaultDraw);
    PRINT_LAYOUT(W3DModelDraw);
    PRINT_LAYOUT(W3DTankDraw);
    PRINT_LAYOUT(W3DTankTruckDraw);
    PRINT_LAYOUT(W3DTruckDraw);
    PRINT_LAYOUT(W3DSupplyDraw);
    PRINT_LAYOUT(W3DOverlordTankDraw);
    PRINT_LAYOUT(W3DDependencyModelDraw);
    PRINT_LAYOUT(W3DOverlordAircraftDraw);
    PRINT_LAYOUT(W3DOverlordTruckDraw);
    return 0;
}
