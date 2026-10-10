// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "Common/W3DDrawProviderData.h"
#include "Common/INI.h"

W3DTankDrawModuleData::W3DTankDrawModuleData() :
	m_treadDebrisNameLeft("TrackDebrisDirtLeft"),
	m_treadDebrisNameRight("TrackDebrisDirtRight"),
	m_treadAnimationRate(0.0f),
	m_treadPivotSpeedFraction(0.6f),
	m_treadDriveSpeedFraction(0.3f)
{
}

W3DTankDrawModuleData::~W3DTankDrawModuleData()
{
}

void W3DTankDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  W3DModelDrawModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "TreadDebrisLeft", INI::parseAsciiString, NULL, offsetof(W3DTankDrawModuleData, m_treadDebrisNameLeft) },
		{ "TreadDebrisRight", INI::parseAsciiString, NULL, offsetof(W3DTankDrawModuleData, m_treadDebrisNameRight) },
		{ "TreadAnimationRate", INI::parseVelocityReal, NULL, offsetof(W3DTankDrawModuleData, m_treadAnimationRate) },
		{ "TreadPivotSpeedFraction", INI::parseReal, NULL, offsetof(W3DTankDrawModuleData, m_treadPivotSpeedFraction) },
		{ "TreadDriveSpeedFraction", INI::parseReal, NULL, offsetof(W3DTankDrawModuleData, m_treadDriveSpeedFraction) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

W3DTruckDrawModuleData::W3DTruckDrawModuleData()
{
}

W3DTruckDrawModuleData::~W3DTruckDrawModuleData()
{
}

void W3DTruckDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  W3DModelDrawModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "Dust", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_dustEffectName) },
		{ "DirtSpray", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_dirtEffectName) },
		{ "PowerslideSpray", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_powerslideEffectName) },

		{ "LeftFrontTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_frontLeftTireBoneName) },
		{ "RightFrontTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_frontRightTireBoneName) },
		{ "LeftRearTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_rearLeftTireBoneName) },
		{ "RightRearTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_rearRightTireBoneName) },
		{ "MidLeftFrontTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_midFrontLeftTireBoneName) },
		{ "MidRightFrontTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_midFrontRightTireBoneName) },
		{ "MidLeftRearTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_midRearLeftTireBoneName) },
		{ "MidRightRearTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_midRearRightTireBoneName) },
		{ "MidLeftMidTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_midMidLeftTireBoneName) },
		{ "MidRightMidTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_midMidRightTireBoneName) },

		{ "TireRotationMultiplier", INI::parseReal, NULL, offsetof(W3DTruckDrawModuleData, m_rotationSpeedMultiplier) },
		{ "PowerslideRotationAddition", INI::parseReal, NULL, offsetof(W3DTruckDrawModuleData, m_powerslideRotationAddition) },
		{ "CabBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_cabBoneName) },
		{ "TrailerBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_trailerBoneName) },
		{ "CabRotationMultiplier", INI::parseReal, NULL, offsetof(W3DTruckDrawModuleData, m_cabRotationFactor) },
		{ "TrailerRotationMultiplier", INI::parseReal, NULL, offsetof(W3DTruckDrawModuleData, m_trailerRotationFactor) },
		{ "RotationDamping", INI::parseReal, NULL, offsetof(W3DTruckDrawModuleData, m_rotationDampingFactor) },

		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

W3DDependencyModelDrawModuleData::W3DDependencyModelDrawModuleData()
{
}

W3DDependencyModelDrawModuleData::~W3DDependencyModelDrawModuleData()
{
}

void W3DDependencyModelDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  W3DModelDrawModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "AttachToBoneInContainer", INI::parseAsciiString, NULL, offsetof(W3DDependencyModelDrawModuleData, m_attachToDrawableBoneInContainer) },

		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

W3DLaserDrawModuleData::W3DLaserDrawModuleData()
{
	m_innerBeamWidth = 0.0f;         //The total width of beam
	m_outerBeamWidth = 1.0f;         //The total width of beam
  m_numBeams = 1;                 //Number of overlapping cylinders that make the beam. 1 beam will just use inner data.
  m_maxIntensityFrames = 0;				//Laser stays at max intensity for specified time in ms.
  m_fadeFrames = 0;               //Laser will fade and delete.
	m_scrollRate = 0.0f;
	m_tile = false;
	m_segments = 1;
	m_arcHeight = 0.0f;
	m_segmentOverlapRatio = 0.0f;
	m_tilingScalar = 1.0f;
}

W3DLaserDrawModuleData::~W3DLaserDrawModuleData()
{
}

void W3DLaserDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  ModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "NumBeams",							INI::parseUnsignedInt,					NULL, offsetof( W3DLaserDrawModuleData, m_numBeams ) },
		{ "InnerBeamWidth",				INI::parseReal,									NULL, offsetof( W3DLaserDrawModuleData, m_innerBeamWidth ) },
		{ "OuterBeamWidth",				INI::parseReal,									NULL, offsetof( W3DLaserDrawModuleData, m_outerBeamWidth ) },
		{ "InnerColor",						INI::parseColorInt,							NULL, offsetof( W3DLaserDrawModuleData, m_innerColor ) },
		{ "OuterColor",						INI::parseColorInt,							NULL, offsetof( W3DLaserDrawModuleData, m_outerColor ) },
		{ "MaxIntensityLifetime",	INI::parseDurationUnsignedInt,	NULL, offsetof( W3DLaserDrawModuleData, m_maxIntensityFrames ) },
		{ "FadeLifetime",					INI::parseDurationUnsignedInt,	NULL, offsetof( W3DLaserDrawModuleData, m_fadeFrames ) },
		{ "Texture",							INI::parseAsciiString,					NULL, offsetof( W3DLaserDrawModuleData, m_textureName ) },
		{ "ScrollRate",						INI::parseReal,									NULL, offsetof( W3DLaserDrawModuleData, m_scrollRate ) },
		{ "Tile",									INI::parseBool,									NULL, offsetof( W3DLaserDrawModuleData, m_tile ) },
		{ "Segments",							INI::parseUnsignedInt,					NULL, offsetof( W3DLaserDrawModuleData, m_segments ) },
    { "ArcHeight",						INI::parseReal,									NULL, offsetof( W3DLaserDrawModuleData, m_arcHeight ) },
		{ "SegmentOverlapRatio",	INI::parseReal,									NULL, offsetof( W3DLaserDrawModuleData, m_segmentOverlapRatio ) },
		{ "TilingScalar",					INI::parseReal,									NULL, offsetof( W3DLaserDrawModuleData, m_tilingScalar ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

W3DOverlordAircraftDrawModuleData::W3DOverlordAircraftDrawModuleData()
{
}

W3DOverlordAircraftDrawModuleData::~W3DOverlordAircraftDrawModuleData()
{
}

void W3DOverlordAircraftDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  W3DModelDrawModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

W3DOverlordTankDrawModuleData::W3DOverlordTankDrawModuleData()
{
}

W3DOverlordTankDrawModuleData::~W3DOverlordTankDrawModuleData()
{
}

void W3DOverlordTankDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  W3DTankDrawModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

W3DOverlordTruckDrawModuleData::W3DOverlordTruckDrawModuleData()
{
}

W3DOverlordTruckDrawModuleData::~W3DOverlordTruckDrawModuleData()
{
}

void W3DOverlordTruckDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  W3DTruckDrawModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

W3DProjectileStreamDrawModuleData::W3DProjectileStreamDrawModuleData()
{
	m_textureName = "";
	m_width = 0.0f;
	m_tileFactor = 0.0f;
	m_scrollRate = 0.0f;
	m_maxSegments = 0;
}

W3DProjectileStreamDrawModuleData::~W3DProjectileStreamDrawModuleData()
{
}

void W3DProjectileStreamDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  ModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "Texture",			INI::parseAsciiString,	NULL, offsetof(W3DProjectileStreamDrawModuleData, m_textureName) },
		{ "Width",				INI::parseReal,					NULL, offsetof(W3DProjectileStreamDrawModuleData, m_width) },
		{ "TileFactor",		INI::parseReal,					NULL, offsetof(W3DProjectileStreamDrawModuleData, m_tileFactor) },
		{ "ScrollRate",		INI::parseReal,					NULL, offsetof(W3DProjectileStreamDrawModuleData, m_scrollRate) },
		{ "MaxSegments",	INI::parseInt,					NULL, offsetof(W3DProjectileStreamDrawModuleData, m_maxSegments) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

W3DPropDrawModuleData::W3DPropDrawModuleData()
{
}

W3DPropDrawModuleData::~W3DPropDrawModuleData()
{
}

void W3DPropDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  ModuleData::buildFieldParse(p);
	static const FieldParse dataFieldParse[] =
	{
		{ "ModelName", INI::parseAsciiString, NULL, offsetof(W3DPropDrawModuleData, m_modelName) },

		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

W3DScienceModelDrawModuleData::W3DScienceModelDrawModuleData()
{
	m_requiredScience = SCIENCE_INVALID;
}

W3DScienceModelDrawModuleData::~W3DScienceModelDrawModuleData()
{
}

void W3DScienceModelDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  W3DModelDrawModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "RequiredScience", INI::parseScience, NULL, offsetof(W3DScienceModelDrawModuleData, m_requiredScience) },

		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

W3DSupplyDrawModuleData::W3DSupplyDrawModuleData()
{
}

W3DSupplyDrawModuleData::~W3DSupplyDrawModuleData()
{
}

void W3DSupplyDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  W3DModelDrawModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "SupplyBonePrefix", INI::parseAsciiString, NULL, offsetof(W3DSupplyDrawModuleData, m_supplyBonePrefix) },

		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

W3DTankTruckDrawModuleData::W3DTankTruckDrawModuleData():
	m_treadDebrisNameLeft("TrackDebrisDirtLeft"),
	m_treadDebrisNameRight("TrackDebrisDirtRight"),
	m_treadAnimationRate(0.0f),
	m_treadPivotSpeedFraction(0.6f),
	m_treadDriveSpeedFraction(0.3f)
{
}

W3DTankTruckDrawModuleData::~W3DTankTruckDrawModuleData()
{
}

void W3DTankTruckDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  W3DModelDrawModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "Dust", INI::parseAsciiString, NULL, offsetof(W3DTankTruckDrawModuleData, m_dustEffectName) },
		{ "DirtSpray", INI::parseAsciiString, NULL, offsetof(W3DTankTruckDrawModuleData, m_dirtEffectName) },
		{ "PowerslideSpray", INI::parseAsciiString, NULL, offsetof(W3DTankTruckDrawModuleData, m_powerslideEffectName) },
		{ "LeftFrontTireBone", INI::parseAsciiString, NULL, offsetof(W3DTankTruckDrawModuleData, m_frontLeftTireBoneName) },
		{ "RightFrontTireBone", INI::parseAsciiString, NULL, offsetof(W3DTankTruckDrawModuleData, m_frontRightTireBoneName) },
		{ "LeftRearTireBone", INI::parseAsciiString, NULL, offsetof(W3DTankTruckDrawModuleData, m_rearLeftTireBoneName) },
		{ "RightRearTireBone", INI::parseAsciiString, NULL, offsetof(W3DTankTruckDrawModuleData, m_rearRightTireBoneName) },
		{ "MidLeftFrontTireBone", INI::parseAsciiString, NULL, offsetof(W3DTankTruckDrawModuleData, m_midFrontLeftTireBoneName) },
		{ "MidRightFrontTireBone", INI::parseAsciiString, NULL, offsetof(W3DTankTruckDrawModuleData, m_midFrontRightTireBoneName) },
		{ "MidLeftRearTireBone", INI::parseAsciiString, NULL, offsetof(W3DTankTruckDrawModuleData, m_midRearLeftTireBoneName) },
		{ "MidRightRearTireBone", INI::parseAsciiString, NULL, offsetof(W3DTankTruckDrawModuleData, m_midRearRightTireBoneName) },
		{ "TireRotationMultiplier", INI::parseReal, NULL, offsetof(W3DTankTruckDrawModuleData, m_rotationSpeedMultiplier) },
		{ "PowerslideRotationAddition", INI::parseReal, NULL, offsetof(W3DTankTruckDrawModuleData, m_powerslideRotationAddition) },
		{ "TreadDebrisLeft", INI::parseAsciiString, NULL, offsetof(W3DTankTruckDrawModuleData, m_treadDebrisNameLeft) },
		{ "TreadDebrisRight", INI::parseAsciiString, NULL, offsetof(W3DTankTruckDrawModuleData, m_treadDebrisNameRight) },
		{ "TreadAnimationRate", INI::parseVelocityReal, NULL, offsetof(W3DTankTruckDrawModuleData, m_treadAnimationRate) },
		{ "TreadPivotSpeedFraction", INI::parseReal, NULL, offsetof(W3DTankTruckDrawModuleData, m_treadPivotSpeedFraction) },
		{ "TreadDriveSpeedFraction", INI::parseReal, NULL, offsetof(W3DTankTruckDrawModuleData, m_treadDriveSpeedFraction) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

W3DTreeDrawModuleData::W3DTreeDrawModuleData() :
m_framesToMoveInward(1),
m_framesToMoveOutward(1),
m_darkening(0.0f),
m_maxOutwardMovement(1.0f)
{

	// Topple parameters. [7/7/2003]
	const Real START_VELOCITY_PERCENT = 0.2f;
	const Real START_ACCEL_PERCENT = 0.01f;
	const Real VELOCITY_BOUNCE_PERCENT = 0.3f;			// multiply the velocity by this when you bounce
  const Real MINIMUM_TOPPLE_SPEED = 0.5f;         // Won't let trees fall slower than this
	m_toppleFX = NULL;
	m_bounceFX = NULL;
	m_stumpName.clear();
	m_killWhenToppled = true;
	m_doTopple = false;
	m_doShadow = false;
	m_initialVelocityPercent = START_VELOCITY_PERCENT;
	m_initialAccelPercent = START_ACCEL_PERCENT;
	m_bounceVelocityPercent = VELOCITY_BOUNCE_PERCENT;
  m_minimumToppleSpeed = MINIMUM_TOPPLE_SPEED;
	m_sinkFrames = 10*LOGICFRAMES_PER_SECOND;
	m_sinkDistance = 20.0f;

}

W3DTreeDrawModuleData::~W3DTreeDrawModuleData()
{
}

void W3DTreeDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  ModuleData::buildFieldParse(p);
	static const FieldParse dataFieldParse[] =
	{
		{ "ModelName", INI::parseAsciiString, NULL, offsetof(W3DTreeDrawModuleData, m_modelName) },
		{ "TextureName", INI::parseAsciiString, NULL, offsetof(W3DTreeDrawModuleData, m_textureName) },
		{ "MoveOutwardTime", INI::parseDurationUnsignedInt, NULL, offsetof(W3DTreeDrawModuleData, m_framesToMoveOutward) },
		{ "MoveInwardTime", INI::parseDurationUnsignedInt, NULL, offsetof(W3DTreeDrawModuleData, m_framesToMoveInward) },
		{ "MoveOutwardDistanceFactor", INI::parseReal, NULL, offsetof(W3DTreeDrawModuleData, m_maxOutwardMovement) },
		{ "DarkeningFactor", INI::parseReal, NULL, offsetof(W3DTreeDrawModuleData, m_darkening) },

// Topple parameters [7/7/2003]
		{ "ToppleFX",	INI::parseFXList, NULL, offsetof( W3DTreeDrawModuleData, m_toppleFX ) },
		{ "BounceFX",	INI::parseFXList, NULL, offsetof( W3DTreeDrawModuleData, m_bounceFX ) },
		{ "StumpName",	INI::parseAsciiString, NULL, offsetof( W3DTreeDrawModuleData, m_stumpName ) },
		{ "KillWhenFinishedToppling",	INI::parseBool, NULL, offsetof( W3DTreeDrawModuleData, m_killWhenToppled ) },
		{ "DoTopple",	INI::parseBool, NULL, offsetof( W3DTreeDrawModuleData, m_doTopple ) },
		{ "InitialVelocityPercent",	INI::parsePercentToReal, NULL, offsetof( W3DTreeDrawModuleData, m_initialVelocityPercent ) },
		{ "InitialAccelPercent",	INI::parsePercentToReal, NULL, offsetof( W3DTreeDrawModuleData, m_initialAccelPercent ) },
		{ "BounceVelocityPercent",	INI::parsePercentToReal, NULL, offsetof( W3DTreeDrawModuleData, m_bounceVelocityPercent ) },
    { "MinimumToppleSpeed",	INI::parsePositiveNonZeroReal, NULL, offsetof( W3DTreeDrawModuleData, m_minimumToppleSpeed ) },
    { "SinkDistance",	INI::parsePositiveNonZeroReal, NULL, offsetof( W3DTreeDrawModuleData, m_sinkDistance ) },
		{ "SinkTime", INI::parseDurationUnsignedInt, NULL, offsetof(W3DTreeDrawModuleData, m_sinkFrames) },
		{ "DoShadow",	INI::parseBool, NULL, offsetof( W3DTreeDrawModuleData, m_doShadow ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

namespace {
template<class Data> ModuleData* createData(INI* ini) {
  if(!ini)return std::make_unique<Data>().release();
  if(!TheNameKeyGenerator)throw ERROR_BAD_ARG;
  NameKeyTransaction keys(*TheNameKeyGenerator);
  auto candidate=std::make_unique<Data>();
  ini->initFromINIMultiProc(candidate.get(),Data::buildFieldParse);
  keys.commit();
  return candidate.release();
}
const NativeDrawProviderData providers[]={
  {"W3DDefaultDraw",createData<ModuleData>},
  {"W3DDebrisDraw",createData<ModuleData>},
  {"W3DModelDraw",createData<W3DModelDrawModuleData>},
  {"W3DLaserDraw",createData<W3DLaserDrawModuleData>},
  {"W3DOverlordTankDraw",createData<W3DOverlordTankDrawModuleData>},
  {"W3DOverlordTruckDraw",createData<W3DOverlordTruckDrawModuleData>},
  {"W3DOverlordAircraftDraw",createData<W3DOverlordAircraftDrawModuleData>},
  {"W3DProjectileStreamDraw",createData<W3DProjectileStreamDrawModuleData>},
  {"W3DPoliceCarDraw",createData<W3DTruckDrawModuleData>},
  {"W3DRopeDraw",createData<ModuleData>},
  {"W3DScienceModelDraw",createData<W3DScienceModelDrawModuleData>},
  {"W3DSupplyDraw",createData<W3DSupplyDrawModuleData>},
  {"W3DDependencyModelDraw",createData<W3DDependencyModelDrawModuleData>},
  {"W3DTankDraw",createData<W3DTankDrawModuleData>},
  {"W3DTruckDraw",createData<W3DTruckDrawModuleData>},
  {"W3DTracerDraw",createData<ModuleData>},
  {"W3DTankTruckDraw",createData<W3DTankTruckDrawModuleData>},
  {"W3DTreeDraw",createData<W3DTreeDrawModuleData>},
  {"W3DPropDraw",createData<W3DPropDrawModuleData>},
};
}
std::span<const NativeDrawProviderData> nativeDrawProviderData() noexcept{return providers;}
