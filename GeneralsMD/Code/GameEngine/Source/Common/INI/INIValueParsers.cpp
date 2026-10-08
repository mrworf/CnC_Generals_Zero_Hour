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

// Original value parsers separated from complete gameplay/presentation registry.
#include "PreRTS.h"
#include "Common/INI.h"
#include "GameClient/Color.h"
#include <cmath>
#include <limits>
#include <strings.h>
void INI::parseAngleReal( INI *ini, void * /*instance*/, 
																			void *store, const void *userData )
{
	const char *token = ini->getNextToken();

	const Real RADS_PER_DEGREE = PI / 180.0f;
	*(Real *)store = scanReal( token ) * RADS_PER_DEGREE;

}

void INI::parseAngularVelocityReal( INI *ini, void * /*instance*/, 
																			void *store, const void *userData )
{
	const char *token = ini->getNextToken();

	// scan the int and convert to radian and store as a real
	*(Real *)store = ConvertAngularVelocityInDegreesPerSecToRadsPerFrame(scanReal( token ));

}

void INI::parseAsciiStringVector( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	std::vector<AsciiString>* asv = (std::vector<AsciiString>*)store;
	asv->clear();
	for (const char *token = ini->getNextTokenOrNull(); token != NULL; token = ini->getNextTokenOrNull())
	{
		asv->push_back(token);
	}
}

void INI::parseAsciiStringVectorAppend( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	std::vector<AsciiString>* asv = (std::vector<AsciiString>*)store;
	// nope, don't clear. duh.
	// asv->clear();
	for (const char *token = ini->getNextTokenOrNull(); token != NULL; token = ini->getNextTokenOrNull())
	{
		asv->push_back(token);
	}
}

void INI::parsePercentToReal( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken(ini->getSepsPercent());
	Real *theReal = (Real *)store;
	*theReal = scanPercentToReal(token);

}

void INI::parseBitString8( INI* ini, void * /*instance*/, void *store, const void* userData )
{
	if (!ini || !store) throw ERROR_BAD_ARG;
	// Byte is plain char in the original protocol. Read/write its raw eight
	// bits without sign extending a high-bit prior mask into the 32-bit parser.
	UnsignedInt tmp = *static_cast<UnsignedByte*>(store);
	INI::parseBitString32(ini, NULL, &tmp, userData);
	if (tmp & 0xffffff00)
	{
		DEBUG_CRASH(("Bad bitstring list INI::parseBitString8"));
		throw ERROR_BUG;
	}
	*static_cast<UnsignedByte*>(store) = static_cast<UnsignedByte>(tmp);
}

