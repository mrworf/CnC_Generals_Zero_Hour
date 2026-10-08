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

// FILE: ThingFactory.cpp /////////////////////////////////////////////////////////////////////////
// Created:   Colin Day, April 2001
// Desc:		This is how we go and make our things, we make our things, we make our things!	
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/FileSystem.h"
#include "Common/GameAudio.h"
#include "Common/MapObject.h"
#include "Common/ModuleFactory.h"
#include "Common/RandomValue.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/CreateModule.h"
#include "Common/ProductionPrerequisite.h"
#include "GameClient/GameClient.h"
#include "GameClient/Drawable.h"
#include "Common/INI.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

enum { TEMPLATE_HASH_SIZE = 12288 };

// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
ThingFactory *TheThingFactory = NULL;  ///< Thing manager singleton declaration

// STATIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
/** Free all data loaded into this template database */
//-------------------------------------------------------------------------------------------------
void ThingFactory::freeDatabase( void )
{
	while (m_firstTemplate)
	{
		ThingTemplate* tmpl = m_firstTemplate;
		m_firstTemplate = m_firstTemplate->friend_getNextTemplate();
		tmpl->deleteInstance();
	}

	m_templateHashMap.clear();

}  // end freeDatabase

//-------------------------------------------------------------------------------------------------
/** add the thing template passed in, into the databse */
//-------------------------------------------------------------------------------------------------
void ThingFactory::addTemplate( ThingTemplate *tmplate )
{
	if (!tmplate) throw ERROR_BAD_ARG;
	// Prepare the fallible index node before publishing its owning list link.
	const auto admitted = m_templateHashMap.emplace(tmplate->getName(), tmplate);
	if (!admitted.second) throw ERROR_BAD_INI;

	// Link it to the list
	tmplate->friend_setNextTemplate(m_firstTemplate);
	m_firstTemplate = tmplate;

}  // end addTemplate

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ThingFactory::ThingFactory()
{
	m_firstTemplate = NULL;
	m_nextTemplateID = 1;	// not zero!

	m_templateHashMap.rehash( TEMPLATE_HASH_SIZE );
}  // end ThingFactory

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ThingFactory::~ThingFactory()
{

	// free all the template data
	freeDatabase();

}  // end ~ThingFactory

//-------------------------------------------------------------------------------------------------
/** Create a new template with name 'name' and add to our template list */
//-------------------------------------------------------------------------------------------------
ThingTemplate *ThingFactory::newTemplate( const AsciiString& name )
{
	if (m_nextTemplateID == 0 || name.isEmpty()) throw ERROR_BAD_ARG;
	ThingTemplate *newTemplate;

	// allocate template
	newTemplate = newInstance(ThingTemplate);
	MemoryPoolObjectHolder candidateOwner(newTemplate);

	// if the default template is present, get it and copy over any data to the new template
	const ThingTemplate *defaultT = findTemplate( AsciiString( "DefaultThingTemplate" ), FALSE );
	if( defaultT )
	{

		// copy over static data
		newTemplate->copyFrom(defaultT);
		newTemplate->setCopiedFromDefault();

	}  // end if

	// give template a unique identifier
	newTemplate->friend_setTemplateID(m_nextTemplateID);

	// assign name
	newTemplate->friend_setTemplateName( name );

	// add to list
	addTemplate( newTemplate );
	// Zero is the exhausted next-ID sentinel, never an admitted template ID.
	m_nextTemplateID = static_cast<UnsignedShort>(UnsignedInt(m_nextTemplateID) + 1);
	candidateOwner.release();

	// return the newly created template
	return newTemplate;

}

//-------------------------------------------------------------------------------------------------
/** Create newTemplate, copy data from final override of 'thingTemplate' to the newly created one,
	* and add newTemplate as the m_override of that final override.  NOTE that newTemplate
	* is *NOT* added to master template list, it is a hidden place to store
	* override values for 'thingTemplate' */
