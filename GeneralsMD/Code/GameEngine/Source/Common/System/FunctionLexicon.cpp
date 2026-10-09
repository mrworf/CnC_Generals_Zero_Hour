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

// FILE: FunctionLexicon.cpp //////////////////////////////////////////////////////////////////////
// Created:    Colin Day, September 2001
// Desc:       Collection of function pointers to help us in managing
//						 and assign callbacks
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/FunctionLexicon.h"
#include "GameClient/GameWindow.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC DATA 
///////////////////////////////////////////////////////////////////////////////////////////////////
// Borrowed singleton publication lives in NativeStartupPublications.cpp.

//-------------------------------------------------------------------------------------------------
/** Since we have a convenient table to organize our callbacks anyway,
	* we'll just use this same storage space to load in any run time
	* components we might want to add to the table, such as generating
	* a key based off the name supplied in the table for faster access */
//-------------------------------------------------------------------------------------------------
void FunctionLexicon::loadTable(std::span<TableEntry> table, TableIndex index)
{
	if (!TheNameKeyGenerator) throw ERROR_BAD_ARG;
	const NativeFunctionTable input{table,index};
	m_registry.loadTables(std::span(&input,1),*TheNameKeyGenerator);
}

NativeWindowCallback FunctionLexicon::findFunction(NameKeyType key, TableIndex index)
{
	return m_registry.find(key,index);
}

#ifdef NOT_IN_USE
//-------------------------------------------------------------------------------------------------
/** Search for the function in the specified table */
//-------------------------------------------------------------------------------------------------
const char *FunctionLexicon::funcToName( NativeWindowCallback func, TableEntry *table )
{

	// sanity
	if( !func )
		return NULL;

	// search the table
	TableEntry *entry = table;
	while( entry && entry->key != NAMEKEY_INVALID )
	{

		// is this it
		if( entry->func == func )
			return entry->name;

		// not it, check next
		entry++;

	}  // end while

	return NULL;  // not found

}  // end funcToName
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS 
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FunctionLexicon::FunctionLexicon(std::span<const NativeFunctionTable> tables)
{
  if(tables.size()>m_sources.size()) throw ERROR_BAD_ARG;
  std::copy(tables.begin(),tables.end(),m_sources.begin());
  m_sourceCount=tables.size();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FunctionLexicon::~FunctionLexicon( void )
{

}  // end ~FunctionLexicon

//-------------------------------------------------------------------------------------------------
/** Initialize our dictionary of funtion pointers and symbols */
//-------------------------------------------------------------------------------------------------
void FunctionLexicon::init() { initWithDeviceTables({},{}); }

void FunctionLexicon::initWithDeviceTables(std::span<TableEntry> draw, std::span<TableEntry> layout)
{
	if (!TheNameKeyGenerator) throw ERROR_BAD_ARG;
	if (draw.empty()!=layout.empty()) throw ERROR_BAD_ARG;
  auto inputs=m_sources;
  auto count=m_sourceCount;
  if(!draw.empty()) {
    if(count>inputs.size()-2) throw ERROR_BAD_ARG;
    inputs[count++]={draw,TABLE_GAME_WIN_DEVICEDRAW};
    inputs[count++]={layout,TABLE_WIN_LAYOUT_DEVICEINIT};
  }
	m_registry.loadTables(std::span(inputs.data(),count),*TheNameKeyGenerator);
	validate();
}

//-------------------------------------------------------------------------------------------------
/** reset */
//-------------------------------------------------------------------------------------------------
void FunctionLexicon::reset( void )
{

	//
	// make sure the ordering of what happens here with respect to derived classes resets is
	// all OK since we're cheating and using the init() method
	//

	// nothing dynamically loaded, just reinit the tables
	init();

}  // end reset

//-------------------------------------------------------------------------------------------------
/** Update */ 
//-------------------------------------------------------------------------------------------------
void FunctionLexicon::update( void )
{

}  // end update

/*
// !NOTE! We can not have this function, see the header for
// more information as to why
//
//-------------------------------------------------------------------------------------------------
// translate a function pointer to its symbolic name
//-------------------------------------------------------------------------------------------------
char *FunctionLexicon::functionToName( void *func )
{
	
	// sanity
	if( !func )
		return NULL;

	// search ALL the tables
	Int i;
	char *name = NULL;
	for( i = 0; i < MAX_FUNCTION_TABLES; i++ )
	{

		name = funcToName( func, m_tables[ i ] );
		if( name )
			return name;

	}  // end for i

	return NULL;  // not found

}  // end functionToName
*/

//-------------------------------------------------------------------------------------------------
/** Scan the tables and make sure that each function address is unique.
	* We want to do this to prevent accidental entries of two identical
	* functions and because the compiler will optimize identical functions
	* to the same address (typically in empty functions with no body and the
	* same parameters) which we MUST keep separate for when we add code to
	* them */
//-------------------------------------------------------------------------------------------------
Bool FunctionLexicon::validate( void ) { return m_registry.validate(); }

//============================================================================
// FunctionLexicon::gameWinDrawFunc
//============================================================================

GameWinDrawFunc FunctionLexicon::gameWinDrawFunc( NameKeyType key, TableIndex index )
{ 
	if ( index == TABLE_ANY )
	{
		// first search the device depended table then the device independent table
		GameWinDrawFunc func;

		func = findFunction( key, TABLE_GAME_WIN_DEVICEDRAW ).get<GameWinDrawFunc>();
		if ( func == NULL )
		{
			func = findFunction( key, TABLE_GAME_WIN_DRAW ).get<GameWinDrawFunc>();
		}
		return func;
	}
	// search the specified table
	return findFunction( key, index ).get<GameWinDrawFunc>();
}

WindowLayoutInitFunc FunctionLexicon::winLayoutInitFunc( NameKeyType key, TableIndex index )
{
	if ( index == TABLE_ANY )
	{
		// first search the device depended table then the device independent table
		WindowLayoutInitFunc func;

		func = findFunction( key, TABLE_WIN_LAYOUT_DEVICEINIT ).get<WindowLayoutInitFunc>();
		if ( func == NULL )
		{
			func = findFunction( key, TABLE_WIN_LAYOUT_INIT ).get<WindowLayoutInitFunc>();
		}
		return func;
	}
	// search the specified table
	return findFunction( key, index ).get<WindowLayoutInitFunc>();
}
