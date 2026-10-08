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
//  (c) 2001-2003 Electronic Arts Inc.
//  //
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: Dict.cpp
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
// File name:  Dict.cpp
//
// Created:    Steven Johnson, November 2001
//
// Desc:       General-purpose dictionary class
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#include "Common/Dict.h"
#include <algorithm>

void Dict::validateKey(NameKeyType key) {
  if (key <= NAMEKEY_INVALID || key > NAMEKEY_MAX)
    throw ERROR_BAD_ARG;
}

Dict::Dict(Int reserve) {
  if (reserve < 0)
    throw ERROR_BAD_ARG;
  if (reserve > MAX_LEN)
    throw ERROR_OUT_OF_MEMORY;
  if (reserve) {
    auto candidate = std::make_shared<Data>();
    candidate->pairs.reserve(static_cast<std::size_t>(reserve));
    m_data = std::move(candidate);
  }
}

void Dict::clear() { m_data.reset(); }
Int Dict::getPairCount() const {
  return m_data ? static_cast<Int>(m_data->pairs.size()) : 0;
}

const Dict::Pair *Dict::findPairByKey(NameKeyType key) const {
  validateKey(key);
  if (!m_data)
    return nullptr;
  const auto &pairs = m_data->pairs;
  auto found = std::lower_bound(
      pairs.begin(), pairs.end(), key,
      [](const Pair &pair, NameKeyType name) { return pair.key < name; });
  return found != pairs.end() && found->key == key ? &*found : nullptr;
}

const Dict::Pair *Dict::nthPair(Int n) const {
  return n >= 0 && n < getPairCount()
             ? &m_data->pairs[static_cast<std::size_t>(n)]
             : nullptr;
}

NameKeyType Dict::getNthKey(Int n) const {
  const auto *pair = nthPair(n);
  return pair ? pair->key : NAMEKEY_INVALID;
}

Dict::DataType Dict::getType(NameKeyType key) const {
  const auto *pair = findPairByKey(key);
  return pair ? static_cast<DataType>(pair->value->index()) : DICT_NONE;
}

Dict::DataType Dict::getNthType(Int n) const {
  const auto *pair = nthPair(n);
  return pair ? static_cast<DataType>(pair->value->index()) : DICT_NONE;
}

template <class T> T Dict::getValue(const Pair *pair, Bool *exists) const {
  const T *value = pair ? std::get_if<T>(pair->value.get()) : nullptr;
  if (exists)
    *exists = value != nullptr;
  return value ? *value : T{};
}

Bool Dict::getBool(NameKeyType key, Bool *exists) const {
  return getValue<Bool>(findPairByKey(key), exists);
}
Int Dict::getInt(NameKeyType key, Bool *exists) const {
  return getValue<Int>(findPairByKey(key), exists);
}
Real Dict::getReal(NameKeyType key, Bool *exists) const {
  return getValue<Real>(findPairByKey(key), exists);
}
AsciiString Dict::getAsciiString(NameKeyType key, Bool *exists) const {
  return getValue<AsciiString>(findPairByKey(key), exists);
}
UnicodeString Dict::getUnicodeString(NameKeyType key, Bool *exists) const {
  return getValue<UnicodeString>(findPairByKey(key), exists);
}
Bool Dict::getNthBool(Int n) const { return getValue<Bool>(nthPair(n)); }
Int Dict::getNthInt(Int n) const { return getValue<Int>(nthPair(n)); }
Real Dict::getNthReal(Int n) const { return getValue<Real>(nthPair(n)); }
AsciiString Dict::getNthAsciiString(Int n) const {
  return getValue<AsciiString>(nthPair(n));
}
UnicodeString Dict::getNthUnicodeString(Int n) const {
  return getValue<UnicodeString>(nthPair(n));
}

void Dict::setValue(NameKeyType key, const Value &value) {
  const auto *prior = findPairByKey(key);
  if (!prior && getPairCount() == MAX_LEN)
    throw ERROR_OUT_OF_MEMORY;
  // Payload construction (including string reference admission) precedes
  // clone/growth and mutation. Pair shifts only move noexcept shared owners.
  static_assert(std::is_nothrow_move_constructible_v<Pair> &&
                std::is_nothrow_move_assignable_v<Pair>);
  auto payload = std::make_shared<const Value>(value);
  auto candidate = m_data && m_data.unique() ? m_data
                   : m_data                  ? std::make_shared<Data>(*m_data)
                                             : std::make_shared<Data>();
  auto &pairs = candidate->pairs;
  if (!prior && pairs.size() == pairs.capacity()) {
    const auto grown = std::min<std::size_t>(
        MAX_LEN, std::max<std::size_t>(8, pairs.size() * 2));
    pairs.reserve(grown);
  }
  auto position = std::lower_bound(
      pairs.begin(), pairs.end(), key,
      [](const Pair &pair, NameKeyType name) { return pair.key < name; });
  if (position != pairs.end() && position->key == key)
    position->value.swap(payload);
  else
    pairs.insert(position, Pair{key, std::move(payload)});
  m_data.swap(candidate);
}

void Dict::setBool(NameKeyType key, Bool value) { setValue(key, Value(value)); }
void Dict::setInt(NameKeyType key, Int value) { setValue(key, Value(value)); }
void Dict::setReal(NameKeyType key, Real value) { setValue(key, Value(value)); }
void Dict::setAsciiString(NameKeyType key, const AsciiString &value) {
  setValue(key, Value(value));
}
void Dict::setUnicodeString(NameKeyType key, const UnicodeString &value) {
  setValue(key, Value(value));
}

Bool Dict::remove(NameKeyType key) {
  if (!findPairByKey(key))
    return false;
  auto candidate = m_data.unique() ? m_data : std::make_shared<Data>(*m_data);
  auto &pairs = candidate->pairs;
  auto position = std::lower_bound(
      pairs.begin(), pairs.end(), key,
      [](const Pair &pair, NameKeyType name) { return pair.key < name; });
  pairs.erase(position);
  m_data.swap(candidate);
  return true;
}

void Dict::copyPairFrom(const Dict &that, NameKeyType key) {
  const auto *source = that.findPairByKey(key);
  if (source)
    setValue(key, *source->value);
  else
    remove(key);
}
