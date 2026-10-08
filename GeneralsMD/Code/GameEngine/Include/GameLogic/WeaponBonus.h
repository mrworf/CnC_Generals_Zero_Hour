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

// Source identities/configuration ownership, independent of weapon execution.
#pragma once
#include "Common/GameMemory.h"
#include "Common/GameCommon.h"
class INI;
enum WeaponBonusConditionType : Int
{
	// The access and use of this enum has the bit shifting built in, so this is a 0,1,2,3,4,5 enum
	WEAPONBONUSCONDITION_INVALID = -1,

	WEAPONBONUSCONDITION_GARRISONED = 0,
	WEAPONBONUSCONDITION_HORDE,
	WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN,
	WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST,
	WEAPONBONUSCONDITION_NATIONALISM,
	WEAPONBONUSCONDITION_PLAYER_UPGRADE,
	WEAPONBONUSCONDITION_DRONE_SPOTTING,
#ifdef ALLOW_DEMORALIZE
	WEAPONBONUSCONDITION_DEMORALIZED,
#else
	WEAPONBONUSCONDITION_DEMORALIZED_OBSOLETE,
#endif
	WEAPONBONUSCONDITION_ENTHUSIASTIC,
	WEAPONBONUSCONDITION_VETERAN,
	WEAPONBONUSCONDITION_ELITE,
	WEAPONBONUSCONDITION_HERO,
	WEAPONBONUSCONDITION_BATTLEPLAN_BOMBARDMENT,
	WEAPONBONUSCONDITION_BATTLEPLAN_HOLDTHELINE,
	WEAPONBONUSCONDITION_BATTLEPLAN_SEARCHANDDESTROY,
	WEAPONBONUSCONDITION_SUBLIMINAL,
	WEAPONBONUSCONDITION_SOLO_HUMAN_EASY,
	WEAPONBONUSCONDITION_SOLO_HUMAN_NORMAL,
	WEAPONBONUSCONDITION_SOLO_HUMAN_HARD,
	WEAPONBONUSCONDITION_SOLO_AI_EASY,
	WEAPONBONUSCONDITION_SOLO_AI_NORMAL,
	WEAPONBONUSCONDITION_SOLO_AI_HARD,
	WEAPONBONUSCONDITION_TARGET_FAERIE_FIRE,
  WEAPONBONUSCONDITION_FANATICISM, // FOR THE NEW GC INFANTRY GENERAL... adds to nationalism
	WEAPONBONUSCONDITION_FRENZY_ONE,
	WEAPONBONUSCONDITION_FRENZY_TWO,
	WEAPONBONUSCONDITION_FRENZY_THREE,

	WEAPONBONUSCONDITION_COUNT
};
#ifdef DEFINE_WEAPONBONUSCONDITION_NAMES
static const char *TheWeaponBonusNames[] = 
{
	// This is a RHS enum (weapon.ini will have WeaponBonus = IT) so it is all caps
	"GARRISONED",
	"HORDE",
	"CONTINUOUS_FIRE_MEAN",
	"CONTINUOUS_FIRE_FAST",
	"NATIONALISM",
	"PLAYER_UPGRADE",
	"DRONE_SPOTTING",
#ifdef ALLOW_DEMORALIZE
	"DEMORALIZED",
#else
	"DEMORALIZED_OBSOLETE",
#endif
	"ENTHUSIASTIC",
	"VETERAN",
	"ELITE",
	"HERO",
	"BATTLEPLAN_BOMBARDMENT",
	"BATTLEPLAN_HOLDTHELINE",
	"BATTLEPLAN_SEARCHANDDESTROY",
	"SUBLIMINAL",
	"SOLO_HUMAN_EASY",
	"SOLO_HUMAN_NORMAL",
	"SOLO_HUMAN_HARD",
	"SOLO_AI_EASY",
	"SOLO_AI_NORMAL",
	"SOLO_AI_HARD",
	"TARGET_FAERIE_FIRE",
  "FANATICISM", // FOR THE NEW GC INFANTRY GENERAL... adds to nationalism
	"FRENZY_ONE",
	"FRENZY_TWO",
	"FRENZY_THREE",

	NULL
};
#endif

// For WeaponBonusConditionFlags
// part of detangling
#include "GameLogic/WeaponBonusConditionFlags.h"

//-------------------------------------------------------------------------------------------------
class WeaponBonus
{
public:

	enum Field
	{
		DAMAGE = 0,
		RADIUS,
		RANGE,
		RATE_OF_FIRE,
		PRE_ATTACK,

		FIELD_COUNT	// keep last
	};

	WeaponBonus()
	{
		clear();
	}

	inline void clear()
	{
		for (int i = 0; i < FIELD_COUNT; ++i)
			m_field[i] = 1.0f;
	}

	inline Real getField(Field f) const { return m_field[f]; }
	inline void setField(Field f, Real v) { m_field[f] = v; }

	void appendBonuses(WeaponBonus& bonus) const;

private:
	Real m_field[FIELD_COUNT];

};

#ifdef DEFINE_WEAPONBONUSFIELD_NAMES
static const char *TheWeaponBonusFieldNames[] = 
{
	"DAMAGE",
	"RADIUS",
	"RANGE",
	"RATE_OF_FIRE",
	"PRE_ATTACK",
	NULL
};
#endif


//-------------------------------------------------------------------------------------------------
class WeaponBonusSet : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( WeaponBonusSet, "WeaponBonusSet" )
private:
	WeaponBonus m_bonus[WEAPONBONUSCONDITION_COUNT];

public:
	void appendBonuses(WeaponBonusConditionFlags flags, WeaponBonus& bonus) const;

	void parseWeaponBonusSet(INI* ini);
	static void parseWeaponBonusSet(INI* ini, void *instance, void* /*store*/, const void* /*userData*/);
	static void parseWeaponBonusSetPtr(INI* ini, void *instance, void* /*store*/, const void* /*userData*/);
};
EMPTY_DTOR(WeaponBonusSet)