void INI::parseBitString32( INI* ini, void * /*instance*/, void *store, const void* userData )
{
	ConstCharPtrArray flagList = (ConstCharPtrArray)userData;
	UnsignedInt *bits = (UnsignedInt *)store;
	if (!ini || !bits) throw ERROR_BAD_ARG;
	UnsignedInt candidate = *bits;

	if( flagList == NULL || flagList[ 0 ] == NULL)
	{
		DEBUG_ASSERTCRASH( flagList, ("INTERNAL ERROR! parseBitString32: No flag list provided!\n") );
		throw INI_INVALID_NAME_LIST;
	}

	Bool foundNormal = false;
	Bool foundAddOrSub = false;

	// loop through all tokens
	for (const char *token = ini->getNextTokenOrNull(); token != NULL; token = ini->getNextTokenOrNull())
	{
		if (strcasecmp(token, "NONE") == 0)
		{
			if (foundNormal || foundAddOrSub)
			{
				DEBUG_CRASH(("you may not mix normal and +- ops in bitstring lists"));
				throw INI_INVALID_NAME_LIST;
			}
			candidate = 0;
			break;
		}

		if (token[0] == '+')
		{
			if (foundNormal)
			{
				DEBUG_CRASH(("you may not mix normal and +- ops in bitstring lists"));
				throw INI_INVALID_NAME_LIST;
			}
			Int bitIndex = INI::scanIndexList(token+1, flagList);	// this throws if the token is not found
			if (bitIndex < 0 || bitIndex >= 32) throw ERROR_BAD_INI;
			candidate |= (UnsignedInt{1} << bitIndex);
			foundAddOrSub = true;
		}
		else if (token[0] == '-')
		{
			if (foundNormal)
			{
				DEBUG_CRASH(("you may not mix normal and +- ops in bitstring lists"));
				throw INI_INVALID_NAME_LIST;
			}
			Int bitIndex = INI::scanIndexList(token+1, flagList);	// this throws if the token is not found
			if (bitIndex < 0 || bitIndex >= 32) throw ERROR_BAD_INI;
			candidate &= ~(UnsignedInt{1} << bitIndex);
			foundAddOrSub = true;
		}
		else
		{
			if (foundAddOrSub)
			{
				DEBUG_CRASH(("you may not mix normal and +- ops in bitstring lists"));
				throw INI_INVALID_NAME_LIST;
			}

			if (!foundNormal)
				candidate = 0;

			Int bitIndex = INI::scanIndexList(token, flagList);	// this throws if the token is not found
			if (bitIndex < 0 || bitIndex >= 32) throw ERROR_BAD_INI;
			candidate |= (UnsignedInt{1} << bitIndex);
			foundNormal = true;
		}
	}
	*bits = candidate; // Only a complete, bounded bit expression publishes.

}

void INI::parseRGBColor( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char* names[3] = { "R", "G", "B" };
	Int colors[3];
	for( Int i = 0; i < 3; i++ )
	{
		colors[i] = scanInt(ini->getNextSubToken(names[i]));
		if( colors[ i ] < 0 )
			throw INI_INVALID_DATA;
		if( colors[ i ] > 255 )
			throw INI_INVALID_DATA;
	}

	// assign the color components to the "RGBColor" pointer at 'store'
	RGBColor *theColor = (RGBColor *)store;
	theColor->red		= (Real)colors[ 0 ] / 255.0f;
	theColor->green = (Real)colors[ 1 ] / 255.0f;
	theColor->blue	= (Real)colors[ 2 ] / 255.0f;

}

void INI::parseRGBAColorInt( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char* names[4] = { "R", "G", "B", "A" };
	Int colors[4];
	for( Int i = 0; i < 4; i++ )
	{
		const char* token = ini->getNextTokenOrNull(ini->getSepsColon());
		if (token == NULL)
		{
			if (i < 3)
			{
				throw INI_INVALID_DATA;
			}
			else
			{
				// it's ok for A to be omitted.
				colors[i] = 255;
			}
		}
		else
		{
			// if present, the token must match.
			if (strcasecmp(token, names[i]) != 0)
			{
				throw INI_INVALID_DATA;				
			}
			colors[i] = scanInt(ini->getNextToken(ini->getSepsColon()));
		}
		if( colors[ i ] < 0 )
			throw INI_INVALID_DATA;
		if( colors[ i ] > 255 )
			throw INI_INVALID_DATA;
	}

	//
	// assign the color components to the "RGBColorInt" pointer at 'store', keep
	// the numbers as between 0 and 255
	//
	RGBAColorInt *theColor = (RGBAColorInt *)store;
	theColor->red		= colors[ 0 ];
	theColor->green = colors[ 1 ];
	theColor->blue	= colors[ 2 ];
	theColor->alpha = colors[ 3 ];

}

