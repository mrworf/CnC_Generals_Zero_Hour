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

// FILE: W3DModelDraw.cpp ///////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:   Default w3d draw module
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Common/W3DModelConditionData.h"
#include "Common/Errors.h"
#include <cmath>

void ModelConditionInfo::addPublicBone(const AsciiString& boneName) const
{
	if (boneName.isEmpty() || boneName.isNone())
		return;

	AsciiString tmp = boneName;
	tmp.toLower();
	if (std::find(m_publicBones.begin(), m_publicBones.end(), tmp) == m_publicBones.end())
	{
		m_publicBones.push_back(tmp);
	}
}

Bool ModelConditionInfo::matchesMode(Bool night, Bool snowy) const
{
	for (std::vector<ModelConditionFlags>::const_iterator it = m_conditionsYesVec.begin();
				it != m_conditionsYesVec.end();
				++it)
	{
		if (it->test(MODELCONDITION_NIGHT) == (night) &&
				it->test(MODELCONDITION_SNOW) == (snowy))
		{
			return true;
		}
	}
	return false;
}

void ModelConditionInfo::clear()
{
	int i;
#if defined(_DEBUG) || defined(_INTERNAL)
	m_description.clear();
#endif
	m_conditionsYesVec.clear();
	m_modelName.clear();
	for (i = 0; i < MAX_TURRETS; ++i)
	{
		m_turrets[i].clear();
	}
	m_hideShowVec.clear();
	for (i = 0; i < WEAPONSLOT_COUNT; ++i)
	{
		m_weaponFireFXBoneName[i].clear();
		m_weaponRecoilBoneName[i].clear();
		m_weaponMuzzleFlashName[i].clear();
		m_weaponProjectileLaunchBoneName[i].clear();
		m_weaponBarrelInfoVec[i].clear();
		m_hasRecoilBonesOrMuzzleFlashes[i] = false;
	}
	m_particleSysBones.clear();
	m_animations.clear();
	m_flags = 0;
	m_transitionKey = NAMEKEY_INVALID;
	m_allowToFinishKey = NAMEKEY_INVALID;
	m_iniReadFlags = 0;
	m_mode = 2;
	m_transitionSig = NO_TRANSITION;
	m_animMinSpeedFactor = 1.0f;
	m_animMaxSpeedFactor = 1.0f;
	m_pristineBones.clear();
	m_validStuff = 0;
}

W3DAnimationInfo::W3DAnimationInfo(const AsciiString& name, Bool idle, Real distance)
  : m_name(name), m_distanceCovered(distance), m_naturalDurationInMsec(-1), m_isIdleAnim(idle) {}
W3DAnimationInfo::W3DAnimationInfo(const W3DAnimationInfo&) = default;
W3DAnimationInfo& W3DAnimationInfo::operator=(const W3DAnimationInfo&) = default;
W3DAnimationInfo::~W3DAnimationInfo() = default;

ModelConditionInfo::NativeBinding ModelConditionInfo::prepareNativeBinding(NativeW3DAssets& assets, Real scale) const {
  if (!std::isfinite(scale)) throw ERROR_BAD_ARG;
  NativeBinding candidate;
  if (m_modelName.isEmpty() || m_modelName.isNone()) return candidate;
  if (!m_animations.empty() && m_animations.front().getName().isEmpty()) throw ERROR_BAD_ARG;
  auto root = NativeW3DPose::identity(); root[0] = root[5] = root[10] = scale;
  // Original Create_Render_Obj scales its HTree only beyond the .01 epsilon;
  // validateCachedBones additionally scales the root matrix unconditionally.
  const float treeScale = std::fabs(scale - 1.f) > .01f ? scale : 1.f;
  const std::string_view animation = m_animations.empty() ? std::string_view{} : m_animations.front().getName().str();
  return assets.bind(m_modelName.str(), animation, (m_flags & (1 << 4)) != 0, root, treeScale);
}
