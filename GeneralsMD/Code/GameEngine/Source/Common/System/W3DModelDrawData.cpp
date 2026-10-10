// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Electronic Arts Inc. Original W3DModelDraw parsing.
// Original draw-definition parsing, independent of renderer implementation.
#define DEFINE_WEAPONSLOTTYPE_NAMES
#include "Common/W3DModelDrawData.h"
#include "Common/Xfer.h"
#include "Common/GameLOD.h"
#include "GameLogic/WeaponSet.h"
#include <strings.h>
#include <cstdint>
#include <memory>
#if defined(_DEBUG) || defined(_INTERNAL)
extern AsciiString TheThingTemplateBeingParsedName;
#endif
static_assert(sizeof(Matrix3D)==48, "Original condition transfer matrix encoding must remain twelve float32 values");

enum INIReadFlagsType
{
	ANIMS_COPIED_FROM_DEFAULT_STATE = 0,
	GOT_NONIDLE_ANIMS,
	GOT_IDLE_ANIMS,
};

enum ACBits
{
	RANDOMIZE_START_FRAME = 0,
	START_FRAME_FIRST,
	START_FRAME_LAST,
	ADJUST_HEIGHT_BY_CONSTRUCTION_PERCENT,
	PRISTINE_BONE_POS_IN_FINAL_FRAME,
	MAINTAIN_FRAME_ACROSS_STATES,
	RESTART_ANIM_WHEN_COMPLETE,
	MAINTAIN_FRAME_ACROSS_STATES2,
	MAINTAIN_FRAME_ACROSS_STATES3,
	MAINTAIN_FRAME_ACROSS_STATES4,
};

static const char *ACBitsNames[] =
{
	"RANDOMSTART",
	"START_FRAME_FIRST",
	"START_FRAME_LAST",
	"ADJUST_HEIGHT_BY_CONSTRUCTION_PERCENT",
	"PRISTINE_BONE_POS_IN_FINAL_FRAME",
	"MAINTAIN_FRAME_ACROSS_STATES",
	"RESTART_ANIM_WHEN_COMPLETE",
	"MAINTAIN_FRAME_ACROSS_STATES2",
	"MAINTAIN_FRAME_ACROSS_STATES3",
	"MAINTAIN_FRAME_ACROSS_STATES4",

	NULL
};


static const char* TheAnimModeNames[]={"MANUAL","LOOP","ONCE","LOOP_PINGPONG","LOOP_BACKWARDS","ONCE_BACKWARDS",nullptr};

enum ParseCondStateType
{
	PARSE_NORMAL,
	PARSE_DEFAULT,
	PARSE_TRANSITION,
	PARSE_ALIAS
};

//-------------------------------------------------------------------------------------------------
static void parseAsciiStringLC( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	AsciiString* asciiString = (AsciiString *)store;
	*asciiString = ini->getNextAsciiString();
	asciiString->toLower();
}