void INI::parseColorInt( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char* names[4] = { "R", "G", "B", "A" };
	Int colors[4];
	for( Int i = 0; i < 4; i++ )
	{
		const char* token = ini->getNextTokenOrNull(ini->getSepsColon());
		if (token == NULL)
		{
			if (i < 3)
			{
				throw INI_INVALID_DATA;
			}
			else
			{
				// it's ok for A to be omitted.
				colors[i] = 255;
			}
		}
		else
		{
			// if present, the token must match.
			if (strcasecmp(token, names[i]) != 0)
			{
				throw INI_INVALID_DATA;				
			}
			colors[i] = scanInt(ini->getNextToken(ini->getSepsColon()));
		}
		if( colors[ i ] < 0 )
			throw INI_INVALID_DATA;
		if( colors[ i ] > 255 )
			throw INI_INVALID_DATA;
	}

	//
	// assign the color components to the "Color" pointer at 'store', keep
	// the numbers as between 0 and 255
	//
	Color *theColor = (Color *)store;
	*theColor = GameMakeColor(colors[0], colors[1], colors[2], colors[3]);

}

void INI::parseCoord3D( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	Coord3D *theCoord = (Coord3D *)store;

	theCoord->x = scanReal(ini->getNextSubToken("X"));
	theCoord->y = scanReal(ini->getNextSubToken("Y"));
	theCoord->z = scanReal(ini->getNextSubToken("Z"));

}

void INI::parseCoord2D( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	Coord2D *theCoord = (Coord2D *)store;

	theCoord->x = scanReal(ini->getNextSubToken("X"));
	theCoord->y = scanReal(ini->getNextSubToken("Y"));

}

void INI::parseICoord2D( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	ICoord2D *theCoord = (ICoord2D *)store;

	theCoord->x = scanInt(ini->getNextSubToken("X"));
	theCoord->y = scanInt(ini->getNextSubToken("Y"));

}

void INI::parseIndexList( INI* ini, void * /*instance*/, void *store, const void* userData )
{
	ConstCharPtrArray nameList = (ConstCharPtrArray)userData;
	*(Int *)store = scanIndexList(ini->getNextToken(), nameList);
}

void INI::parseByteSizedIndexList( INI* ini, void * /*instance*/, void *store, const void* userData )
{
	ConstCharPtrArray nameList = (ConstCharPtrArray)userData;
	Int value = scanIndexList(ini->getNextToken(), nameList);
	if (value < 0 || value > 255)
	{
		DEBUG_CRASH(("Bad index list INI::parseByteSizedIndexList"));
		throw ERROR_BUG;
	}
	*(Byte *)store = (Byte)value;
}

void INI::parseLookupList( INI* ini, void * /*instance*/, void *store, const void* userData )
{
	ConstLookupListRecArray lookupList = (ConstLookupListRecArray)userData;
	*(Int *)store = scanLookupList(ini->getNextToken(), lookupList);
}

void INI::parseDurationReal( INI *ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	Real val = scanReal(ini->getNextToken());
	*(Real *)store = ConvertDurationFromMsecsToFrames(val);
}

void INI::parseDurationUnsignedInt( INI *ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	UnsignedInt val = scanUnsignedInt(ini->getNextToken());
	const Real frames = ceilf(ConvertDurationFromMsecsToFrames((Real)val));
	if (!std::isfinite(frames) || frames < 0 || static_cast<double>(frames) > std::numeric_limits<UnsignedInt>::max()) throw ERROR_BAD_INI;
	*(UnsignedInt *)store = static_cast<UnsignedInt>(frames);
}

void INI::parseDurationUnsignedShort( INI *ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	UnsignedInt val = scanUnsignedInt(ini->getNextToken());
	const Real frames = ceilf(ConvertDurationFromMsecsToFrames((Real)val));
	if (!std::isfinite(frames) || frames < 0 || frames > std::numeric_limits<UnsignedShort>::max()) throw ERROR_BAD_INI;
	*(UnsignedShort *)store = static_cast<UnsignedShort>(frames);
}

void INI::parseVelocityReal( INI *ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();
	Real val = scanReal(token);
	*(Real *)store = ConvertVelocityInSecsToFrames(val);
}

void INI::parseAccelerationReal( INI *ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();
	Real val = scanReal(token);
	*(Real *)store = ConvertAccelerationInSecsToFrames(val);
}
