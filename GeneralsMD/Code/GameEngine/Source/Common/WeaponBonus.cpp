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

// Source-owned bonus configuration split from projectile/simulation services.
#include "PreRTS.h"
#define DEFINE_WEAPONBONUSCONDITION_NAMES
#define DEFINE_WEAPONBONUSFIELD_NAMES
#include "GameLogic/WeaponBonus.h"
#include "Common/INI.h"
void WeaponBonus::appendBonuses(WeaponBonus& bonus) const
{
	for (int f = 0; f < WeaponBonus::FIELD_COUNT; ++f)
	{
		bonus.m_field[f] += this->m_field[f] - 1.0f;
	}
}

//-------------------------------------------------------------------------------------------------
/*static*/ void WeaponBonusSet::parseWeaponBonusSet(INI* ini, void* /*instance*/, void* store, const void* /*userData*/)
{
	WeaponBonusSet* self = (WeaponBonusSet*)store;
	self->parseWeaponBonusSet(ini);
}

//-------------------------------------------------------------------------------------------------
/*static*/ void WeaponBonusSet::parseWeaponBonusSetPtr(INI* ini, void* /*instance*/, void* store, const void* /*userData*/)
{
	WeaponBonusSet** selfPtr = (WeaponBonusSet**)store;
	(*selfPtr)->parseWeaponBonusSet(ini);
}

//-------------------------------------------------------------------------------------------------
void WeaponBonusSet::parseWeaponBonusSet(INI* ini)
{
	if (!ini) throw ERROR_BAD_ARG;
	WeaponBonusConditionType wb = (WeaponBonusConditionType)INI::scanIndexList(ini->getNextToken(), TheWeaponBonusNames);
	WeaponBonus::Field wf = (WeaponBonus::Field)INI::scanIndexList(ini->getNextToken(), TheWeaponBonusFieldNames);
	if (wb < 0 || wb >= WEAPONBONUSCONDITION_COUNT || wf < 0 || wf >= WeaponBonus::FIELD_COUNT)
		throw ERROR_BAD_INI;
	m_bonus[wb].setField(wf, INI::scanPercentToReal(ini->getNextToken()));
}

//-------------------------------------------------------------------------------------------------
void WeaponBonusSet::appendBonuses(WeaponBonusConditionFlags flags, WeaponBonus& bonus) const
{
	static_assert(WEAPONBONUSCONDITION_COUNT <= 32);
	if (flags == 0)
		return;	// my, that was easy

	for (int i = 0; i < WEAPONBONUSCONDITION_COUNT; ++i)
	{
		if ((flags & (UnsignedInt{1} << i)) == 0)
			continue;
		
		this->m_bonus[i].appendBonuses(bonus);
	}
}
