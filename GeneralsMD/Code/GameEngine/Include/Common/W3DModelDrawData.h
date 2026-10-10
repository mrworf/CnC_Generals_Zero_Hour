// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Electronic Arts Inc. Original W3DModelDraw definitions.
#pragma once
#include "Common/W3DModelConditionData.h"
#include "Common/Module.h"

//typedef std::hash_map< TransitionSig, ModelConditionInfo, std::hash<TransitionSig>, std::equal_to<TransitionSig> > TransitionMap;
typedef std::map< TransitionSig, ModelConditionInfo, std::less<TransitionSig> > TransitionMap;

//-------------------------------------------------------------------------------------------------
// this is more efficient and also helps solve a projectile-launch-offset problem for double-upgraded
// technicals. it is not within risk, however, and WILL NOT WORK if the attach-to-module is animated
// in any meaningful way, so be careful. (srj)
#define CACHE_ATTACH_BONE

//-------------------------------------------------------------------------------------------------
class W3DModelDrawModuleData : public ModuleData
{
public:

	mutable ModelConditionVector			m_conditionStates;
	mutable SparseMatchFinder< ModelConditionInfo, ModelConditionFlags >
																		m_conditionStateMap;
	mutable TransitionMap							m_transitionMap;
	std::vector<AsciiString>					m_extraPublicBones;
	AsciiString												m_trackFile;						///< if present, leaves tracks using this texture
	AsciiString												m_attachToDrawableBone;
#ifdef CACHE_ATTACH_BONE
	mutable Vector3										m_attachToDrawableBoneOffset;
#endif
	Int																m_defaultState;
	Int																m_projectileBoneFeedbackEnabledSlots;	///< Hide and show the launch bone geometries according to clip status adjustments.
	Real															m_initialRecoil;
	Real															m_maxRecoil;
	Real															m_recoilDamping;
	Real															m_recoilSettle;
	StaticGameLODLevel								m_minLODRequired;				///< minumum game LOD level necessary to use this module.
	ModelConditionFlags								m_ignoreConditionStates;
	Bool															m_okToChangeModelColor;
	Bool															m_animationsRequirePower;///< Should UnderPowered disable type pause animations in this draw module?
#ifdef CACHE_ATTACH_BONE
	mutable Bool											m_attachToDrawableBoneOffsetValid;
#endif
	mutable Byte											m_validated;

  Bool                              m_particlesAttachedToAnimatedBones;

  Bool                              m_receivesDynamicLights; ///< just like it sounds... it sets a property of Drawable, actually


	W3DModelDrawModuleData();
  static std::unique_ptr<W3DModelDrawModuleData> createFromINI(INI*);
	~W3DModelDrawModuleData();
	void validateStuffForTimeAndWeather(const Drawable* draw, Bool night, Bool snowy) const;
	static void buildFieldParse(MultiIniFieldParse& p);
	AsciiString getBestModelNameForWB(const ModelConditionFlags& c) const;
	const ModelConditionInfo* findBestInfo(const ModelConditionFlags& c) const;
	const ModelConditionInfo* findBestInfoUncached(const ModelConditionFlags& c) const;
	void preloadAssets( TimeOfDay timeOfDay, Real scale ) const;
#ifdef CACHE_ATTACH_BONE
	const Vector3* getAttachToDrawableBoneOffset(const Drawable* draw) const;
#endif

	// ugh, hack
	virtual const W3DModelDrawModuleData* getAsW3DModelDrawModuleData() const { return this; }
	virtual StaticGameLODLevel getMinimumRequiredGameLOD() const { return m_minLODRequired;}

private:
	static void parseConditionState( INI* ini, void *instance, void * /*store*/, const void* /*userData*/ );

public:
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );
};

//-------------------------------------------------------------------------------------------------
