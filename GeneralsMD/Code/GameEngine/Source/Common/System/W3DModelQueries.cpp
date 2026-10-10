// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Electronic Arts Inc. Original pristine weapon/turret queries.
#include "Common/W3DModelConditionData.h"
#include "Common/NameKeyGenerator.h"
#include "Common/Errors.h"
#include <strings.h>

namespace {
bool sameName(std::string_view a, std::string_view b) noexcept {
  if(a.size()!=b.size())return false;
  for(std::size_t i=0;i<a.size();++i){
    const auto lower=[](unsigned char c){return c>='A' && c<='Z'?c+('a'-'A'):c;};
    if(lower(a[i])!=lower(b[i]))return false;
  }
  return true;
}
const NativeW3DAssets::Attachment* subobject(const NativeW3DAssets::Model& model,std::string_view name) {
  if(!model)return nullptr;
  // Original Get_Sub_Object enumerates all LODs, then additional models;
  // Get_Sub_Object_By_Name completes the full-name pass before suffix lookup.
  for(bool suffix:{false,true}){
    auto match=[&](const auto& objects)->const NativeW3DAssets::Attachment*{
      for(const auto& object:objects){auto candidate=std::string_view(object.model->name);
        if(suffix){auto dot=candidate.find('.');if(dot!=candidate.npos)candidate.remove_prefix(dot+1);}
        if(sameName(candidate,name))return &object;
      }return nullptr;
    };
    for(const auto& level:model->levels)if(auto result=match(level.objects))return result;
    if(auto result=match(model->aggregates))return result;
  }return nullptr;
}
void discover(const ModelConditionInfo::NativeBinding& binding,const AsciiString& prefix,PristineBoneInfoMap& map) {
  if(prefix.isEmpty() || prefix.isNone() || !binding.model)return;
  AsciiString lower=prefix;lower.toLower();
  auto add=[&](const AsciiString& name,bool child){
    std::size_t index=0;
    if(child){auto object=subobject(binding.model,name.str());if(!object)return false;index=object->bone;}
    else {if(!binding.model->hierarchy)return false;index=binding.pose.sourceBoneIndex(name.str());if(!index)return false;}
    PristineBoneInfo info;info.boneIndex=static_cast<Int>(index);
    if(binding.model->hierarchy){const auto& matrix=binding.pose.bone(index).transform;
      for(unsigned row=0;row<3;++row)for(unsigned col=0;col<4;++col)info.mtx[row][col]=matrix[row*4+col];}
    else info.mtx.Make_Identity();
    map[NAMEKEY(name)]=info;return true;
  };
  auto sweep=[&](bool child){bool found=add(lower,child);AsciiString numbered;
    for(Int i=1;i<=99;++i){numbered.format("%s%02d",lower.str(),i);if(!add(numbered,child))break;found=true;}
    return found;
  };
  if(!sweep(false))sweep(true);
}
}

ModelConditionInfo::NativeQuery ModelConditionInfo::prepareNativeQueries(NativeW3DAssets& assets,Real scale,
    const std::vector<AsciiString>& extra,const std::vector<AsciiString>& standard) const {
  if(!TheNameKeyGenerator)throw ERROR_BAD_ARG;
  // Destruction order is candidate -> asset admission -> key rollback.
  NameKeyTransaction keys(*TheNameKeyGenerator);
  NativeW3DAssets::Transaction admission(assets);
  NativeQuery candidate;
  candidate.condition=std::make_unique<ModelConditionInfo>(*this);
  auto& data=*candidate.condition;
  data.m_validStuff=0;data.m_pristineBones.clear();
  for(const auto& name:extra)data.addPublicBone(name);
  candidate.binding=prepareNativeBinding(assets,scale);
  for(const auto& name:standard)discover(candidate.binding,name,data.m_pristineBones);
  for(const auto& name:data.m_publicBones)discover(candidate.binding,name,data.m_pristineBones);
  data.m_validStuff=PUBLIC_BONES_VALID|PRISTINE_BONES_VALID;
  data.buildTurretInfo(true);data.buildWeaponBarrelInfo();
  admission.commit();keys.commit();return candidate;
}

