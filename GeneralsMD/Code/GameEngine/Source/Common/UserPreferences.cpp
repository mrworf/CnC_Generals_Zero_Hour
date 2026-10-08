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

///////////////////////////////////////////////////////////////////////////////////////
// FILE: UserPreferences.cpp
// Author: Matthew D. Campbell, April 2002
// Description: Saving/Loading of user preferences
///////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/UserPreferences.h"
#include "Common/NativeUserStorage.h"
#include <cerrno>
#include <climits>
#include <cmath>
#include <string_view>
#include <utility>

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE TYPES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE DATA ///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PUBLIC DATA ////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

static AsciiString intAsStr(Int val)
{
	AsciiString ret;
	ret.format("%d", val);
	return ret;
}

static AsciiString boolAsStr(Bool val)
{
	AsciiString ret;
	ret.format("%d", val);
	return ret;
}

static AsciiString realAsStr(Real val)
{
	AsciiString ret;
	ret.format("%g", val);
	return ret;
}

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// UserPreferences Class 
//-----------------------------------------------------------------------------

UserPreferences::UserPreferences(NativeUserStorage* storage) : m_storage(storage)
{
}

UserPreferences::~UserPreferences( void )
{
}

Bool UserPreferences::load(AsciiString fname)
{
	// Preference names are leaves. Never resolve them through the asset VFS.
	const std::string_view name(fname.str(), fname.getLength());
	if (name.empty() || name.find_first_of("/\\:") != name.npos ||
		name == "." || name == "..")
		throw NativeStorageError();
	NativeUserStorage* storage = m_storage ? m_storage : TheNativeUserStorage;
	if (!storage) throw NativeStorageError();
	auto bytes = storage->readFile(NativeUserArea::Data, name, INT_MAX);
	if (!bytes) {
		// A missing first-boot file is accepted: retain values and the writable leaf.
		m_filename.swap(fname);
		return false;
	}
	if (std::find(bytes->begin(), bytes->end(), 0) != bytes->end())
		throw NativeStorageError();
	PreferenceMap candidate(*this); // original load overlays existing values
	std::string_view remaining;
	if (!bytes->empty())
		remaining = std::string_view(reinterpret_cast<const char*>(bytes->data()), bytes->size());
	while (!remaining.empty()) {
		const auto newline = remaining.find('\n');
		const auto row = remaining.substr(0, newline);
		if (row.size() > AsciiString::MAX_LEN) throw NativeStorageError();
		const auto equal = row.find('=');
		if (equal != row.npos) {
			AsciiString key(std::string(row.substr(0, equal)).c_str());
			AsciiString value(std::string(row.substr(equal + 1)).c_str());
			key.trim(); value.trim();
			if (!key.isEmpty() && !value.isEmpty())
				candidate.insert_or_assign(std::move(key), std::move(value));
		}
		if (newline == remaining.npos) break;
		remaining.remove_prefix(newline + 1);
	}
	PreferenceMap::swap(candidate);
	m_filename.swap(fname); // both publications are non-throwing
	return true;
}

Bool UserPreferences::write()
{
	if (m_filename.isEmpty()) return false;
	NativeUserStorage* storage = m_storage ? m_storage : TheNativeUserStorage;
	if (!storage) throw NativeStorageError();
	// Validate the complete map before opening a publication candidate.
	for (const auto& [key, value] : *this) {
		if (key.isEmpty() ||
			std::strpbrk(key.str(), "=\r\n") || std::strpbrk(value.str(), "\r\n"))
			throw NativeStorageError();
	}
	auto output = storage->beginWrite(NativeUserArea::Data, m_filename.str());
	for (const auto& [key, value] : *this) {
		output->write(key.str(), key.getLength());
		output->write(" = ", 3);
		output->write(value.str(), value.getLength());
		output->write("\n", 1);
	}
	// A successful rename with uncertain directory durability is still published.
	(void)output->commit();
	return true;
}

Bool UserPreferences::getBool(AsciiString key, Bool defaultValue) const
{
	AsciiString val = getAsciiString(key, AsciiString::TheEmptyString);
	if (val.isEmpty())
	{
		return defaultValue;
	}

	val.toLower();
	return (val == "1" || val == "t" || val == "true" || val == "y" || val == "yes" || val == "ok");
}

Real UserPreferences::getReal(AsciiString key, Real defaultValue) const
{
	AsciiString val = getAsciiString(key, AsciiString::TheEmptyString);
	if (val.isEmpty())
	{
		return defaultValue;
	}

	errno = 0;
	const Real parsed = std::strtof(val.str(), nullptr);
	return errno == ERANGE || !std::isfinite(parsed) ? defaultValue : parsed;
}

Int UserPreferences::getInt(AsciiString key, Int defaultValue) const
{
	AsciiString val = getAsciiString(key, AsciiString::TheEmptyString);
	if (val.isEmpty())
	{
		return defaultValue;
	}

	errno = 0;
	const long parsed = std::strtol(val.str(), nullptr, 10);
	return errno == ERANGE || parsed < INT_MIN || parsed > INT_MAX
		? defaultValue : static_cast<Int>(parsed);
}

AsciiString UserPreferences::getAsciiString(AsciiString key, AsciiString defaultValue) const
{
	UserPreferences::const_iterator it = find(key);
	if (it == end())
	{
		return defaultValue;
	}

	return it->second;
}

void UserPreferences::setBool(AsciiString key, Bool val)
{
	insert_or_assign(std::move(key), boolAsStr(val));
}

void UserPreferences::setReal(AsciiString key, Real val)
{
	if (!std::isfinite(val)) throw NativeStorageError();
	insert_or_assign(std::move(key), realAsStr(val));
}

void UserPreferences::setInt(AsciiString key, Int val)
{
	insert_or_assign(std::move(key), intAsStr(val));
}

void UserPreferences::setAsciiString(AsciiString key, AsciiString val)
{
	insert_or_assign(std::move(key), std::move(val));
}

// Obsolete Internet profile preferences are excluded from the native runtime.