//-------------------------------------------------------------------------------------------------
void W3DModelDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  ModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "InitialRecoilSpeed",	INI::parseVelocityReal, NULL, offsetof(W3DModelDrawModuleData, m_initialRecoil) },
		{ "MaxRecoilDistance",	INI::parseReal, NULL, offsetof(W3DModelDrawModuleData, m_maxRecoil) },
		{ "RecoilDamping",	INI::parseReal, NULL, offsetof(W3DModelDrawModuleData, m_recoilDamping) },
		{ "RecoilSettleSpeed",	INI::parseVelocityReal, NULL, offsetof(W3DModelDrawModuleData, m_recoilSettle) },
		{ "OkToChangeModelColor",	INI::parseBool, NULL, offsetof(W3DModelDrawModuleData, m_okToChangeModelColor) },
		{ "AnimationsRequirePower",	INI::parseBool, NULL, offsetof(W3DModelDrawModuleData, m_animationsRequirePower) },
		{ "ParticlesAttachedToAnimatedBones",	INI::parseBool, NULL, offsetof(W3DModelDrawModuleData, m_particlesAttachedToAnimatedBones) },
		{ "MinLODRequired",		INI::parseStaticGameLODLevel,	NULL,	offsetof(W3DModelDrawModuleData, m_minLODRequired) },
		{ "ProjectileBoneFeedbackEnabledSlots", INI::parseBitString32, TheWeaponSlotTypeNames, offsetof(W3DModelDrawModuleData, m_projectileBoneFeedbackEnabledSlots) },
		{ "DefaultConditionState", W3DModelDrawModuleData::parseConditionState, (void*)PARSE_DEFAULT, 0 },
		{ "ConditionState", W3DModelDrawModuleData::parseConditionState, (void*)PARSE_NORMAL, 0 },
		{ "AliasConditionState", W3DModelDrawModuleData::parseConditionState, (void*)PARSE_ALIAS, 0 },
		{ "TransitionState", W3DModelDrawModuleData::parseConditionState, (void*)PARSE_TRANSITION, 0 },
		{ "TrackMarks", parseAsciiStringLC, NULL, offsetof(W3DModelDrawModuleData, m_trackFile) },
		{ "ExtraPublicBone", INI::parseAsciiStringVectorAppend, NULL, offsetof(W3DModelDrawModuleData, m_extraPublicBones) },
		{ "AttachToBoneInAnotherModule", parseAsciiStringLC, NULL, offsetof(W3DModelDrawModuleData, m_attachToDrawableBone) },
		{ "IgnoreConditionStates", ModelConditionFlags::parseFromINI, NULL, offsetof(W3DModelDrawModuleData, m_ignoreConditionStates) },
		{ "ReceivesDynamicLights", INI::parseBool, NULL, offsetof(W3DModelDrawModuleData, m_receivesDynamicLights) },
    { 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);

}

//-------------------------------------------------------------------------------------------------
enum AnimParseType
{
	ANIM_NORMAL,
	ANIM_IDLE
};

//-------------------------------------------------------------------------------------------------
static void parseAnimation(INI* ini, void *instance, void * /*store*/, const void* userData)
{
	const auto rawType=reinterpret_cast<std::intptr_t>(userData);
	if (rawType<ANIM_NORMAL || rawType>ANIM_IDLE) throw INI_INVALID_DATA;
	AnimParseType animType = static_cast<AnimParseType>(rawType);

	AsciiString animName = ini->getNextAsciiString();
	animName.toLower();

	const char* distanceCoveredToken = ini->getNextTokenOrNull();
	Real distanceCovered = distanceCoveredToken ? INI::scanReal(distanceCoveredToken) : 0;
	DEBUG_ASSERTCRASH(!(animType == ANIM_IDLE && distanceCovered != 0), ("You should not specify nonzero DistanceCovered values for Idle Anims"));
	if (animType==ANIM_IDLE && distanceCovered!=0) throw INI_INVALID_DATA;

	const char* timesToRepeatToken = ini->getNextTokenOrNull();
	Int timesToRepeat = timesToRepeatToken ? INI::scanInt(timesToRepeatToken) : 1;
	if (timesToRepeat < 1) timesToRepeat = 1;

	W3DAnimationInfo animInfo(animName, (animType == ANIM_IDLE), distanceCovered);

	ModelConditionInfo* self = (ModelConditionInfo*)instance;
	if (self->m_iniReadFlags & (1<<ANIMS_COPIED_FROM_DEFAULT_STATE))
	{
		self->m_iniReadFlags &= ~((1<<ANIMS_COPIED_FROM_DEFAULT_STATE)|(1<<GOT_IDLE_ANIMS)|(1<<GOT_NONIDLE_ANIMS));
		self->m_animations.clear();
	}

	if (animInfo.isIdleAnim())
		self->m_iniReadFlags |= (1<<GOT_IDLE_ANIMS);
	else
		self->m_iniReadFlags |= (1<<GOT_NONIDLE_ANIMS);

	if (!animName.isEmpty() && !animName.isNone())
	{
		while (timesToRepeat--)
			self->m_animations.push_back(animInfo);
	}
}

