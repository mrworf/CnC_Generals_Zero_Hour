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

// FILE: NameKeyGenerator.cpp /////////////////////////////////////////////////////////////////////
// Created:   Michael Booth, May 2001
//						Colin Day, May 2001
// Desc:      Name key system to translate between names and unique key ids
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/NameKeyGenerator.h"
#include "Common/INI.h"
#include <atomic>
#include <strings.h>

namespace {
UnsignedInt64 nextNamespaceGeneration() {
    static std::atomic<UnsignedInt64> generation{0};
    auto prior=generation.load(std::memory_order_relaxed);
    do {if(prior==std::numeric_limits<UnsignedInt64>::max())throw ERROR_OUT_OF_MEMORY;}
    while(!generation.compare_exchange_weak(prior,prior+1,std::memory_order_relaxed));
    return prior+1;
}
}

// Public Data ////////////////////////////////////////////////////////////////////////////////////
NameKeyGenerator *TheNameKeyGenerator = NULL;  ///< name key gen. singleton

//------------------------------------------------------------------------------------------------- 
NameKeyGenerator::NameKeyGenerator(Int capacity):m_capacity(UnsignedInt(capacity))
{
    if(capacity<1 || capacity>NAMEKEY_MAX)throw ERROR_BAD_ARG;

	m_nextID = (UnsignedInt)NAMEKEY_INVALID;  // uninitialized system

	for (Int i = 0; i < SOCKET_COUNT; ++i)
		m_sockets[i] = NULL;

}  // end NameKeyGenerator

//------------------------------------------------------------------------------------------------- 
NameKeyGenerator::~NameKeyGenerator()
{
	
	// free all system data
	freeSockets();

}  // end ~NameKeyGenerator

//------------------------------------------------------------------------------------------------- 
void NameKeyGenerator::init()
{
	if(m_transactionDepth) throw ERROR_BAD_ARG;
	DEBUG_ASSERTCRASH(m_nextID == (UnsignedInt)NAMEKEY_INVALID, ("NameKeyGen already inited"));
    if(m_nextID!=UnsignedInt(NAMEKEY_INVALID))throw ERROR_BAD_ARG;
    const auto generation=nextNamespaceGeneration();

	// start keys at the beginning again
	freeSockets();
	m_nextID = 1;
    m_generation=generation;

}  // end init

//------------------------------------------------------------------------------------------------- 
void NameKeyGenerator::reset()
{
    if(m_transactionDepth) throw ERROR_BAD_ARG;
    const auto generation=nextNamespaceGeneration();
	freeSockets();
	m_nextID = 1;
    m_generation=generation;

}  // end reset

NameKeyTransaction::NameKeyTransaction(NameKeyGenerator& owner)
  :m_owner(owner),m_start(owner.m_nextID),m_rollbackGeneration(0) {
  if(!m_start || owner.m_transactionDepth==std::numeric_limits<UnsignedInt>::max()) throw ERROR_BAD_ARG;
  // Reserve a unique cache-invalidation token before acquiring anything.
  // Rollback must not throw when the global issuance boundary is exhausted.
  m_rollbackGeneration=nextNamespaceGeneration();
  ++owner.m_transactionDepth;
}
NameKeyTransaction::~NameKeyTransaction() noexcept {
  if(!m_committed && m_owner.m_nextID!=m_start) {
    // Every insertion pushes onto its socket. Assigned ordinals are ownership
    // units; no allocations/callbacks or unowned key retirement are needed.
    for(Bucket*& socket:m_owner.m_sockets)
      while(socket && UnsignedInt(socket->m_key)>=m_start) {
        Bucket* retired=socket;socket=retired->m_nextInSocket;retired->deleteInstance();
      }
    m_owner.m_nextID=m_start;
    m_owner.m_generation=m_rollbackGeneration;
  }
  --m_owner.m_transactionDepth;
}

//------------------------------------------------------------------------------------------------- 
void NameKeyGenerator::freeSockets()
{
	for (Int i = 0; i < SOCKET_COUNT; ++i)
	{
		Bucket *next;
		for (Bucket *b = m_sockets[i]; b; b = next)
		{
			next = b->m_nextInSocket;
			b->deleteInstance();
		}
		m_sockets[i] = NULL;
	}

}  // end freeSockets

/* ------------------------------------------------------------------------ */
inline UnsignedInt calcHashForString(const char* p)
{
	UnsignedInt result = 0; 
	Byte *pp = (Byte*)p;
	while (*pp) 
		result = (result << 5) + result + *pp++; 
	return result;
}

/* ------------------------------------------------------------------------ */
inline UnsignedInt calcHashForLowercaseString(const char* p)
{
	UnsignedInt result = 0; 
	Byte *pp = (Byte*)p;
	while (*pp) 
		result = (result << 5) + result + std::tolower(static_cast<unsigned char>(*pp++));
	return result;
}

//------------------------------------------------------------------------------------------------- 
AsciiString NameKeyGenerator::keyToName(NameKeyType key)
{
	for (Int i = 0; i < SOCKET_COUNT; ++i)
	{
		for (Bucket *b = m_sockets[i]; b; b = b->m_nextInSocket)
		{
			if (key == b->m_key)
				return b->m_nameString;
		}
	}
	return AsciiString::TheEmptyString;
}

