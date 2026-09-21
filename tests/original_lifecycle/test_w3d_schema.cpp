#include "PreRTS.h"
#include "Common/GameEngine.h"
#include "Common/NameKeyGenerator.h"
#include "W3DDevice/Common/W3DModuleFactory.h"
#include "W3DDevice/GameClient/Module/W3DDependencyModelDraw.h"
#include "W3DDevice/GameClient/Module/W3DLaserDraw.h"
#include "W3DDevice/GameClient/Module/W3DModelDraw.h"
#include "W3DDevice/GameClient/Module/W3DOverlordAircraftDraw.h"
#include "W3DDevice/GameClient/Module/W3DOverlordTankDraw.h"
#include "W3DDevice/GameClient/Module/W3DOverlordTruckDraw.h"
#include "W3DDevice/GameClient/Module/W3DProjectileStreamDraw.h"
#include "W3DDevice/GameClient/Module/W3DPropDraw.h"
#include "W3DDevice/GameClient/Module/W3DScienceModelDraw.h"
#include "W3DDevice/GameClient/Module/W3DSupplyDraw.h"
#include "W3DDevice/GameClient/Module/W3DTankDraw.h"
#include "W3DDevice/GameClient/Module/W3DTankTruckDraw.h"
#include "W3DDevice/GameClient/Module/W3DTreeDraw.h"
#include "W3DDevice/GameClient/Module/W3DTruckDraw.h"
#include <cstdio>
#include <typeinfo>

GameEngine *TheGameEngine = NULL;

namespace {
template <typename T> bool matches(ModuleData *data) { return typeid(*data) == typeid(T); }
struct Registration { const char *name; bool (*matches)(ModuleData *); };
int fail(const char *message, const char *name = "")
{
	std::fprintf(stderr, "w3d schema: %s %s\n", message, name);
	return 1;
}
}

int main()
{
	initMemoryManager();
	NameKeyGenerator keys;
	TheNameKeyGenerator = &keys;
	keys.init();
	W3DModuleFactory factory;
	TheModuleFactory = &factory;
	factory.init();
	const Registration expected[] = {
		{"W3DDefaultDraw", matches<ModuleData>}, {"W3DDebrisDraw", matches<ModuleData>},
		{"W3DModelDraw", matches<W3DModelDrawModuleData>}, {"W3DLaserDraw", matches<W3DLaserDrawModuleData>},
		{"W3DOverlordTankDraw", matches<W3DOverlordTankDrawModuleData>},
		{"W3DOverlordTruckDraw", matches<W3DOverlordTruckDrawModuleData>},
		{"W3DOverlordAircraftDraw", matches<W3DOverlordAircraftDrawModuleData>},
		{"W3DProjectileStreamDraw", matches<W3DProjectileStreamDrawModuleData>},
		{"W3DPoliceCarDraw", matches<W3DTruckDrawModuleData>}, {"W3DRopeDraw", matches<ModuleData>},
		{"W3DScienceModelDraw", matches<W3DScienceModelDrawModuleData>},
		{"W3DSupplyDraw", matches<W3DSupplyDrawModuleData>},
		{"W3DDependencyModelDraw", matches<W3DDependencyModelDrawModuleData>},
		{"W3DTankDraw", matches<W3DTankDrawModuleData>}, {"W3DTruckDraw", matches<W3DTruckDrawModuleData>},
		{"W3DTracerDraw", matches<ModuleData>}, {"W3DTankTruckDraw", matches<W3DTankTruckDrawModuleData>},
		{"W3DTreeDraw", matches<W3DTreeDrawModuleData>}, {"W3DPropDraw", matches<W3DPropDrawModuleData>},
	};
	for (const Registration &registration : expected)
	{
		if (factory.findModuleInterfaceMask(AsciiString(registration.name), MODULETYPE_DRAW) != MODULEINTERFACE_DRAW)
			return fail("wrong registration interface", registration.name);
		ModuleData *data = factory.newModuleDataFromINI(NULL, AsciiString(registration.name), MODULETYPE_DRAW,
			AsciiString("schema-test"));
		if (data == NULL || !registration.matches(data))
			return fail("wrong concrete ModuleData", registration.name);
		if (AsciiString(registration.name) == AsciiString("W3DLaserDraw"))
		{
			const W3DLaserDrawModuleData *laser = static_cast<const W3DLaserDrawModuleData *>(data);
			if (laser->m_innerBeamWidth != 0.0f || laser->m_outerBeamWidth != 1.0f ||
				laser->m_numBeams != 1 || laser->m_segments != 1 || laser->m_tile)
				return fail("original inherited/default values changed", registration.name);
		}
	}
	if (factory.newModuleDataFromINI(NULL, AsciiString("W3DUnknownDraw"), MODULETYPE_DRAW,
		AsciiString("missing")) != NULL)
		return fail("unknown provider was accepted");
	bool rejected = false;
	try { factory.newModule(NULL, AsciiString("W3DDebrisDraw"), NULL, MODULETYPE_DRAW); }
	catch (ErrorCode error) { rejected = error == ERROR_INVALID_D3D; }
	if (!rejected)
		return fail("physical draw instance did not fail closed");
	TheModuleFactory = NULL;
	TheNameKeyGenerator = NULL;
	std::printf("w3d schema: 19 original registrations; physical instances fail closed\n");
	return 0;
}
