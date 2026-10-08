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


// Actual options owner extracted from OptionsMenu.cpp; GUI and excluded Internet
// plumbing no longer own simulation-startup configuration.
#include "PreRTS.h"
#include "Common/UserPreferences.h"
#include "Common/GlobalData.h"
#include "GameNetwork/IPEnumeration.h"
#include <bit>
#include <cerrno>
#include <cstdlib>
#include <limits>
#include <strings.h>
namespace {
bool parseResolution(const char* text, Int& x, Int& y) {
    char* end = nullptr;
    errno = 0;
    const long first = std::strtol(text, &end, 10);
    if (end == text || errno == ERANGE || first < std::numeric_limits<Int>::min() ||
        first > std::numeric_limits<Int>::max()) return false;
    const char* secondStart = end;
    errno = 0;
    const long second = std::strtol(secondStart, &end, 10);
    if (end == secondStart || errno == ERANGE || second < std::numeric_limits<Int>::min() ||
        second > std::numeric_limits<Int>::max()) return false;
    x = static_cast<Int>(first); y = static_cast<Int>(second);
    return true;
}
}
const GlobalData& OptionPreferences::defaults() const {
    const GlobalData* data = m_defaults ? m_defaults : TheGlobalData;
    if (!data) throw ERROR_BAD_ARG;
    return *data;
}
OptionPreferences::OptionPreferences() : UserPreferences(), m_defaults(nullptr)
{
	(void)defaults();
	// note, the superclass will put this in the right dir automatically, this is just a leaf name
	load("Options.ini");
}

OptionPreferences::OptionPreferences(const GlobalData& defaults, NativeUserStorage* storage)
    : UserPreferences(storage), m_defaults(&defaults)
{
    load("Options.ini");
}

OptionPreferences::~OptionPreferences()
{
}






UnsignedInt OptionPreferences::getLANIPAddress(void)
{
	AsciiString selectedIP = (*this)["IPAddress"];
	IPEnumeration IPs;
	EnumeratedIP *IPlist = IPs.getAddresses();
	while (IPlist)
	{
		if (selectedIP.compareNoCase(IPlist->getIPstring()) == 0)
		{
			return IPlist->getIP();
		}
		IPlist = IPlist->getNext();
	}
	return defaults().m_defaultIP;
}

void OptionPreferences::setLANIPAddress( AsciiString IP )
{
	(*this)["IPAddress"] = IP;
}

void OptionPreferences::setLANIPAddress( UnsignedInt IP )
{
	AsciiString tmp;
	tmp.format("%d.%d.%d.%d", ((IP & 0xff000000) >> 24), ((IP & 0xff0000) >> 16), ((IP & 0xff00) >> 8), (IP & 0xff));
	(*this)["IPAddress"] = tmp;
}

UnsignedInt OptionPreferences::getOnlineIPAddress(void)
{
	AsciiString selectedIP = (*this)["GameSpyIPAddress"];
	IPEnumeration IPs;
	EnumeratedIP *IPlist = IPs.getAddresses();
	while (IPlist)
	{
		if (selectedIP.compareNoCase(IPlist->getIPstring()) == 0)
		{
			return IPlist->getIP();
		}
		IPlist = IPlist->getNext();
	}
	return defaults().m_defaultIP;
}

void OptionPreferences::setOnlineIPAddress( AsciiString IP )
{
	(*this)["GameSpyIPAddress"] = IP;
}

void OptionPreferences::setOnlineIPAddress( UnsignedInt IP )
{
	AsciiString tmp;
	tmp.format("%d.%d.%d.%d", ((IP & 0xff000000) >> 24), ((IP & 0xff0000) >> 16), ((IP & 0xff00) >> 8), (IP & 0xff));
	(*this)["GameSpyIPAddress"] = tmp;
}

Bool OptionPreferences::getAlternateMouseModeEnabled(void)
{
	OptionPreferences::const_iterator it = find("UseAlternateMouse");
	if (it == end())
		return defaults().m_useAlternateMouse;

	if (strcasecmp(it->second.str(), "yes") == 0) {
		return TRUE;
	}
	return FALSE;
}

Bool OptionPreferences::getRetaliationModeEnabled(void)
{
	OptionPreferences::const_iterator it = find("Retaliation");
	if (it == end())
		return defaults().m_clientRetaliationModeEnabled;

	if (strcasecmp(it->second.str(), "yes") == 0) {
		return TRUE;
	}
	return FALSE;
}