void ModelConditionInfo::buildWeaponBarrelInfo() const
{
	for (int wslot = 0; wslot < WEAPONSLOT_COUNT; ++wslot)
	{
		m_weaponBarrelInfoVec[wslot].clear();
		m_hasRecoilBonesOrMuzzleFlashes[wslot] = false;

		const AsciiString& fxBoneName = m_weaponFireFXBoneName[wslot];
		const AsciiString& recoilBoneName = m_weaponRecoilBoneName[wslot];
		const AsciiString& mfName = m_weaponMuzzleFlashName[wslot];
		const AsciiString& plbName = m_weaponProjectileLaunchBoneName[wslot];

// a useful tool for finding missing INI entries in the weapon-bone block
// since this class has no idea which object it refers to, it must assume that the projectilelaunchbonename
// is required if the fxBoneName is specified
// this is not always true, since some weapons have no launched projectiles, hence the caveat in the assert message
//#if defined(_DEBUG) || defined(_INTERNAL)
//    if (
//          ( m_modelName.startsWith("UV") || m_modelName.startsWith("NV") || m_modelName.startsWith("AV") ||
//            m_modelName.startsWith("uv") || m_modelName.startsWith("nv") || m_modelName.startsWith("av")
//          )
//
//          && fxBoneName.isNotEmpty()
//
//      )
//    DEBUG_ASSERTCRASH( plbName.isNotEmpty(), ("You appear to have a missing projectilelaunchbonename. \n Promptly ignore this assert if this model is used by a non-projectile-weapon-bearing unit\nModel name = %s.", m_modelName.str()) );
//#endif

		if (fxBoneName.isNotEmpty() || recoilBoneName.isNotEmpty() || mfName.isNotEmpty() || plbName.isNotEmpty())
		{
			Int prevFxBone = 0;
			AsciiString buffer;
			for (Int i = 1; i <= 99; ++i)
			{
				WeaponBarrelInfo info;
				info.m_projectileOffsetMtx.Make_Identity();

				if (!recoilBoneName.isEmpty())
				{
					buffer.format( "%s%02d", recoilBoneName.str(), i);
					findPristineBone(NAMEKEY(buffer), &info.m_recoilBone);
				}
				if (!mfName.isEmpty())
				{
					buffer.format( "%s%02d", mfName.str(), i);
					findPristineBone(NAMEKEY(buffer), &info.m_muzzleFlashBone);
#if defined(_DEBUG) || defined(_INTERNAL)
					if (info.m_muzzleFlashBone)
						info.m_muzzleFlashBoneName = buffer;
#endif
				}
				if (!fxBoneName.isEmpty())
				{
					buffer.format( "%s%02d", fxBoneName.str(), i);
					findPristineBone(NAMEKEY(buffer), &info.m_fxBone);
					// special case: if we have multiple muzzleflashes, but only one fxbone, use that fxbone for everything.
					if (info.m_fxBone == 0 && info.m_muzzleFlashBone != 0)
						info.m_fxBone = prevFxBone;
				}

				Int plbBoneIndex = 0;
				if (!plbName.isEmpty())
				{
					buffer.format( "%s%02d", plbName.str(), i);
					const Matrix3D* mtx = findPristineBone(NAMEKEY(buffer), &plbBoneIndex);
					if (mtx != NULL)
						info.m_projectileOffsetMtx = *mtx;
				}

				if (info.m_fxBone == 0 && info.m_recoilBone == 0 && info.m_muzzleFlashBone == 0 && plbBoneIndex == 0)
					break;

				m_weaponBarrelInfoVec[wslot].push_back(info);

				if (info.m_recoilBone != 0 || info.m_muzzleFlashBone != 0)
					m_hasRecoilBonesOrMuzzleFlashes[wslot] = true;

				prevFxBone = info.m_fxBone;
			}

			if (m_weaponBarrelInfoVec[wslot].empty())
			{
				// try the unadorned names
				WeaponBarrelInfo info;
				if (!recoilBoneName.isEmpty())
					findPristineBone(NAMEKEY(recoilBoneName), &info.m_recoilBone);

				if (!mfName.isEmpty())
					findPristineBone(NAMEKEY(mfName), &info.m_muzzleFlashBone);

#if defined(_DEBUG) || defined(_INTERNAL)
				if (info.m_muzzleFlashBone)
					info.m_muzzleFlashBoneName = mfName;
#endif

				const Matrix3D* plbMtx = plbName.isEmpty() ? NULL : findPristineBone(NAMEKEY(plbName), NULL);
				if (plbMtx != NULL)
					info.m_projectileOffsetMtx = *plbMtx;
				else
					info.m_projectileOffsetMtx.Make_Identity();

				if (!fxBoneName.isEmpty())
					findPristineBone(NAMEKEY(fxBoneName), &info.m_fxBone);

				if (info.m_fxBone != 0 || info.m_recoilBone != 0 || info.m_muzzleFlashBone != 0 || plbMtx != NULL)
				{
					m_weaponBarrelInfoVec[wslot].push_back(info);
					if (info.m_recoilBone != 0 || info.m_muzzleFlashBone != 0)
						m_hasRecoilBonesOrMuzzleFlashes[wslot] = true;
				}
				else
				{
				}
			}	// if empty

			DEBUG_ASSERTCRASH(!(m_modelName.isNotEmpty() && m_weaponBarrelInfoVec[wslot].empty()), ("*** ASSET ERROR: No fx bone named '%s' found in model %s!\n",fxBoneName.str(),m_modelName.str()));
		}
	}
	m_validStuff |= BARRELS_VALID;
}