//-------------------------------------------------------------------------------------------------
ThingTemplate* ThingFactory::newOverride( ThingTemplate *thingTemplate )
{
	// Compare borrowed raw identities before dereferencing the proposed parent.
	Bool owned = FALSE;
	for (const auto& entry : m_templateHashMap)
		if (entry.second == thingTemplate) { owned = TRUE; break; }
	if (!owned || !thingTemplate) throw ERROR_BAD_ARG;

	// sanity
	DEBUG_ASSERTCRASH( thingTemplate, ("newOverride(): NULL 'parent' thing template\n") );

	// sanity just for debuging, the weapon must be in the master list to do overrides
	DEBUG_ASSERTCRASH( findTemplate( thingTemplate->getName() ) != NULL,
										 ("newOverride(): Thing template '%s' not in master list\n", 
										 thingTemplate->getName().str()) );

	// find final override of the 'parent' template
	ThingTemplate *child = (ThingTemplate*) thingTemplate->friend_getFinalOverride();

	// allocate new template
	ThingTemplate *newTemplate = newInstance(ThingTemplate);
	MemoryPoolObjectHolder candidateOwner(newTemplate);

	// copy data from final override to 'newTemplate' as a set of initial default values
	newTemplate->copyFrom(child);
	newTemplate->friend_setTemplateName(child->getName());
	newTemplate->friend_setTemplateID(child->getTemplateID());
	newTemplate->friend_setNextTemplate(child->friend_getNextTemplate());
	newTemplate->setCopiedFromDefault();

	newTemplate->markAsOverride();
	child->setNextOverride(newTemplate);
	candidateOwner.release();

	// return the newly created override for us to set values with etc
	return newTemplate;

}  // end newOverride

//-------------------------------------------------------------------------------------------------
/** Init */
//-------------------------------------------------------------------------------------------------
void ThingFactory::init( void )
{

}  // end init

//-------------------------------------------------------------------------------------------------
/** Reset */
//-------------------------------------------------------------------------------------------------
void ThingFactory::reset( void )
{
	ThingTemplate* previous = nullptr;
	for (ThingTemplate* current = m_firstTemplate; current; )
	{
		ThingTemplate* next = current->friend_getNextTemplate();
		if (current->friend_isOverride())
		{
			// Map-only roots may appear anywhere in the list. Withdraw both
			// ownership links before deleting; a retained predecessor must not
			// continue pointing at a retired non-head root.
			m_templateHashMap.erase(current->getName());
			if (previous) previous->friend_setNextTemplate(next);
			else m_firstTemplate = next;
			current->deleteInstance();
		}
		else
		{
			current->deleteOverrides();
			previous = current;
		}
		current = next;
	}
}  // end reset

//-------------------------------------------------------------------------------------------------
/** Update */
//-------------------------------------------------------------------------------------------------
void ThingFactory::update( void )
{

}  // end update

//-------------------------------------------------------------------------------------------------
/** Return the template with the matching database name */
//-------------------------------------------------------------------------------------------------
const ThingTemplate *ThingFactory::findByTemplateID( UnsignedShort id )
{
	for (ThingTemplate *tmpl = m_firstTemplate; tmpl; tmpl = tmpl->friend_getNextTemplate())
	{
		if (tmpl->getTemplateID() == id)
			return tmpl;
	}
	DEBUG_CRASH(("template %d not found\n",(Int)id));
	return NULL;
}

//-------------------------------------------------------------------------------------------------
/** Return the template with the matching database name */
//-------------------------------------------------------------------------------------------------
ThingTemplate *ThingFactory::findTemplateInternal( const AsciiString& name, Bool check )
{
	ThingTemplateHashMapIt tIt = m_templateHashMap.find(name);

	if (tIt != m_templateHashMap.end()) {
		return tIt->second;
	}

#ifdef LOAD_TEST_ASSETS
	if (!strncmp(name.str(), TEST_STRING, strlen(TEST_STRING))) 
	{
		ThingTemplate *tmplate = newTemplate( AsciiString( "Un-namedTemplate" ) );

		// load the values
		tmplate->initForLTA( name );

		// Kinda lame, but necessary.
		m_templateHashMap.erase("Un-namedTemplate");
		m_templateHashMap[name] = tmplate;

		// add tmplate template to the database
		return findTemplateInternal( name );

	}
	
#endif
	
	if( check && name.isNotEmpty() )
	{
		DEBUG_CRASH( ("Failed to find thing template %s (case sensitive) This issue has a chance of crashing after you ignore it!", name.str() ) );
	}
	return NULL;

}  // end getTemplate