Bool OptionPreferences::getDoubleClickAttackMoveEnabled(void)
{
	OptionPreferences::const_iterator it = find("UseDoubleClickAttackMove");
	if( it == end() )
		return defaults().m_doubleClickAttackMove;

	if( strcasecmp( it->second.str(), "yes" ) == 0 )
		return TRUE;

	return FALSE;
}

Real OptionPreferences::getScrollFactor(void)
{
	OptionPreferences::const_iterator it = find("ScrollFactor");
	if (it == end())
		return defaults().m_keyboardDefaultScrollFactor;

	Int factor = getInt(it->first, 0);
	if (factor < 0)
		factor = 0;
	if (factor > 100)
		factor = 100;
	
	return factor/100.0f;
}

Bool OptionPreferences::usesSystemMapDir(void)
{
	OptionPreferences::const_iterator it = find("UseSystemMapDir");
	if (it == end())
		return TRUE;

	if (strcasecmp(it->second.str(), "yes") == 0) {
		return TRUE;
	}
	return FALSE;
}

Bool OptionPreferences::saveCameraInReplays(void)
{
	OptionPreferences::const_iterator it = find("SaveCameraInReplays");
	if (it == end())
		return TRUE;

	if (strcasecmp(it->second.str(), "yes") == 0) {
		return TRUE;
	}
	return FALSE;
}

Bool OptionPreferences::useCameraInReplays(void)
{
	OptionPreferences::const_iterator it = find("UseCameraInReplays");
	if (it == end())
		return TRUE;

	if (strcasecmp(it->second.str(), "yes") == 0) {
		return TRUE;
	}
	return FALSE;
}





Bool OptionPreferences::getSendDelay(void)
{
	OptionPreferences::const_iterator it = find("SendDelay");
	if (it == end())
		return defaults().m_firewallSendDelay;

	if (strcasecmp(it->second.str(), "yes") == 0) {
		return TRUE;
	}
	return FALSE;
}

Int OptionPreferences::getFirewallBehavior()
{
	OptionPreferences::const_iterator it = find("FirewallBehavior");
	if (it == end())
		return defaults().m_firewallBehavior;

	Int behavior = getInt(it->first, 0);
	if (behavior < 0)
	{
		behavior = 0;
	}
	return behavior;
}

Short OptionPreferences::getFirewallPortAllocationDelta()
{
	OptionPreferences::const_iterator it = find("FirewallPortAllocationDelta");
	if (it == end()) {
		return defaults().m_firewallPortAllocationDelta;
	}

	const UnsignedShort encoded = static_cast<UnsignedShort>(getInt(it->first, 0));
	Short delta = std::bit_cast<Short>(encoded);
	return delta;
}

UnsignedShort OptionPreferences::getFirewallPortOverride()
{
	OptionPreferences::const_iterator it = find("FirewallPortOverride");
	if (it == end()) {
		return defaults().m_firewallPortOverride;
	}

	Int override = getInt(it->first, 0);
	if (override < 0 || override > 65535)
		override = 0;
	return override;
}

Bool OptionPreferences::getFirewallNeedToRefresh()
{
	OptionPreferences::const_iterator it = find("FirewallNeedToRefresh");
	if (it == end()) {
		return FALSE;
	}

	Bool retval = FALSE;
	AsciiString str = it->second;
	if (str.compareNoCase("TRUE") == 0) {
		retval = TRUE;
	}
	return retval;
}











Bool OptionPreferences::getCloudShadowsEnabled(void)
{
	OptionPreferences::const_iterator it = find("UseCloudMap");
	if (it == end())
		return defaults().m_useCloudMap;

	if (strcasecmp(it->second.str(), "yes") == 0) {
		return TRUE;
	}
	return FALSE;
}

Bool OptionPreferences::getLightmapEnabled(void)
{
	OptionPreferences::const_iterator it = find("UseLightMap");
	if (it == end())
		return defaults().m_useLightMap;

	if (strcasecmp(it->second.str(), "yes") == 0) {
		return TRUE;
	}
	return FALSE;
}

