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


#include "PreRTS.h"
#include "Common/MapObject.h"
#include "Common/WellKnownKeys.h"
#include "GameLogic/SidesList.h"

void MapObject::validate(void)
{
    if(!TheNameKeyGenerator) throw ERROR_BAD_ARG;
    NameKeyTransaction names(*TheNameKeyGenerator);
    Dict previous(*getProperties());
    struct RestoreProperties {
        Dict& live;
        Dict& prior;
        bool accepted=false;
        ~RestoreProperties() noexcept { if(!accepted) live.swap(prior); }
    } properties{*getProperties(),previous};
    verifyValidTeam();
    verifyValidUniqueID();
    properties.accepted=true;
    names.commit();
}

void MapObject::verifyValidTeam(void)
{
	// if this map object has a valid team, then do nothing.
	// if it has an invalid team, the place it on the default neutral team, (by clearing the
	// existing team name.)
	Bool exists;
	AsciiString teamName = getProperties()->getAsciiString(TheKey_originalOwner, &exists);
	if (exists) {
		if(!TheSidesList) throw ERROR_BAD_ARG;
		Bool valid = false;

		int numSides = TheSidesList->getNumTeams();

		for (int i = 0; i < numSides; ++i) {
			TeamsInfo *teamInfo = TheSidesList->getTeamInfo(i);
			if (!teamInfo) {
				continue;
			}

			Bool itBetter;
			AsciiString testAgainstTeamName = teamInfo->getDict()->getAsciiString(TheKey_teamName, &itBetter);
			if (itBetter) {
				if (testAgainstTeamName.compare(teamName) == 0) {
					valid = true;
				}
			}
		}

		if (!valid) {
			getProperties()->remove(TheKey_originalOwner);
		}
	}
}
