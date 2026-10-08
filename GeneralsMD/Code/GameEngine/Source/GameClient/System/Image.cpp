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

// FILE: Image.cpp ////////////////////////////////////////////////////////////////////////////////
// Created:   Colin Day, June 2001
// Desc:      High level representation of images, this is currently being
//						written so we have a way to refer to images in the windows
//						GUI, this system should be replaced with something that can
//						handle real image management or written to accomodate 
//						all parts of the engine that need images.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#define DEFINE_IMAGE_STATUS_NAMES
#include "Lib/BaseType.h"
#include "Common/Debug.h"
#include "Common/INI.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "GameClient/Image.h"
#include "Common/NameKeyGenerator.h"
#include <cstdint>
#include <limits>
#include <utility>

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
const FieldParse Image::m_imageFieldParseTable[] = 
{

	{ "Texture",				INI::parseAsciiString,							NULL, 		offsetof( Image, m_filename ) },
	{ "TextureWidth",		INI::parseInt,											NULL, 		offsetof( Image, m_textureSize.x ) },
	{ "TextureHeight",	INI::parseInt,											NULL, 		offsetof( Image, m_textureSize.y ) },
	{ "Coords",					Image::parseImageCoords,						NULL, 		offsetof( Image, m_UVCoords ) },
	{ "Status",					Image::parseImageStatus,						NULL, 		offsetof( Image, m_status ) },

	{ NULL,							NULL,																NULL, 		0 }

};

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------
/** Parse an image coordinates in the form of
	*
	* COORDS = Left:AAA Top:BBB Right:CCC Bottom:DDD */
//-------------------------------------------------------------------------------------------------
void Image::parseImageCoords( INI* ini, void *instance, void *store, const void* /*userData*/ )
{
	Int left = INI::scanInt(ini->getNextSubToken("Left"));
	Int top = INI::scanInt(ini->getNextSubToken("Top"));
	Int right = INI::scanInt(ini->getNextSubToken("Right"));
	Int bottom = INI::scanInt(ini->getNextSubToken("Bottom"));
    const auto width=std::int64_t(right)-std::int64_t(left);
    const auto height=std::int64_t(bottom)-std::int64_t(top);
    if (width<std::numeric_limits<Int>::min() || width>std::numeric_limits<Int>::max() ||
        height<std::numeric_limits<Int>::min() || height>std::numeric_limits<Int>::max())
        throw ERROR_BAD_INI;

	// get the image we're storing in
	Image *theImage = (Image *)instance;

	//
	// store the UV coords based on what we've read in and the texture size
	// defined for this image
	//
	Region2D uvCoords;

	uvCoords.lo.x = (Real)left;
	uvCoords.lo.y = (Real)top;
	uvCoords.hi.x = (Real)right;
	uvCoords.hi.y = (Real)bottom;
	
	// adjust the coords by texture size
	const ICoord2D *textureSize = theImage->getTextureSize();
	if( textureSize->x )
	{
		uvCoords.lo.x /= (Real)textureSize->x;
		uvCoords.hi.x /= (Real)textureSize->x;
	}  // end if
	if( textureSize->y )
	{
		uvCoords.lo.y /= (Real)textureSize->y;
		uvCoords.hi.y /= (Real)textureSize->y;
	}  // end if

	// store the uv coords
	theImage->setUV( &uvCoords );

	// compute the image size based on the coords we read and store
	ICoord2D imageSize;
	imageSize.x = static_cast<Int>(width);
	imageSize.y = static_cast<Int>(height);
	theImage->setImageSize( &imageSize );

}  // end parseImageCoord

//-------------------------------------------------------------------------------------------------
/** Parse the image status line */
//-------------------------------------------------------------------------------------------------
void Image::parseImageStatus( INI* ini, void *instance, void *store, const void* /*userData*/)
{	
	// use existing INI parsing for the bit strings
	INI::parseBitString32(ini, instance, store, imageStatusNames);

	//
	// if we are rotated 90 degrees clockwise we need to swap our width and height as
	// they were computed from the page location rect, which was for the rotated image
	// (see ImagePacker tool for more details)
	//
	UnsignedInt *theStatusBits = (UnsignedInt *)store;
	if( BitTest( *theStatusBits, IMAGE_STATUS_ROTATED_90_CLOCKWISE ) )
	{
		Image *theImage = (Image *)instance;
		ICoord2D imageSize;

		imageSize.x = theImage->getImageHeight();  // note it's height not width
		imageSize.y = theImage->getImageWidth();   // note it's width not height
		theImage->setImageSize( &imageSize );

	}  // end if

}  // end parseImageStatus

// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
ImageCollection *TheMappedImageCollection = NULL;  ///< mapped images

// PUBLIC FUNCTIONS////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Image::Image( void )
{

	m_name.clear();
	m_filename.clear();
	m_textureSize.x = 0;
	m_textureSize.y = 0;
	m_UVCoords.lo.x = 0.0f;
	m_UVCoords.lo.y = 0.0f;
	m_UVCoords.hi.x = 1.0f;
	m_UVCoords.hi.y = 1.0f;
	m_imageSize.x = 0;
	m_imageSize.y = 0;
	m_rawTextureData = NULL;
	m_status = IMAGE_STATUS_NONE;

}  // end Image

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Image::copyDefinition(const Image& source) {
    // Raw texture data is borrowed; the source destructor does not release it.
    m_name=source.m_name; m_filename=source.m_filename;
    m_textureSize=source.m_textureSize; m_UVCoords=source.m_UVCoords;
    m_imageSize=source.m_imageSize; m_rawTextureData=source.m_rawTextureData;
    m_status=source.m_status;
}
void Image::swapDefinition(Image& other) noexcept {
    m_name.swap(other.m_name); m_filename.swap(other.m_filename);
    std::swap(m_textureSize,other.m_textureSize);std::swap(m_UVCoords,other.m_UVCoords);
    std::swap(m_imageSize,other.m_imageSize);std::swap(m_rawTextureData,other.m_rawTextureData);
    std::swap(m_status,other.m_status);
}

