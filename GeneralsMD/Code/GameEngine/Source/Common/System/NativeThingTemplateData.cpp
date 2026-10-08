// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Electronic Arts Inc.
// Actual data owner helpers, independent of gameplay parsing/factory registration.
#include "Common/ThingTemplate.h"
#include "Common/GlobalData.h"
#include "Common/Radar.h"
#include "GameClient/Shadow.h"

ProductionPrerequisite::ProductionPrerequisite() { init(); }
ProductionPrerequisite::~ProductionPrerequisite() { }
void ProductionPrerequisite::init() {
	m_prereqUnits.clear();
	m_prereqSciences.clear();
}

ThingTemplate::ThingTemplate() :
	m_geometryInfo(GEOMETRY_SPHERE, FALSE, 1, 1, 1)
{
	m_moduleParsingMode = MODULEPARSE_NORMAL;
	m_reskinnedFrom = NULL;
	m_radarPriority = RADAR_PRIORITY_INVALID;

	m_nextThingTemplate = NULL;
	m_transportSlotCount = 0;
	m_fenceWidth = 0;
	m_fenceXOffset = 0;
	m_visionRange = 0.0f;
	m_shroudClearingRange = -1.0f;
	m_shroudRevealToAllRange = -1.0f;

	m_buildCost = 0;
	m_buildTime = 1;
	m_refundValue = 0;
	m_energyProduction = 0;
	m_energyBonus = 0;
	m_buildCompletion = BC_APPEARS_AT_RALLY_POINT;

	for( Int levelIndex = 0; levelIndex < LEVEL_COUNT; levelIndex++ )
	{
		m_experienceValues[levelIndex] = 0;
		m_experienceRequired[levelIndex] = 0;
		// -1 means "same value as experienceValues for that level"
		m_skillPointValues[levelIndex] = USE_EXP_VALUE_FOR_SKILL_VALUE;
	}
	m_isTrainable = FALSE;
	m_enterGuard = FALSE;
	m_hijackGuard = FALSE;

	m_templateID = 0;
	m_kindof = KINDOFMASK_NONE;
	//m_defaultOwningSide = "";	// unnecessary
	m_isBuildFacility = FALSE;
	m_isPrerequisite = FALSE;
	m_placementViewAngle = 0.0f;
	m_factoryExitWidth = 0.0f;
	m_factoryExtraBibWidth = 0.0f;

	m_selectedPortraitImage = NULL;
	m_buttonImage = NULL;

	m_shadowType = SHADOW_NONE;
	m_shadowSizeX = 0.0f;
	m_shadowSizeY = 0.0f;
	m_shadowOffsetX = 0.0f;
	m_shadowOffsetY = 0.0f;
	if (!TheGlobalData) throw ERROR_BAD_ARG;
	m_occlusionDelay = TheGlobalData->m_defaultOcclusionDelay;

	m_structureRubbleHeight = 0;
	m_instanceScaleFuzziness = 0;
	m_threatValue = 0;
	m_maxSimultaneousOfType = 0;	// unlimited
  m_maxSimultaneousLinkKey = NAMEKEY_INVALID; // Not linked
  m_maxSimultaneousDeterminedBySuperweaponRestriction = false;
	m_crusherLevel = 0;			//Unspecified, this object is unable to crush anything!
	m_crushableLevel = 255; //Unspecified, this object is unable to be crushed by anything!

}

//-------------------------------------------------------------------------------------------------
void ThingTemplate::copyFrom(const ThingTemplate* that)
{
	if (!that)
		return;

	ThingTemplate* next = this->m_nextThingTemplate;
	UnsignedShort id = this->m_templateID;
	AsciiString name = this->m_nameString;
	struct OverrideLinkGuard {
		ThingTemplate& owner;
		Overridable* accepted;
		Bool status;
		~OverrideLinkGuard() noexcept { owner.friend_restoreOverrideOwnership(accepted, status); }
	} overrideLink{*this, friend_getNextOverride(), friend_isOverride()};

	*this = *that;

	this->m_nextThingTemplate = next;
	this->m_templateID = id;
	this->m_nameString.swap(name);
}

//-------------------------------------------------------------------------------------------------
void ThingTemplate::setCopiedFromDefault()
{
	m_armorCopiedFromDefault = true;
	m_weaponsCopiedFromDefault = true;
	m_behaviorModuleInfo.setCopiedFromDefault(true);
	m_drawModuleInfo.setCopiedFromDefault(true);
	m_clientUpdateModuleInfo.setCopiedFromDefault(true);
}

//-------------------------------------------------------------------------------------------------
ThingTemplate::~ThingTemplate()
{
	// note, we don't need to take any special action for Armor/WeaponSets...
	// though it is just a list of 'raw' pointers, we don't have ownership of 'em,
	// and so we MUST NOT delete them
} 

//=============================================================================