//=============================================================================
Object *ThingFactory::newObject( const ThingTemplate *tmplate, Team *team, ObjectStatusMaskType statusBits )
{
	if (tmplate == NULL)
		throw ERROR_BAD_ARG;

	const std::vector<AsciiString>& asv = tmplate->getBuildVariations();
	if (!asv.empty())
	{
		Int which = GameLogicRandomValue(0, asv.size()-1);
		const ThingTemplate* tmp = findTemplate( asv[which] );
		if (tmp != NULL)
			tmplate = tmp;
	}

	DEBUG_ASSERTCRASH(!tmplate->isKindOf(KINDOF_DRAWABLE_ONLY), ("You may not create Objects with the template %s, only Drawables\n",tmplate->getName().str()));

	// have the game logic create an object of the correct type.
	// (this will throw an exception on failure.)
	//Added ability to pass in optional statusBits. This is needed to be set prior to
	//the onCreate() calls... in the case of constructing.
	Object *obj = TheGameLogic->friend_createObject( tmplate, statusBits, team );

	// run the create function for the thing
	for (BehaviorModule** m = obj->getBehaviorModules(); *m; ++m)
	{
		CreateModuleInterface* create = (*m)->getCreate();
		if (!create)
			continue;
	
		create->onCreate();
	}

	//
	// all objects are part of the partition manager system, add it to that 
	// system now
	//
	ThePartitionManager->registerObject( obj );

	obj->initObject();

	return obj;

} 

//=============================================================================
Drawable *ThingFactory::newDrawable(const ThingTemplate *tmplate, DrawableStatus statusBits)
{
	if (tmplate == NULL)
		throw ERROR_BAD_ARG;

	Drawable *draw = TheGameClient->friend_createDrawable( tmplate, statusBits );

	/** @todo we should keep track of all the drawables we've allocated here
	but we'll wait until we have an drawable storage to do that cause it will
	all be tied together */

	return draw;

}  // end newDrawableByType

#if defined(_DEBUG) || defined(_INTERNAL)
AsciiString TheThingTemplateBeingParsedName;
#endif

//-------------------------------------------------------------------------------------------------
/** Parse Object entry */
//-------------------------------------------------------------------------------------------------
/*static*/ void ThingFactory::parseObjectDefinition( INI* ini, const AsciiString& name, const AsciiString& reskinFrom )
{
    if (!ini || !TheThingFactory || !TheModuleFactory || !TheNameKeyGenerator || name.isEmpty())
        throw ERROR_BAD_ARG;
    ThingFactory& factory = *TheThingFactory;
    NameKeyTransaction keys(*TheNameKeyGenerator);
    NativeModuleDataTransaction moduleData(TheModuleFactory->m_moduleDataList);
#if defined(_DEBUG) || defined(_INTERNAL)
    struct ParseNameGuard {
        AsciiString previous;
        ParseNameGuard() : previous(TheThingTemplateBeingParsedName) { }
        ~ParseNameGuard() noexcept { TheThingTemplateBeingParsedName.swap(previous); }
    } parseName;
    TheThingTemplateBeingParsedName = name;
#endif
    ThingTemplate* parent = factory.findTemplateInternal(name, FALSE);
    if (parent && ini->getLoadType() != INI_LOAD_CREATE_OVERRIDES)
        throw ERROR_BAD_INI; // Original duplicate-definition diagnostic, without partial overwrite.
    ThingTemplate* last = parent ? static_cast<ThingTemplate*>(parent->friend_getFinalOverride()) : nullptr;
    ThingTemplate* candidate = nullptr;
    struct Publication {
        ThingTemplate*& head;
        UnsignedShort& nextID;
        ThingTemplateHashMap& index;
        ThingTemplateHashMap previousIndex;
        ThingTemplate* previousHead;
        UnsignedShort previousID;
        ThingTemplate* overrideParent;
        ThingTemplate*& candidate;
        std::vector<std::pair<ThingTemplate*, Bool>> flags;
        bool committed = false;
        Publication(ThingTemplate*& h, UnsignedShort& id, ThingTemplateHashMap& map,
                    ThingTemplate* parent, ThingTemplate*& value)
            : head(h), nextID(id), index(map), previousIndex(map), previousHead(h),
              previousID(id), overrideParent(parent), candidate(value) {
            // Complete all fallible journal preparation before changing the live index.
            for (ThingTemplate* root = h; root; root = root->friend_getNextTemplate())
                for (ThingTemplate* node = root; node;
                     node = static_cast<ThingTemplate*>(node->friend_getNextOverride()))
                    flags.emplace_back(node, node->isBuildFacility());
            index.swap(previousIndex);
        }
        ~Publication() noexcept {
            if (!committed) {
                if (overrideParent) overrideParent->setNextOverride(nullptr);
                head = previousHead;
                nextID = previousID;
                index.swap(previousIndex);
                for (const auto& entry : flags) entry.first->friend_setBuildFacility(entry.second);
                if (candidate) candidate->deleteInstance();
            }
        }
    } publication(factory.m_firstTemplate, factory.m_nextTemplateID,
                  factory.m_templateHashMap, last, candidate);
    // Scoped visibility preserves original self/prerequisite lookup order. The
    // publication journal restores every link/backing before candidate retirement.
    candidate = parent ? factory.newOverride(parent) : factory.newTemplate(name);
    if (!parent && ini->getLoadType() == INI_LOAD_CREATE_OVERRIDES)
        candidate->markAsOverride();
    if (reskinFrom.isNotEmpty()) {
        const ThingTemplate* source = factory.findTemplate(reskinFrom);
        if (!source) throw INI_INVALID_DATA;
        candidate->copyFrom(source);
        candidate->setCopiedFromDefault();
        candidate->setReskinnedFrom(source);
        ini->initFromINI(candidate, candidate->getReskinFieldParse());
    } else {
        ini->initFromINI(candidate, candidate->getFieldParse());
    }
    candidate->validate();
    if (ini->getLoadType() == INI_LOAD_CREATE_OVERRIDES)
        candidate->resolveNames();
    // No fallible work remains after these publication decisions.
    publication.committed = true;
    moduleData.commit();
    keys.commit();
}