Image::~Image( void )
{

}  // end ~Image

//-------------------------------------------------------------------------------------------------
/** Set a status bit into the existing status, return the previous status
	* bit collection from before the set */
//-------------------------------------------------------------------------------------------------
UnsignedInt Image::setStatus( UnsignedInt bit )
{
	UnsignedInt prevStatus = m_status;

	BitSet( m_status, bit );
	return prevStatus;

}  // end setStatus

//-------------------------------------------------------------------------------------------------
/** Clear a status bit from the existing status, return the previous
	* status bit collection from before the clear */
//-------------------------------------------------------------------------------------------------
UnsignedInt Image::clearStatus( UnsignedInt bit )
{
	UnsignedInt prevStatus = m_status;

	BitClear( m_status, bit );
	return prevStatus;

}  // end clearStatus

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ImageCollection::ImageCollection( void )
{
}  // end ImageCollection

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ImageCollection::~ImageCollection( void )
{
  for (std::map<unsigned,Image *>::iterator i=m_imageMap.begin();i!=m_imageMap.end();++i)
    i->second->deleteInstance();
}  // end ~ImageCollection

//-------------------------------------------------------------------------------------------------
/** adds the given image to the collection, transfers ownership to this object */
//-------------------------------------------------------------------------------------------------
void ImageCollection::addImage( Image *image )
{
    if (!image || !TheNameKeyGenerator || image->getName().isEmpty()) throw ERROR_BAD_ARG;
    NameKeyTransaction keys(*TheNameKeyGenerator);
    const auto key=TheNameKeyGenerator->nameToLowercaseKey(image->getName());
    if (!m_imageMap.emplace(key,image).second) throw ERROR_BAD_INI;
    keys.commit();
}  // end newImage

//-------------------------------------------------------------------------------------------------
/** Find an image given the image name */
//-------------------------------------------------------------------------------------------------
const Image *ImageCollection::findImageByName( const AsciiString& name )
{
    if (!TheNameKeyGenerator) throw ERROR_BAD_ARG;
    NameKeyTransaction keys(*TheNameKeyGenerator);
    const auto found=m_imageMap.find(TheNameKeyGenerator->nameToLowercaseKey(name));
    if (found==m_imageMap.end()) return nullptr;
    keys.commit();
    return found->second;
}  // end findImageByName

//-------------------------------------------------------------------------------------------------
/** Load this image collection with all the images specified in the INI files
	* for the proper texture size directory */
//-------------------------------------------------------------------------------------------------
void ImageCollection::load( Int textureSize )
{
    if (!TheFileSystem || !TheNameKeyGenerator || TheMappedImageCollection!=this) throw ERROR_BAD_ARG;
    NameKeyTransaction keys(*TheNameKeyGenerator);
    ImageCollection candidate;
    for (const auto& entry:m_imageMap) {
        if (!entry.second) throw ERROR_BAD_ARG;
        Image* clone=newInstance(Image);
        MemoryPoolObjectHolder owner(clone);
        clone->copyDefinition(*entry.second);
        if (!candidate.m_imageMap.emplace(entry.first,clone).second) throw ERROR_BAD_ARG;
        owner.release();
    }
    struct Publication {
        ImageCollection* previous;
        explicit Publication(ImageCollection& value):previous(TheMappedImageCollection) {
            TheMappedImageCollection=&value;
        }
        ~Publication() noexcept {TheMappedImageCollection=previous;}
    } publication(candidate);
    const INIBlockDefinition blocks[]{{"MappedImage",INI::parseMappedImageDefinition}};
    INI ini;
    // Preserve the source's top-level user-INI gate and recursive load order.
    if (TheGlobalData) {
        AsciiString directory;
        directory.format("%sINI\\MappedImages",TheGlobalData->getPath_UserData().str());
        FilenameList present;
        TheFileSystem->getFileListInDirectory(directory,"*.ini",present,FALSE);
        if (!present.empty()) ini.loadDirectoryBlocks(directory,TRUE,INI_LOAD_OVERWRITE,blocks);
    }
    AsciiString directory;
    directory.format("Data\\INI\\MappedImages\\TextureSize_%d",textureSize);
    ini.loadDirectoryBlocks(directory,TRUE,INI_LOAD_OVERWRITE,blocks);
    ini.loadDirectoryBlocks("Data\\INI\\MappedImages\\HandCreated",TRUE,INI_LOAD_OVERWRITE,blocks);
    // Validate the entire commit ledger before any accepted payload changes.
    for (const auto& entry:m_imageMap) {
        const auto prepared=candidate.m_imageMap.find(entry.first);
        if (prepared==candidate.m_imageMap.end() || !prepared->second) throw ERROR_BAD_ARG;
    }
    // Keep every accepted Image address stable. Candidate clones retire the old
    // payloads/index only after this allocation-free complete publication.
    for (auto& entry:m_imageMap) {
        auto prepared=candidate.m_imageMap.find(entry.first);
        entry.second->swapDefinition(*prepared->second);
        std::swap(entry.second,prepared->second);
    }
    m_imageMap.swap(candidate.m_imageMap);
    keys.commit();
}  // end load
