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

// FILE: Dict.h 
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information					         
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  Dict.h
//
// Created:    Steven Johnson, November 2001
//
// Desc:       General-purpose dictionary class
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef Dict_H
#define Dict_H

#include "Common/Errors.h"
#include "Common/NameKeyGenerator.h"
#include "Common/UnicodeString.h"
#include <memory>
#include <variant>
#include <vector>

// -----------------------------------------------------
/**
	Dict provides a general utility class for maintaining 
	a sorted key-value pair list. Keys are currently required
	to be of type NameKeyType, and data may be Bool, int, real, 
	or string.

	Current implementation keeps the pairs sorted by key, and
	does a binary search for lookups; this might change in the future,
	depending on usage and performance (e.g., to a hashtable). So
	don't rely on the quirks of this implementation.
*/
class Dict
{
public:
	enum 
	{ 
		MAX_LEN = 32767							///< max total len of any Dict, in Pairs
	};

	enum DataType : Int
	{
		DICT_NONE = -1,	// this is returned by getType and getNthType to indicate "invalid key/index"
		DICT_BOOL = 0,	// note, we rely on the fact that this constant is zero in the code. so don't change it.
		DICT_INT,
		DICT_REAL,
		DICT_ASCIISTRING,
		DICT_UNICODESTRING
	};

	/**
		Default constructor -- construct a new, empty Dict.
	*/
	Dict(Int numPairsToPreAllocate = 0);

	/**
		Copy constructor -- make this Dict identical to the
		other Dict. (This is actually quite efficient, because
		they will simply share the same data and increment the
		refcount.)
	*/
	Dict(const Dict& src) = default;

	/**
		Destructor. Not too exciting... clean up the works and such.
	*/
	~Dict() = default;

	/**
	*/
	Dict& operator=(const Dict& src) = default;

	/**
		remove all pairs.
	*/
	void clear();

	/**
		return the number of key-value pairs in the dict.
	*/
	Int getPairCount() const;

	/**
		Return the key for the nth pair (0-based index).
		return NAMEKEY_INVALID if n is out of range.
	*/
	NameKeyType getNthKey(Int n) const;

	/**
		Return the datatype for the pair with the given key.
		return DICT_NONE if no pair with the key exists.
	*/
	DataType getType(NameKeyType key) const;

	/**
		Return there is a pair with the given key and datatype, return true.
	*/
	inline Bool known(NameKeyType key, DataType d) const
	{
		return getType(key) == d;
	}

	/**
		Return the datatype for the nth pair (0-based index).
		return DICT_NONE if n is out of range.
	*/
	DataType getNthType(Int n) const;

	/**
		return the value for the pair with the given key.
		if there is no pair with the given key, or the value is 
		not of the correct type, 0 is returned.
	*/
	Bool getBool(NameKeyType key, Bool* exists = NULL) const;
	/**
		return the value for the pair with the given key.
		if there is no pair with the given key, or the value is 
		not of the correct type, 0 is returned.
	*/
	Int getInt(NameKeyType key, Bool* exists = NULL) const;
	/**
		return the value for the pair with the given key.
		if there is no pair with the given key, or the value is 
		not of the correct type, 0 is returned.
	*/
	Real getReal(NameKeyType key, Bool* exists = NULL) const;
	/**
		return the value for the pair with the given key.
		if there is no pair with the given key, or the value is 
		not of the correct type, "" is returned.
	*/
	AsciiString getAsciiString(NameKeyType key, Bool* exists = NULL) const;
	/**
		return the value for the pair with the given key.
		if there is no pair with the given key, or the value is 
		not of the correct type, "" is returned.
	*/
	UnicodeString getUnicodeString(NameKeyType key, Bool* exists = NULL) const;

	/**
		return the value for the pair with the given index.
		if the index is out of range, or the value is 
		not of the correct type, 0 is returned.
	*/
	Bool getNthBool(Int n) const;
	/**
		return the value for the pair with the given index.
		if the index is out of range, or the value is 
		not of the correct type, 0 is returned.
	*/
	Int getNthInt(Int n) const;
	/**
		return the value for the pair with the given index.
		if the index is out of range, or the value is 
		not of the correct type, 0 is returned.
	*/
	Real getNthReal(Int n) const;
	/**
		return the value for the pair with the given index.
		if the index is out of range, or the value is 
		not of the correct type, "" is returned.
	*/
	AsciiString getNthAsciiString(Int n) const;
	/**
		return the value for the pair with the given index.
		if the index is out of range, or the value is 
		not of the correct type, "" is returned.
	*/
	UnicodeString getNthUnicodeString(Int n) const;

	/**
		set the value for the pair with the given key.
		if no such pair exists, it is created.
		if such a pair exists, it is replaced.
		note that when replacing a pair, the new and old
		data types need not match.
	*/
	void setBool(NameKeyType key, Bool value);
	/**
		set the value for the pair with the given key.
		if no such pair exists, it is created.
		if such a pair exists, it is replaced.
		note that when replacing a pair, the new and old
		data types need not match.
	*/
	void setInt(NameKeyType key, Int value);
	/**
		set the value for the pair with the given key.
		if no such pair exists, it is created.
		if such a pair exists, it is replaced.
		note that when replacing a pair, the new and old
		data types need not match.
	*/
	void setReal(NameKeyType key, Real value);
	/**
		set the value for the pair with the given key.
		if no such pair exists, it is created.
		if such a pair exists, it is replaced.
		note that when replacing a pair, the new and old
		data types need not match.
	*/
	void setAsciiString(NameKeyType key, const AsciiString& value);
	/**
		set the value for the pair with the given key.
		if no such pair exists, it is created.
		if such a pair exists, it is replaced.
		note that when replacing a pair, the new and old
		data types need not match.
	*/
	void setUnicodeString(NameKeyType key, const UnicodeString& value);

	/**
		remove the pair with the given key. if such a pair existed, return true.
		if no such pair existed, return false.
	*/
	Bool remove(NameKeyType key);

	/**
		copy the pair with the given key from 'that', replacing any such pair in 'this'.
		if no such pair exists in 'that', any pair with that key will be removed from 'this'.
	*/
	void copyPairFrom(const Dict& that, NameKeyType key);

private:
    // Typed values have real lifetimes. No strings are constructed in void*
    // storage, and no byte-zero/refcount convention leaks into native storage.
    using Value = std::variant<Bool, Int, Real, AsciiString, UnicodeString>;
    struct Pair {
        NameKeyType key;
        std::shared_ptr<const Value> value;
    };
    struct Data {
        std::vector<Pair> pairs;
    };
    std::shared_ptr<Data> m_data;

    static void validateKey(NameKeyType key);
    const Pair* findPairByKey(NameKeyType key) const;
    const Pair* nthPair(Int n) const;
    void setValue(NameKeyType key, const Value& value);
    template<class T> T getValue(const Pair* pair, Bool* exists = nullptr) const;
};

#endif // Dict_H