//------------------------------------------------------------------------------------------------- 
NameKeyType NameKeyGenerator::nameToKey(const char* nameString)
{
    if(!nameString || !m_nextID)throw ERROR_BAD_ARG;
	Bucket *b;

	UnsignedInt hash = calcHashForString(nameString) % SOCKET_COUNT;

	// hmm, do we have it already?
	for (b = m_sockets[hash]; b; b = b->m_nextInSocket)
	{
		if (strcmp(nameString, b->m_nameString.str()) == 0)
			return b->m_key; 
	}

	// nope, guess not. let's allocate it.
    if(m_nextID>m_capacity)throw ERROR_OUT_OF_MEMORY;
	b = newInstance(Bucket);
    MemoryPoolObjectHolder candidate(b);
	b->m_key = (NameKeyType)m_nextID;
	b->m_nameString = nameString;
	b->m_nextInSocket = m_sockets[hash];
	m_sockets[hash] = b;
    ++m_nextID;candidate.release();

	NameKeyType result = b->m_key;

#if defined(_DEBUG) || defined(_INTERNAL)
	// reality-check to be sure our hasher isn't going bad.
	const Int maxThresh = 3;
	Int numOverThresh = 0;
	for (Int i = 0; i < SOCKET_COUNT; ++i)
	{
		Int numInThisSocket = 0;
		for (b = m_sockets[i]; b; b = b->m_nextInSocket)
			++numInThisSocket;

		if (numInThisSocket > maxThresh)
			++numOverThresh;
	}
	
	// if more than a small percent of the sockets are getting deep, probably want to increase the socket count.
	if (numOverThresh > SOCKET_COUNT/20)
	{
		DEBUG_CRASH(("hmm, might need to increase the number of bucket-sockets for NameKeyGenerator (numOverThresh %d = %f%%)\n",numOverThresh,(Real)numOverThresh/(Real)(SOCKET_COUNT/20)));
	}
#endif

	return result;

}  // end nameToKey

//------------------------------------------------------------------------------------------------- 
NameKeyType NameKeyGenerator::nameToLowercaseKey(const char* nameString)
{
    if(!nameString || !m_nextID)throw ERROR_BAD_ARG;
	Bucket *b;

	UnsignedInt hash = calcHashForLowercaseString(nameString) % SOCKET_COUNT;

	// hmm, do we have it already?
	for (b = m_sockets[hash]; b; b = b->m_nextInSocket)
	{
		if (strcasecmp(nameString, b->m_nameString.str()) == 0)
			return b->m_key; 
	}

	// nope, guess not. let's allocate it.
    if(m_nextID>m_capacity)throw ERROR_OUT_OF_MEMORY;
	b = newInstance(Bucket);
    MemoryPoolObjectHolder candidate(b);
	b->m_key = (NameKeyType)m_nextID;
	b->m_nameString = nameString;
	b->m_nextInSocket = m_sockets[hash];
	m_sockets[hash] = b;
    ++m_nextID;candidate.release();

	NameKeyType result = b->m_key;

#if defined(_DEBUG) || defined(_INTERNAL)
	// reality-check to be sure our hasher isn't going bad.
	const Int maxThresh = 3;
	Int numOverThresh = 0;
	for (Int i = 0; i < SOCKET_COUNT; ++i)
	{
		Int numInThisSocket = 0;
		for (b = m_sockets[i]; b; b = b->m_nextInSocket)
			++numInThisSocket;

		if (numInThisSocket > maxThresh)
			++numOverThresh;
	}
	
	// if more than a small percent of the sockets are getting deep, probably want to increase the socket count.
	if (numOverThresh > SOCKET_COUNT/20)
	{
		DEBUG_CRASH(("hmm, might need to increase the number of bucket-sockets for NameKeyGenerator (numOverThresh %d = %f%%)\n",numOverThresh,(Real)numOverThresh/(Real)(SOCKET_COUNT/20)));
	}
#endif

	return result;

}  // end nameToLowercaseKey

//------------------------------------------------------------------------------------------------- 
// Get a string out of the INI. Store it into a NameKeyType
//------------------------------------------------------------------------------------------------- 
void NameKeyGenerator::parseStringAsNameKeyType( INI *ini, void *instance, void *store, const void* userData )
{
  *(NameKeyType *)store = TheNameKeyGenerator->nameToKey( ini->getNextToken() );
}


//------------------------------------------------------------------------------------------------- 
NameKeyType StaticNameKey::key() const
{
    if(!TheNameKeyGenerator){m_key=NAMEKEY_INVALID;m_generation=0;return NAMEKEY_INVALID;}
    const auto generation=TheNameKeyGenerator->getNamespaceGeneration();
    if(m_key==NAMEKEY_INVALID || m_generation!=generation) {
        const auto candidate=TheNameKeyGenerator->nameToKey(m_name);
        m_key=candidate;m_generation=generation;
    }
    return m_key;
}
