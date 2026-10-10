// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/W3DModelDrawData.h"
#include "Common/ScienceType.h"
#include <span>

class FXList;

// Shared original declarations, not a second parsed schema.

class W3DTankDrawModuleData : public W3DModelDrawModuleData
{
public:
	AsciiString m_treadDebrisNameLeft;
	AsciiString m_treadDebrisNameRight;

	Real m_treadAnimationRate;	///<amount of tread texture to scroll per sec.  1.0 == full width.
	Real m_treadPivotSpeedFraction;	///<fraction of locomotor speed below which we allow pivoting.
	Real m_treadDriveSpeedFraction;	///<fraction of locomotor speed below which treads stop animating.

	W3DTankDrawModuleData();
	~W3DTankDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

class W3DTruckDrawModuleData : public W3DModelDrawModuleData
{
public:
	AsciiString m_dustEffectName;
	AsciiString m_dirtEffectName;
	AsciiString m_powerslideEffectName;

	AsciiString m_frontLeftTireBoneName;
	AsciiString m_frontRightTireBoneName;
	AsciiString m_rearLeftTireBoneName;
	AsciiString m_rearRightTireBoneName;
	//4 extra tires to support up to 8 tires.
	AsciiString m_midFrontLeftTireBoneName;
	AsciiString m_midFrontRightTireBoneName;
	AsciiString m_midRearLeftTireBoneName;
	AsciiString m_midRearRightTireBoneName;
	//And some more
	AsciiString m_midMidLeftTireBoneName;
	AsciiString m_midMidRightTireBoneName;

	// Cab bone for a segmented truck.
	AsciiString m_cabBoneName;
	AsciiString m_trailerBoneName;
	Real				m_cabRotationFactor;
	Real				m_trailerRotationFactor;
	Real				m_rotationDampingFactor;


	Real				m_rotationSpeedMultiplier;
	Real				m_powerslideRotationAddition;

	W3DTruckDrawModuleData();
	~W3DTruckDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

class W3DDependencyModelDrawModuleData : public W3DModelDrawModuleData
{
public:
	AsciiString	m_attachToDrawableBoneInContainer;// Not just a different draw module, this bone is in our container

	W3DDependencyModelDrawModuleData();
	~W3DDependencyModelDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

class W3DLaserDrawModuleData : public ModuleData
{
public:

  Color m_innerColor;
  Color m_outerColor;
	Real m_innerBeamWidth;
	Real m_outerBeamWidth;
	Real m_scrollRate;
	Bool m_tile;
  UnsignedInt m_numBeams;
  UnsignedInt m_maxIntensityFrames;
  UnsignedInt m_fadeFrames;
	AsciiString m_textureName;
	UnsignedInt m_segments;
	Real m_arcHeight;
	Real m_segmentOverlapRatio;
	Real m_tilingScalar;

	W3DLaserDrawModuleData();
	~W3DLaserDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

class W3DOverlordAircraftDrawModuleData : public W3DModelDrawModuleData
{
public:

	W3DOverlordAircraftDrawModuleData();
	~W3DOverlordAircraftDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

class W3DOverlordTankDrawModuleData : public W3DTankDrawModuleData
{
public:
	AsciiString m_treadDebrisNameLeft;
	AsciiString m_treadDebrisNameRight;

	Real m_treadAnimationRate;	///<amount of tread texture to scroll per sec.  1.0 == full width.
	Real m_treadPivotSpeedFraction;	///<fraction of locomotor speed below which we allow pivoting.
	Real m_treadDriveSpeedFraction;	///<fraction of locomotor speed below which treads stop animating.

	W3DOverlordTankDrawModuleData();
	~W3DOverlordTankDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

class W3DOverlordTruckDrawModuleData : public W3DTruckDrawModuleData
{
public:
	AsciiString m_treadDebrisNameLeft;
	AsciiString m_treadDebrisNameRight;