//-------------------------------------------------------------------------------------------------
static void parseShowHideSubObject(INI* ini, void *instance, void *store, const void* userData)
{
	std::vector<ModelConditionInfo::HideShowSubObjInfo>* vec = (std::vector<ModelConditionInfo::HideShowSubObjInfo>*)store;
	AsciiString subObjName = ini->getNextAsciiString();
	subObjName.toLower();

	// handle "None"
	if (subObjName.isNone())
	{
		vec->clear();
		return;
	}

	while (subObjName.isNotEmpty())
	{
		Bool found = false;
		for (std::vector<ModelConditionInfo::HideShowSubObjInfo>::iterator it = vec->begin(); it != vec->end(); ++it)
		{
			if (strcasecmp(it->subObjName.str(), subObjName.str()) == 0)
			{
				it->hide = (userData != NULL);
				found = true;
			}
		}

		if (!found)
		{
			ModelConditionInfo::HideShowSubObjInfo info;
			info.subObjName = subObjName;
			info.hide = (userData != NULL);
			vec->push_back(info);
		}
		subObjName = ini->getNextAsciiString();
		subObjName.toLower();
	}
}

//-------------------------------------------------------------------------------------------------




//-------------------------------------------------------------------------------------------------
static void parseWeaponBoneName(INI* ini, void *instance, void * store, const void* /*userData*/)
{
	ModelConditionInfo* self = (ModelConditionInfo*)instance;
	AsciiString* arr = (AsciiString*)store;

	WeaponSlotType wslot = (WeaponSlotType)INI::scanIndexList(ini->getNextToken(), TheWeaponSlotTypeNames);
	arr[wslot] = ini->getNextAsciiString();
	arr[wslot].toLower();
	if (arr[wslot].isNone())
		arr[wslot].clear();

	if (self)
		self->addPublicBone(arr[wslot]);
}

//-------------------------------------------------------------------------------------------------
static void parseParticleSysBone(INI* ini, void *instance, void * store, const void * /*userData*/)
{
	ParticleSysBoneInfo info;
	info.boneName = ini->getNextAsciiString();
	info.boneName.toLower();
	ini->parseParticleSystemTemplate(ini, instance, &(info.particleSystemTemplate), NULL);
	ModelConditionInfo *self = (ModelConditionInfo *)instance;
	self->m_particleSysBones.push_back(info);
}

//-------------------------------------------------------------------------------------------------
static void parseRealRange( INI *ini, void *instance, void *store, const void* /*userData*/ )
{
	ModelConditionInfo *self = (ModelConditionInfo *)instance;

	const char *token = ini->getNextToken();
	self->m_animMinSpeedFactor = ini->scanReal( token );
	token = ini->getNextToken();
	self->m_animMaxSpeedFactor = ini->scanReal( token );
}

//-------------------------------------------------------------------------------------------------
static void parseLowercaseNameKey(INI* ini, void *instance, void * store, const void * /*userData*/)
{
	NameKeyType* key = (NameKeyType*)store;

	AsciiString tmp = ini->getNextToken();
	tmp.toLower();

	*key = NAMEKEY(tmp.str());
}

//-------------------------------------------------------------------------------------------------
static void parseBoneNameKey(INI* ini, void *instance, void * store, const void * /*userData*/)
{
	ModelConditionInfo* self = (ModelConditionInfo*)instance;
	NameKeyType* key = (NameKeyType*)store;

	AsciiString tmp = ini->getNextToken();
	tmp.toLower();

	if (self)
		self->addPublicBone(tmp);

	if (tmp.isEmpty() || tmp.isNone())
		*key = NAMEKEY_INVALID;
	else
		*key = NAMEKEY(tmp.str());
}