Bool OptionPreferences::getSmoothWaterEnabled(void)
{
	OptionPreferences::const_iterator it = find("ShowSoftWaterEdge");
	if (it == end())
		return defaults().m_showSoftWaterEdge;

	if (strcasecmp(it->second.str(), "yes") == 0) {
		return TRUE;
	}
	return FALSE;
}

Bool OptionPreferences::getTreesEnabled(void)
{
	OptionPreferences::const_iterator it = find("ShowTrees");
	if (it == end())
		return defaults().m_useTrees;

	if (strcasecmp(it->second.str(), "yes") == 0) {
		return TRUE;
	}
	return FALSE;
}

Bool OptionPreferences::getExtraAnimationsDisabled(void)
{
	OptionPreferences::const_iterator it = find("ExtraAnimations");
	if (it == end())
		return defaults().m_useDrawModuleLOD;

	if (strcasecmp(it->second.str(), "yes") == 0) {
		return FALSE;	//we are enabling extra animations, so disabled LOD
	}
	return TRUE;
}

Bool OptionPreferences::getUseHeatEffects(void)
{
	OptionPreferences::const_iterator it = find("HeatEffects");
	if (it == end())
		return defaults().m_useHeatEffects;

	if (strcasecmp(it->second.str(), "yes") == 0) {
		return TRUE;
	}
	return FALSE;
}

Bool OptionPreferences::getDynamicLODEnabled(void)
{
	OptionPreferences::const_iterator it = find("DynamicLOD");
	if (it == end())
		return defaults().m_enableDynamicLOD;

	if (strcasecmp(it->second.str(), "yes") == 0) {
		return TRUE;
	}
	return FALSE;
}

Bool OptionPreferences::getFPSLimitEnabled(void)
{
	OptionPreferences::const_iterator it = find("FPSLimit");
	if (it == end())
		return defaults().m_useFpsLimit;

	if (strcasecmp(it->second.str(), "yes") == 0) {
		return TRUE;
	}
	return FALSE;
}

Bool OptionPreferences::get3DShadowsEnabled(void)
{
	OptionPreferences::const_iterator it = find("UseShadowVolumes");
	if (it == end())
		return defaults().m_useShadowVolumes;

	if (strcasecmp(it->second.str(), "yes") == 0) {
		return TRUE;
	}
	return FALSE;
}

Bool OptionPreferences::get2DShadowsEnabled(void)
{
	OptionPreferences::const_iterator it = find("UseShadowDecals");
	if (it == end())
		return defaults().m_useShadowDecals;

	if (strcasecmp(it->second.str(), "yes") == 0) {
		return TRUE;
	}
	return FALSE;
}

Bool OptionPreferences::getBuildingOcclusionEnabled(void)
{
	OptionPreferences::const_iterator it = find("BuildingOcclusion");
	if (it == end())
		return defaults().m_enableBehindBuildingMarkers;

	if (strcasecmp(it->second.str(), "yes") == 0) {
		return TRUE;
	}
	return FALSE;
}

Int OptionPreferences::getParticleCap(void)
{
	OptionPreferences::const_iterator it = find("MaxParticleCount");
	if (it == end())
		return defaults().m_maxParticleCount;

	Int factor = (Int) getInt(it->first, 0);
	if (factor < 100)	//clamp to at least 100 particles.
		factor = 100;

	return factor;
}

Int OptionPreferences::getTextureReduction(void)
{
	OptionPreferences::const_iterator it = find("TextureReduction");
	if (it == end())
		return -1;	//unknown texture reduction

	Int factor = (Int) getInt(it->first, 0);
	if (factor > 2)	//clamp it.
		factor=2;
	return factor;
}

Real OptionPreferences::getGammaValue(void)
{
	OptionPreferences::const_iterator it = find("Gamma");
 	if (it == end())
 		return 50.0f;
 
 	Real gamma = (Real) getInt(it->first, 0);
 	return gamma;
}

void OptionPreferences::getResolution(Int *xres, Int *yres)
{
	*xres = defaults().m_xResolution;
	*yres = defaults().m_yResolution;

	OptionPreferences::const_iterator it = find("Resolution");
	if (it == end())
		return;

	Int selectedXRes,selectedYRes;
	if (!parseResolution(it->second.str(), selectedXRes, selectedYRes))
		return;

	*xres=selectedXRes;
	*yres=selectedYRes;
}
