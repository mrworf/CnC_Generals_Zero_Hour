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

//----------------------------------------------------------------------------=
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information					                  
//                Copyright(C) 2001 - All Rights Reserved                  
//                                                                          
//----------------------------------------------------------------------------
//
// Project:    GameEngine
//
// Module:     IO
//
// File name:  FileSystem.h
//
// Created:    
//
//----------------------------------------------------------------------------

#pragma once

#ifndef __FILESYSTEM_H
#define __FILESYSTEM_H

//----------------------------------------------------------------------------
//           Includes                                                      
//----------------------------------------------------------------------------

//#include "Common/File.h"
#include "Common/FilenameList.h"
#include "Common/SubsystemInterface.h"

//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------
class File;
class NativeUserStorage;

//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------
//#define W3D_DIR_PATH "../FinalArt/W3D/"					///< .w3d files live here
//#define TGA_DIR_PATH "../FinalArt/Textures/"		///< .tga texture files live here
//#define TERRAIN_TGA_DIR_PATH "../FinalArt/Terrain/"		///< terrain .tga texture files live here
#define W3D_DIR_PATH "Art/W3D/"					///< .w3d files live here
#define TGA_DIR_PATH "Art/Textures/"		///< .tga texture files live here
#define TERRAIN_TGA_DIR_PATH "Art/Terrain/"		///< terrain .tga texture files live here
#define MAP_PREVIEW_DIR_PATH "%sMapPreviews/"	///< We need a common place we can copy the map previews to at runtime.
#define USER_W3D_DIR_PATH "%sW3D/"					///< .w3d files live here
#define USER_TGA_DIR_PATH "%sTextures/"		///< User .tga texture files live here

// the following defines are only to be used while maintaining legacy compatability
// with old files until they are completely gone and in the regular art set
#ifdef MAINTAIN_LEGACY_FILES
#define LEGACY_W3D_DIR_PATH "../LegacyArt/W3D/"				///< .w3d files live here
#define LEGACY_TGA_DIR_PATH "../LegacyArt/Textures/"	///< .tga texture files live here
#endif  // MAINTAIN_LEGACY_FILES

// The release Linux runtime does not auto-discover development TestArt outside
// its configured asset roots. Legacy authoring-only guards remain dormant; no
// runtime build defines LOAD_TEST_ASSETS or LOOK_FOR_TEST_ART.

struct FileInfo {
	Int sizeHigh;
	Int sizeLow;
	Int timestampHigh;
	Int timestampLow;
};

//===============================
// FileSystem
//===============================
/**
  * FileSystem is an interface class for creating specific FileSystem objects.
  * 
	* A FileSystem object's implemenation decides what derivative of File object needs to be 
	* created when FileSystem::Open() gets called.
	*/
//===============================
#include <map>

class FileSystem : public SubsystemInterface
{
  FileSystem(const FileSystem&);
  FileSystem& operator=(const FileSystem&);
  
public:
	FileSystem();
	virtual	~FileSystem();
	// Explicit ordered roots: Zero Hour first, then original Generals assets.
	// Replace mounts only after a complete offside read-only index is admitted.
	void mountReadOnly(const std::vector<std::string>& roots);
  // Original mod semantics: archive overrides only, explicit BIG before sorted
  // recursive directory BIGs. Complete admission preserves attached storage.
  void mountReadOnlyMods(const std::string& bigFile, const std::string& directory);
	// Caller resolves the complete prospective physical path before write admission.
	bool admitsUserStorage(const std::string& canonicalPath) const;
  // Borrowed storage outlives this attachment. Asset-relative names keep their
  // immutable namespace; only explicit configured user-root paths use it.
  void attachUserStorage(const NativeUserStorage* storage);

	void init();
	void reset();
	void update();

	File* openFile( const Char *filename, Int access = 0 );		///< opens a File interface to the specified file
	Bool doesFileExist(const Char *filename) const;								///< returns TRUE if the file exists.  filename should have no directory.
	void getFileListInDirectory(const AsciiString& directory, const AsciiString& searchName, FilenameList &filenameList, Bool searchSubdirectories) const; ///< search the given directory for files matching the searchName (egs. *.ini, *.rep).  Possibly search subdirectories.
	Bool getFileInfo(const AsciiString& filename, FileInfo *fileInfo) const; ///< fills in the FileInfo struct for the file given. returns TRUE if successful.

	Bool createDirectory(AsciiString directory); ///< create a directory of the given name.

	Bool areMusicFilesOnCD();
	void loadMusicFilesFromCD();
	void unloadMusicFilesFromCD();
protected:
  mutable std::map<unsigned,bool> m_fileExist;
private:
  friend class NativeUserStorage;
  void withdrawUserStorage(const NativeUserStorage* storage) const noexcept {
    if(m_userStorage==storage) m_userStorage=nullptr;
  }
  struct NativeMounts;
  std::unique_ptr<NativeMounts> m_nativeMounts;
  // Outputs retain this token without callbacks into a possibly retired owner.
  // Its outstanding references prohibit changes to asset/write ownership.
  std::shared_ptr<const int> m_outputLease;
  mutable const NativeUserStorage* m_userStorage = nullptr;
};

extern FileSystem*	TheFileSystem;



//----------------------------------------------------------------------------
//           Inlining                                                       
//----------------------------------------------------------------------------



#endif // __WSYS_FILESYSTEM_H
