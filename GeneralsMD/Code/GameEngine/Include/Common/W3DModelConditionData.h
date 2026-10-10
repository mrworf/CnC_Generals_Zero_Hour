/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: W3DModelDraw.h /////////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:   Default client update module
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Common/GameType.h"
#include "Common/GameCommon.h"
#include "Common/ModelState.h"
#include "Common/NativeW3DAssets.h"
#include "Common/SparseMatchFinder.h"
#include "WWMath/matrix3d.h"

class HAnimClass;
class RenderObjClass;
class ParticleSystemTemplate;

typedef UnsignedInt64 TransitionSig;

const TransitionSig NO_TRANSITION = 0;

inline TransitionSig buildTransitionSig(NameKeyType src, NameKeyType dst)
{
	return (((UnsignedInt64)src) << 32) | ((UnsignedInt64)dst);
}

inline NameKeyType recoverSrcState(TransitionSig sig)
{
	return (NameKeyType)((sig >> 32) & 0xffffffff);
}

inline NameKeyType recoverDstState(TransitionSig sig)
{
	return (NameKeyType)((sig) & 0xffffffff);
}

//-------------------------------------------------------------------------------------------------
// please do not define RETAIN_ANIM_HANDLES. It prevents us from ever releasing animations.
// old code should be deleting by mid-Jan 2003. (srj)
#define NO_RETAIN_ANIM_HANDLES

//-------------------------------------------------------------------------------------------------
class W3DAnimationInfo
{
private:
	AsciiString						m_name;
#ifdef RETAIN_ANIM_HANDLES
	mutable HAnimClass*		m_handle;
#endif
	Real									m_distanceCovered;		// if nonzero, the distance covered by a single loop of the anim
	mutable Real					m_naturalDurationInMsec;
	Bool									m_isIdleAnim;					// if true, we pick another animation at random when this one completes

public:
	W3DAnimationInfo(const AsciiString& name, Bool isIdle, Real distanceCovered);
	W3DAnimationInfo(const W3DAnimationInfo &r);
	W3DAnimationInfo &operator=(const W3DAnimationInfo& r);

	~W3DAnimationInfo();

	HAnimClass* getAnimHandle() const;
	const AsciiString& getName() const { return m_name; }
	Bool isIdleAnim() const { return m_isIdleAnim; }
	Real getDistanceCovered() const { return m_distanceCovered; }
	Real getNaturalDurationInMsec() const { return m_naturalDurationInMsec; }

};
typedef std::vector<W3DAnimationInfo>	W3DAnimationVector;

//-------------------------------------------------------------------------------------------------
struct ParticleSysBoneInfo
{
	AsciiString boneName;
	const ParticleSystemTemplate* particleSystemTemplate;
};

typedef std::vector<ParticleSysBoneInfo> ParticleSysBoneInfoVector;

//-------------------------------------------------------------------------------------------------
struct PristineBoneInfo
{
	Matrix3D mtx;
	Int boneIndex;
};
//typedef std::hash_map< NameKeyType, PristineBoneInfo, rts::hash<NameKeyType>, rts::equal_to<NameKeyType> > PristineBoneInfoMap;
typedef std::map< NameKeyType, PristineBoneInfo, std::less<NameKeyType> > PristineBoneInfoMap;

//-------------------------------------------------------------------------------------------------

struct ModelConditionInfo
{
	struct HideShowSubObjInfo
	{
		AsciiString subObjName;
		Bool hide;
	};

	struct TurretInfo
	{
		// read from INI
		NameKeyType		m_turretAngleNameKey;
		NameKeyType		m_turretPitchNameKey;
		Real					m_turretArtAngle;
		Real					m_turretArtPitch;
		// calculated at runtime
		Int						m_turretAngleBone;
		Int						m_turretPitchBone;

		void clear()
		{
			m_turretAngleNameKey = NAMEKEY_INVALID;
			m_turretPitchNameKey = NAMEKEY_INVALID;
			m_turretArtAngle = 0;
			m_turretArtPitch = 0;
			m_turretAngleBone = 0;
			m_turretPitchBone = 0;
		}
	};

	struct WeaponBarrelInfo
	{
		Int							m_recoilBone;									///< the W3D bone for this barrel (zero == no bone)
		Int							m_fxBone;											///< the FX bone for this barrel (zero == no bone)
		Int							m_muzzleFlashBone;						///< the muzzle-flash subobj bone for this barrel (zero == none)
		Matrix3D				m_projectileOffsetMtx;				///< where the projectile fires from
#if defined(_DEBUG) || defined(_INTERNAL)
		AsciiString			m_muzzleFlashBoneName;
#endif