	Real m_treadAnimationRate;	///<amount of tread texture to scroll per sec.  1.0 == full width.
	Real m_treadPivotSpeedFraction;	///<fraction of locomotor speed below which we allow pivoting.
	Real m_treadDriveSpeedFraction;	///<fraction of locomotor speed below which treads stop animating.

	W3DOverlordTruckDrawModuleData();
	~W3DOverlordTruckDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

class W3DProjectileStreamDrawModuleData : public ModuleData
{
public:

	AsciiString m_textureName;
	Real m_width;
	Real m_tileFactor;
	Real m_scrollRate;
	Int m_maxSegments;

	W3DProjectileStreamDrawModuleData();
	~W3DProjectileStreamDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

class W3DPropDrawModuleData : public ModuleData
{
public:
	AsciiString m_modelName;

	W3DPropDrawModuleData();
	~W3DPropDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
	// ugh, hack
	virtual const W3DPropDrawModuleData* getAsW3DPropDrawModuleData() const { return this; }
};

class W3DScienceModelDrawModuleData : public W3DModelDrawModuleData
{
public:
	ScienceType m_requiredScience; ///< Local player must have this science for me to ever draw

	W3DScienceModelDrawModuleData();
	~W3DScienceModelDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

class W3DSupplyDrawModuleData : public W3DModelDrawModuleData
{
public:
	AsciiString m_supplyBonePrefix;

	W3DSupplyDrawModuleData();
	~W3DSupplyDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

class W3DTankTruckDrawModuleData : public W3DModelDrawModuleData
{
public:
	AsciiString m_dustEffectName;
	AsciiString m_dirtEffectName;
	AsciiString m_powerslideEffectName;

	AsciiString m_frontLeftTireBoneName;
	AsciiString m_frontRightTireBoneName;
	AsciiString m_rearLeftTireBoneName;
	AsciiString m_rearRightTireBoneName;
	//4 extra tires to support up to 8 tires.
	AsciiString m_midFrontLeftTireBoneName;
	AsciiString m_midFrontRightTireBoneName;
	AsciiString m_midRearLeftTireBoneName;
	AsciiString m_midRearRightTireBoneName;

	Real				m_rotationSpeedMultiplier;
	Real				m_powerslideRotationAddition;

	//Tank data
	AsciiString m_treadDebrisNameLeft;
	AsciiString m_treadDebrisNameRight;

	Real m_treadAnimationRate;	///<amount of tread texture to scroll per sec.  1.0 == full width.
	Real m_treadPivotSpeedFraction;	///<fraction of locomotor speed below which we allow pivoting.
	Real m_treadDriveSpeedFraction;	///<fraction of locomotor speed below which treads stop animating.

	W3DTankTruckDrawModuleData();
	~W3DTankTruckDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

class W3DTreeDrawModuleData : public ModuleData
{
public:
	AsciiString m_modelName;
	AsciiString m_textureName;
	// Push aside parameters. [5/29/2003]
	UnsignedInt	m_framesToMoveOutward;
	UnsignedInt	m_framesToMoveInward;
	Real				m_maxOutwardMovement;
	Real				m_darkening;

	// Topple parameters. [7/7/2003]
	const FXList* m_toppleFX;
	const FXList* m_bounceFX;
	AsciiString m_stumpName;
	Real m_initialVelocityPercent;
	Real m_initialAccelPercent;
	Real m_bounceVelocityPercent;
  Real m_minimumToppleSpeed;
	Bool m_killWhenToppled;
	Bool m_doTopple;
	UnsignedInt m_sinkFrames; // How long it takes to sink after toppling. [7/11/2003]
	Real m_sinkDistance;		// How far it sinks.

	Bool m_doShadow;

	W3DTreeDrawModuleData();
	~W3DTreeDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
	// ugh, hack
	virtual const W3DTreeDrawModuleData* getAsW3DTreeDrawModuleData() const { return this; }
};

struct NativeDrawProviderData {
  const char* name;
  ModuleData* (*create)(INI*);
};
// Original W3DModuleFactory order; concrete runtime creators are separate.
std::span<const NativeDrawProviderData> nativeDrawProviderData() noexcept;