//#define CHECK_THING_NAMES
#ifdef CHECK_THING_NAMES

#include "Common/STLTypedefs.h"

const char *outFilenameINI				= "thing.txt";
const char *outFilenameStringFile	= "thingString.txt";

void resetReportFile( void )
{
	FILE *fp = fopen(outFilenameINI, "w");
	if (fp)
	{
		fprintf(fp, "-- ThingTemplate INI Report --\n\n");
		fclose(fp);
	}

	fp = fopen(outFilenameStringFile, "w");
	if (fp)
	{
		fprintf(fp, "-- ThingTemplate String File Report --\n\n");
		fclose(fp);
	}
}

AsciiStringList missingStrings;
void reportMissingNameInStringFile( AsciiString templateName )
{
	// see if we've seen it before
	AsciiStringListConstIterator cit = std::find(missingStrings.begin(), missingStrings.end(), templateName);
	if (cit != missingStrings.end())
		return;

	missingStrings.push_back(templateName);
}

void dumpMissingStringNames( void )
{
	missingStrings.sort();
	FILE *fp = fopen(outFilenameStringFile, "w");
	if (fp)
	{
		fprintf(fp, "-- ThingTemplate String File Report --\n\n");
		for (AsciiStringListConstIterator cit = missingStrings.begin(); cit!=missingStrings.end(); cit++)
		{
			fprintf(fp, "OBJECT:%s\n\"%s\"\nEND\n\n", cit->str(), cit->str());
		}
		fclose(fp);
	}
}

AsciiStringList missingNames;
void reportMissingNameInTemplate( AsciiString templateName )
{
	// see if we've seen it before
	AsciiStringListConstIterator cit = std::find(missingNames.begin(), missingNames.end(), templateName);
	if (cit != missingNames.end())
		return;

	missingNames.push_back(templateName);

	FILE *fp = fopen(outFilenameINI, "a+");
	if (fp)
	{
		fprintf(fp, "  DisplayName      = OBJECT:%s\n", templateName.str());
		fclose(fp);
	}

	//reportMissingNameInStringFile( templateName );
}

#endif

//-------------------------------------------------------------------------------------------------
/** Post process phase after loading the database files */
//-------------------------------------------------------------------------------------------------
void ThingFactory::postProcessLoad()
{
#ifdef CHECK_THING_NAMES
	//resetReportFile();
#endif

	// go through all thing templates
	for( ThingTemplate *thingTemplate = m_firstTemplate; 
			 thingTemplate; 
			 thingTemplate = thingTemplate->friend_getNextTemplate() )
	{

		// resolve the prerequisite names
		thingTemplate->resolveNames();

#ifdef CHECK_THING_NAMES
		if (thingTemplate->getDisplayName().isEmpty())
		{
			reportMissingNameInTemplate( thingTemplate->getName() );
		}
		else if (wcsstr(thingTemplate->getDisplayName().str(), L"MISSING:"))
		{
			AsciiString asciiName;
			asciiName.translate(thingTemplate->getDisplayName());
			asciiName.removeLastChar();
			asciiName = asciiName.str() + 17;
			reportMissingNameInStringFile( asciiName );
		}
#endif

	}  // end for 

#ifdef CHECK_THING_NAMES
	dumpMissingStringNames();
	exit(0);
#endif
}  // end postProcess