		WeaponBarrelInfo()
		{
			clear();
		}

		void clear()
		{
			m_recoilBone = 0;
			m_fxBone = 0;
			m_muzzleFlashBone = 0;
			m_projectileOffsetMtx.Make_Identity();
#if defined(_DEBUG) || defined(_INTERNAL)
			m_muzzleFlashBoneName.clear();
#endif
		}

		void setMuzzleFlashHidden(RenderObjClass *fullObject, Bool hide) const;
	};
	typedef std::vector<WeaponBarrelInfo>	WeaponBarrelInfoVec;

#if defined(_DEBUG) || defined(_INTERNAL)
	AsciiString												m_description;
#endif
	std::vector<ModelConditionFlags>	m_conditionsYesVec;
	AsciiString												m_modelName;
	std::vector<HideShowSubObjInfo>		m_hideShowVec;
	mutable std::vector<AsciiString>	m_publicBones;
	AsciiString												m_weaponFireFXBoneName[WEAPONSLOT_COUNT];
	AsciiString												m_weaponRecoilBoneName[WEAPONSLOT_COUNT];
	AsciiString												m_weaponMuzzleFlashName[WEAPONSLOT_COUNT];
	AsciiString												m_weaponProjectileLaunchBoneName[WEAPONSLOT_COUNT];
	AsciiString												m_weaponProjectileHideShowName[WEAPONSLOT_COUNT];
	W3DAnimationVector								m_animations;
	NameKeyType												m_transitionKey;
	NameKeyType												m_allowToFinishKey;
	Int																m_flags;
	Int																m_iniReadFlags;	// not read from ini, but used for helping with default states
	Int					m_mode;
	ParticleSysBoneInfoVector					m_particleSysBones;			///< Bone names and attached particle systems.
	TransitionSig											m_transitionSig;
	Real															m_animMinSpeedFactor; //Min speed factor (randomized each time it's played)
	Real															m_animMaxSpeedFactor; //Max speed factor (randomized each time it's played)

	mutable PristineBoneInfoMap				m_pristineBones;
	mutable TurretInfo								m_turrets[MAX_TURRETS];
	mutable WeaponBarrelInfoVec				m_weaponBarrelInfoVec[WEAPONSLOT_COUNT];
	mutable Bool											m_hasRecoilBonesOrMuzzleFlashes[WEAPONSLOT_COUNT];
	mutable Byte											m_validStuff;

	enum
	{
		PRISTINE_BONES_VALID		= 0x0001,
		TURRETS_VALID						= 0x0002,
		HAS_PROJECTILE_BONES		= 0x0004,
		BARRELS_VALID						= 0x0008,
		PUBLIC_BONES_VALID			= 0x0010
	};

	inline ModelConditionInfo()
	{
		clear();
	}

	using NativeBinding = NativeW3DAssets::Binding;
  // A complete owned logic-query candidate; does not publish bone caches.
  NativeBinding prepareNativeBinding(NativeW3DAssets&, Real scale) const;

	void clear();
	void loadAnimations() const;
	void preloadAssets( TimeOfDay timeOfDay, Real scale );			///< preload any assets for time of day

	inline Int getConditionsYesCount() const { DEBUG_ASSERTCRASH(m_conditionsYesVec.size() > 0, ("empty m_conditionsYesVec.size(), see srj")); return m_conditionsYesVec.size(); }
	inline const ModelConditionFlags& getNthConditionsYes(Int i) const { return m_conditionsYesVec[i]; }
#if defined(_DEBUG) || defined(_INTERNAL)
	inline AsciiString getDescription() const { return m_description; }
#endif

	const Matrix3D* findPristineBone(NameKeyType boneName, Int* boneIndex) const;
	Bool findPristineBonePos(NameKeyType boneName, Coord3D& pos) const;
	void addPublicBone(const AsciiString& boneName) const;
	Bool matchesMode(Bool night, Bool snowy) const;

	void validateStuff(RenderObjClass* robj, Real scale, const std::vector<AsciiString>& extraPublicBones) const;

private:
	void validateWeaponBarrelInfo() const;
	void validateTurretInfo() const;
	void validateCachedBones(RenderObjClass* robj, Real scale) const;
};
typedef std::vector<ModelConditionInfo> ModelConditionVector;