//-------------------------------------------------------------------------------------------------
static Bool doesStateExist(const ModelConditionVector& v, const ModelConditionFlags& f)
{
	for (ModelConditionVector::const_iterator it = v.begin(); it != v.end(); ++it)
	{
		for (Int i = it->getConditionsYesCount()-1; i >= 0; --i)
		{
			if (f == it->getNthConditionsYes(i))
				return true;
		}
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
void W3DModelDrawModuleData::parseConditionState(INI* ini, void *instance, void * /*store*/, const void* userData)
{
	static const FieldParse myFieldParse[] =
	{
		{ "Model",	parseAsciiStringLC, NULL, offsetof(ModelConditionInfo, m_modelName) },
		{ "Turret",	parseBoneNameKey, NULL, offsetof(ModelConditionInfo, m_turrets[0].m_turretAngleNameKey) },
		{ "TurretArtAngle", INI::parseAngleReal, NULL, offsetof(ModelConditionInfo, m_turrets[0].m_turretArtAngle) },
		{ "TurretPitch",	parseBoneNameKey, NULL, offsetof(ModelConditionInfo, m_turrets[0].m_turretPitchNameKey) },
		{ "TurretArtPitch", INI::parseAngleReal, NULL, offsetof(ModelConditionInfo, m_turrets[0].m_turretArtPitch) },
		{ "AltTurret",	parseBoneNameKey, NULL, offsetof(ModelConditionInfo, m_turrets[1].m_turretAngleNameKey) },
		{ "AltTurretArtAngle", INI::parseAngleReal, NULL, offsetof(ModelConditionInfo, m_turrets[1].m_turretArtAngle) },
		{ "AltTurretPitch",	parseBoneNameKey, NULL, offsetof(ModelConditionInfo, m_turrets[1].m_turretPitchNameKey) },
		{ "AltTurretArtPitch", INI::parseAngleReal, NULL, offsetof(ModelConditionInfo, m_turrets[1].m_turretArtPitch) },
		{ "ShowSubObject", parseShowHideSubObject, (void*)0, offsetof(ModelConditionInfo, m_hideShowVec) },
		{ "HideSubObject", parseShowHideSubObject, (void*)1, offsetof(ModelConditionInfo, m_hideShowVec) },
		{ "WeaponFireFXBone", parseWeaponBoneName, NULL, offsetof(ModelConditionInfo, m_weaponFireFXBoneName[0]) },
		{ "WeaponRecoilBone", parseWeaponBoneName, NULL, offsetof(ModelConditionInfo, m_weaponRecoilBoneName[0]) },
		{ "WeaponMuzzleFlash", parseWeaponBoneName, NULL, offsetof(ModelConditionInfo, m_weaponMuzzleFlashName[0]) },
		{ "WeaponLaunchBone", parseWeaponBoneName, NULL, offsetof(ModelConditionInfo, m_weaponProjectileLaunchBoneName[0]) },
		{ "WeaponHideShowBone", parseWeaponBoneName, NULL, offsetof(ModelConditionInfo, m_weaponProjectileHideShowName[0]) },
		{ "Animation", parseAnimation, (void*)ANIM_NORMAL, offsetof(ModelConditionInfo, m_animations) },
		{ "IdleAnimation", parseAnimation, (void*)ANIM_IDLE, offsetof(ModelConditionInfo, m_animations) },
		{ "AnimationMode", INI::parseIndexList, TheAnimModeNames, offsetof(ModelConditionInfo, m_mode) },
		{ "TransitionKey", parseLowercaseNameKey, NULL, offsetof(ModelConditionInfo, m_transitionKey) },
		{ "WaitForStateToFinishIfPossible", parseLowercaseNameKey, NULL, offsetof(ModelConditionInfo, m_allowToFinishKey) },
		{ "Flags", INI::parseBitString32, ACBitsNames, offsetof(ModelConditionInfo, m_flags) },
		{ "ParticleSysBone", parseParticleSysBone, NULL, 0 },
		{ "AnimationSpeedFactorRange", parseRealRange, NULL, 0 },
		{ 0, 0, 0, 0 }
	};

	ModelConditionInfo info;
	W3DModelDrawModuleData* self = (W3DModelDrawModuleData*)instance;
	const auto rawState=reinterpret_cast<std::intptr_t>(userData);
	if (rawState<PARSE_NORMAL || rawState>PARSE_ALIAS) throw INI_INVALID_DATA;
	ParseCondStateType cst = static_cast<ParseCondStateType>(rawState);
	switch (cst)
	{
		case PARSE_DEFAULT:
		{
			if (self->m_defaultState >= 0)
			{
				DEBUG_CRASH(("*** ASSET ERROR: you may have only one default state!\n"));
				throw INI_INVALID_DATA;
			}
			else if (ini->getNextTokenOrNull())
			{
				DEBUG_CRASH(("*** ASSET ERROR: unknown keyword\n"));
				throw INI_INVALID_DATA;
			}
			else
			{
				if (!self->m_conditionStates.empty())
				{
					DEBUG_CRASH(("*** ASSET ERROR: when using DefaultConditionState, it must be the first state listed (%s)\n",TheThingTemplateBeingParsedName.str()));
					throw INI_INVALID_DATA;
				}

				// note, this is size(), not size()-1, since we haven't actually modified the list yet
				self->m_defaultState = self->m_conditionStates.size();
				//DEBUG_LOG(("set default state to %d\n",self->m_defaultState));

				// add an empty conditionstateflag set
				ModelConditionFlags blankConditions;
				info.m_conditionsYesVec.clear();
				info.m_conditionsYesVec.push_back(blankConditions);

	#if defined(_DEBUG) || defined(_INTERNAL)
				info.m_description.clear();
				info.m_description.concat(TheThingTemplateBeingParsedName);
				info.m_description.concat(" DEFAULT");
	#endif
			}
		}
		break;

		case PARSE_TRANSITION:
		{
		  AsciiString firstNm = ini->getNextToken(); firstNm.toLower();
		  AsciiString secondNm = ini->getNextToken(); secondNm.toLower();
			NameKeyType firstKey = NAMEKEY(firstNm);
			NameKeyType secondKey = NAMEKEY(secondNm);

			if (firstKey == secondKey)
			{
				DEBUG_CRASH(("*** ASSET ERROR: You may not declare a transition between two identical states\n"));
				throw INI_INVALID_DATA;
			}

			if (self->m_defaultState >= 0)
			{
				info = self->m_conditionStates.at(self->m_defaultState);
				info.m_iniReadFlags |= (1<<ANIMS_COPIED_FROM_DEFAULT_STATE);
				info.m_transitionKey = NAMEKEY_INVALID;
				info.m_allowToFinishKey = NAMEKEY_INVALID;
			}

			info.m_transitionSig = buildTransitionSig(firstKey, secondKey);
	#if defined(_DEBUG) || defined(_INTERNAL)
			info.m_description.clear();
			info.m_description.concat(TheThingTemplateBeingParsedName);
			info.m_description.concat(" TRANSITION: ");
			info.m_description.concat(firstNm);
			info.m_description.concat(" ");
			info.m_description.concat(secondNm);
	#endif

		}
		break;

		case PARSE_ALIAS:
		{
			if (self->m_conditionStates.empty())
			{
				DEBUG_CRASH(("*** ASSET ERROR: AliasConditionState must refer to the previous state!\n"));
				throw INI_INVALID_DATA;
			}

			ModelConditionInfo& prevState = self->m_conditionStates.at(self->m_conditionStates.size()-1);

			ModelConditionFlags conditionsYes;

	#if defined(_DEBUG) || defined(_INTERNAL)
			AsciiString description;
			conditionsYes.parse(ini, &description);

			prevState.m_description.concat("\nAKA: ");
			prevState.m_description.concat(description);
	#else
			conditionsYes.parse(ini, NULL);
	#endif

			if (conditionsYes.anyIntersectionWith(self->m_ignoreConditionStates))
			{
				DEBUG_CRASH(("You should not specify bits in a state once they are used in IgnoreConditionStates (%s)\n", TheThingTemplateBeingParsedName.str()));
				throw INI_INVALID_DATA;
			}

			if (doesStateExist(self->m_conditionStates, conditionsYes))
			{
				DEBUG_CRASH(("*** ASSET ERROR: duplicate condition states are not currently allowed"));
				throw INI_INVALID_DATA;
			}


			if (!conditionsYes.any() && self->m_defaultState >= 0)
			{
				DEBUG_CRASH(("*** ASSET ERROR: you may not specify both a Default state and a Conditions=None state"));
				throw INI_INVALID_DATA;
			}

			prevState.m_conditionsYesVec.push_back(conditionsYes);

			// yes, return, NOT break!
			return;
		}

		case PARSE_NORMAL:
		{
			if (self->m_defaultState >= 0 && cst != PARSE_ALIAS)
			{
				info = self->m_conditionStates.at(self->m_defaultState);
				info.m_iniReadFlags |= (1<<ANIMS_COPIED_FROM_DEFAULT_STATE);
				info.m_conditionsYesVec.clear();
			}
	// no, we do not currently require a default state, cuz it would break the exiting INI
	// files too badly. maybe someday.
	//	else
	//	{
	//		DEBUG_CRASH(("*** ASSET ERROR: you must specify a default state\n"));
	//		throw INI_INVALID_DATA;
	//	}

			ModelConditionFlags conditionsYes;
	#if defined(_DEBUG) || defined(_INTERNAL)
			AsciiString description;
			conditionsYes.parse(ini, &description);

			info.m_description.clear();
			info.m_description.concat(TheThingTemplateBeingParsedName);
			info.m_description.concat("\n         ");
			info.m_description.concat(description);
	#else
			conditionsYes.parse(ini, NULL);
	#endif

			if (conditionsYes.anyIntersectionWith(self->m_ignoreConditionStates))
			{
				DEBUG_CRASH(("You should not specify bits in a state once they are used in IgnoreConditionStates (%s)\n", TheThingTemplateBeingParsedName.str()));
				throw INI_INVALID_DATA;
			}

			if (self->m_defaultState < 0 && self->m_conditionStates.empty() && conditionsYes.any())
			{
				// it doesn't actually NEED to be first, but it does need to be present, and this is the simplest way to enforce...
				DEBUG_CRASH(("*** ASSET ERROR: when not using DefaultConditionState, the first ConditionState must be for NONE (%s)\n",TheThingTemplateBeingParsedName.str()));
				throw INI_INVALID_DATA;
			}

			if (!conditionsYes.any() && self->m_defaultState >= 0)
			{
				DEBUG_CRASH(("*** ASSET ERROR: you may not specify both a Default state and a Conditions=None state"));
				throw INI_INVALID_DATA;
			}

			if (doesStateExist(self->m_conditionStates, conditionsYes))
			{
				DEBUG_CRASH(("*** ASSET ERROR: duplicate condition states are not currently allowed (%s)",info.m_description.str()));
				throw INI_INVALID_DATA;
			}

			DEBUG_ASSERTCRASH(info.m_conditionsYesVec.size() == 0, ("*** ASSET ERROR: nonempty m_conditionsYesVec.size(), see srj"));
			info.m_conditionsYesVec.clear();
			info.m_conditionsYesVec.push_back(conditionsYes);
		}
		break;
	}

	ini->initFromINI(&info, myFieldParse);

	if (info.m_modelName.isEmpty())
	{
		DEBUG_CRASH(("*** ASSET ERROR: you must specify a model name"));
		throw INI_INVALID_DATA;
	}
	else if (info.m_modelName.isNone())
	{
		info.m_modelName.clear();
	}

	if ((info.m_iniReadFlags & (1<<GOT_IDLE_ANIMS)) && (info.m_iniReadFlags & (1<<GOT_NONIDLE_ANIMS)))
	{
		DEBUG_CRASH(("*** ASSET ERROR: you should not specify both Animations and IdleAnimations for the same state"));
		throw INI_INVALID_DATA;
	}

	if ((info.m_iniReadFlags & (1<<GOT_IDLE_ANIMS)) && (info.m_mode != 2 && info.m_mode != 5))
	{
		DEBUG_CRASH(("*** ASSET ERROR: Idle Anims should always use ONCE or ONCE_BACKWARDS (%s)\n",TheThingTemplateBeingParsedName.str()));
		throw INI_INVALID_DATA;
	}

	info.m_validStuff &= ~ModelConditionInfo::HAS_PROJECTILE_BONES;
	for (int wslot = 0; wslot < WEAPONSLOT_COUNT; ++wslot)
	{
		if (info.m_weaponProjectileLaunchBoneName[wslot].isNotEmpty())
		{
			info.m_validStuff |= ModelConditionInfo::HAS_PROJECTILE_BONES;
			break;
		}
	}

	if (cst == PARSE_TRANSITION)
	{
		if (info.m_iniReadFlags & (1<<GOT_IDLE_ANIMS))
		{
			DEBUG_CRASH(("*** ASSET ERROR: Transition States should not specify Idle anims"));
			throw INI_INVALID_DATA;
		}

		if (info.m_mode != 2 && info.m_mode != 5)
		{
			DEBUG_CRASH(("*** ASSET ERROR: Transition States should always use ONCE or ONCE_BACKWARDS"));
			throw INI_INVALID_DATA;
		}

		if (info.m_transitionKey != NAMEKEY_INVALID || info.m_allowToFinishKey != NAMEKEY_INVALID)
		{
			DEBUG_CRASH(("*** ASSET ERROR: Transition States must not have transition keys or m_allowToFinishKey"));
			throw INI_INVALID_DATA;
		}

		self->m_transitionMap[info.m_transitionSig] = info;
	}
	else
	{
		self->m_conditionStates.push_back(info);
	}
}

W3DModelDrawModuleData::W3DModelDrawModuleData() :
	m_validated(0),
	m_okToChangeModelColor(false),
  m_particlesAttachedToAnimatedBones(false),
	m_animationsRequirePower(true),
#ifdef CACHE_ATTACH_BONE
	m_attachToDrawableBoneOffsetValid(false),
#endif
	m_minLODRequired(STATIC_GAME_LOD_LOW),
	m_defaultState(-1)
{
	const Real MAX_SHIFT = 3.0f;
	const Real INITIAL_RECOIL_RATE = 2.0f;
	const Real RECOIL_DAMPING = 0.4f;
	const Real SETTLE_RATE = 0.065f;

	m_projectileBoneFeedbackEnabledSlots = 0;
	m_attachToDrawableBoneOffset.Set(0,0,0);
	m_initialRecoil = INITIAL_RECOIL_RATE;
	m_maxRecoil = MAX_SHIFT;
	m_recoilDamping = RECOIL_DAMPING;
	m_recoilSettle = SETTLE_RATE;


  m_receivesDynamicLights = TRUE;

	// m_ignoreConditionStates defaults to all zero, which is what we want
}

W3DModelDrawModuleData::~W3DModelDrawModuleData()
{
	m_conditionStateMap.clear();
}

AsciiString W3DModelDrawModuleData::getBestModelNameForWB(const ModelConditionFlags& c) const
{
	const ModelConditionInfo* info = findBestInfo(c);
	if (info)
		return info->m_modelName;
	return AsciiString::TheEmptyString;
}

const ModelConditionInfo* W3DModelDrawModuleData::findBestInfo(const ModelConditionFlags& c) const
{
	ModelConditionFlags bits = c;
	bits.clear(m_ignoreConditionStates);
	return m_conditionStateMap.findBestInfo(m_conditionStates, bits);
}

void W3DModelDrawModuleData::crc( Xfer *x )
{
	xfer(x);
}

const ModelConditionInfo* W3DModelDrawModuleData::findBestInfoUncached(const ModelConditionFlags& c) const
{
	ModelConditionFlags bits=c;
	bits.clear(m_ignoreConditionStates);
	return m_conditionStateMap.findBestInfoUncached(m_conditionStates,bits);
}

void W3DModelDrawModuleData::xfer( Xfer *x )
{

	// version
	const XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	x->xferVersion( &version, currentVersion );

	for (ModelConditionVector::iterator it = m_conditionStates.begin(); it != m_conditionStates.end(); ++it)
	{
		ModelConditionInfo *info = &(*it);
		x->xferByte(&(info->m_validStuff));
#if defined(_DEBUG) || defined(_INTERNAL)
		x->xferAsciiString(&(info->m_description));
#endif
		if (info->m_validStuff)
		{
			for (PristineBoneInfoMap::iterator bit = info->m_pristineBones.begin(); bit != info->m_pristineBones.end(); ++bit)
			{
				PristineBoneInfo *bone = &(bit->second);
				x->xferInt(&(bone->boneIndex));
				x->xferUser(&(bone->mtx), sizeof(Matrix3D));
			}
			for (Int i=0; i<MAX_TURRETS; ++i)
			{
				x->xferInt(&(info->m_turrets[i].m_turretAngleBone));
				x->xferInt(&(info->m_turrets[i].m_turretPitchBone));
			}
			for (Int i=0; i<WEAPONSLOT_COUNT; ++i)
			{
				for (ModelConditionInfo::WeaponBarrelInfoVec::iterator wit = info->m_weaponBarrelInfoVec[i].begin(); wit != info->m_weaponBarrelInfoVec[i].end(); ++wit)
				{
					x->xferUser(&(wit->m_projectileOffsetMtx), sizeof(Matrix3D));
				}
			}
		}
	}
}

void W3DModelDrawModuleData::loadPostProcess( void )
{
}

std::unique_ptr<W3DModelDrawModuleData> W3DModelDrawModuleData::createFromINI(INI* ini) {
  if (!ini) return std::make_unique<W3DModelDrawModuleData>();
  if (!TheNameKeyGenerator) throw ERROR_BAD_ARG;
  NameKeyTransaction keys(*TheNameKeyGenerator);
  auto candidate=std::make_unique<W3DModelDrawModuleData>();
  if (ini) ini->initFromINIMultiProc(candidate.get(),buildFieldParse);
  keys.commit();
  return candidate;
}