void ModelConditionInfo::buildTurretInfo(Bool logicAllowed) const
{
	for (int tslot = 0; tslot < MAX_TURRETS; ++tslot)
	{
		TurretInfo& tur = m_turrets[tslot];
		if (!logicAllowed || m_modelName.isEmpty())
		{
			tur.m_turretAngleBone = 0;
			tur.m_turretPitchBone = 0;
			continue;
		}

		if (tur.m_turretAngleNameKey != NAMEKEY_INVALID)
		{
			if (findPristineBone(tur.m_turretAngleNameKey, &tur.m_turretAngleBone) == NULL)
			{
				DEBUG_CRASH(("*** ASSET ERROR: TurretBone %s not found! (%s)\n",KEYNAME(tur.m_turretAngleNameKey).str(),m_modelName.str()));
				tur.m_turretAngleBone = 0;
			}
		}
		else
		{
			tur.m_turretAngleBone = 0;
		}

		if (tur.m_turretPitchNameKey != NAMEKEY_INVALID)
		{
			if (findPristineBone(tur.m_turretPitchNameKey, &tur.m_turretPitchBone) == NULL)
			{
				DEBUG_CRASH(("*** ASSET ERROR: TurretBone %s not found! (%s)\n",KEYNAME(tur.m_turretPitchNameKey).str(),m_modelName.str()));
				tur.m_turretPitchBone = 0;
			}
		}
		else
		{
			tur.m_turretPitchBone = 0;
		}
	}

	if (logicAllowed)
	{
		m_validStuff |= TURRETS_VALID;
	}
}


const Matrix3D* ModelConditionInfo::findPristineBone(NameKeyType boneName, Int* boneIndex) const
{
	DEBUG_ASSERTCRASH((m_validStuff & PRISTINE_BONES_VALID), ("*** ASSET ERROR: bones are not valid"));
	if (!(m_validStuff & PRISTINE_BONES_VALID))
	{
		// set it to zero, some callers rely on this
		if (boneIndex)
			*boneIndex = 0;
		return NULL;
	}

	if (boneName == NAMEKEY_INVALID)
	{
		// set it to zero, some callers rely on this
		if (boneIndex)
			*boneIndex = 0;
		return NULL;
	}

	PristineBoneInfoMap::const_iterator it = m_pristineBones.find(boneName);
	if (it != m_pristineBones.end())
	{
		if (boneIndex)
			*boneIndex = it->second.boneIndex;
		return &it->second.mtx;
	}
	else
	{
		// set it to zero -- some callers rely on this!
		if (boneIndex)
			*boneIndex = 0;
		return NULL;
	}
}

//-------------------------------------------------------------------------------------------------
Bool ModelConditionInfo::findPristineBonePos(NameKeyType boneName, Coord3D& pos) const
{
	const Matrix3D* mtx = findPristineBone(boneName, NULL);
	if (mtx)
	{
		Vector3 v = mtx->Get_Translation();
		pos.x = v.X;
		pos.y = v.Y;
		pos.z = v.Z;
		return true;
	}
	else
	{
		pos.zero();
		return false;
	}
}
