cmake_minimum_required(VERSION 3.25)

project(GeneralsZeroHourNative VERSION 0.0.0 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ZhHostChecks)
include(ZhDependencies)

zh_require_x86_64("${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Zero Hour host architecture: ${CMAKE_SYSTEM_PROCESSOR}")

option(ZH_BUILD_TESTS "Build asset-free native-port tests" ON)
option(ZH_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ZH_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ZH_ENABLE_LAN "Build LAN support when its milestone is available" ON)
option(ZH_ENABLE_GPU_TESTS "Enable tests that require a real GPU" OFF)
option(ZH_ENABLE_RETAIL_TESTS "Enable tests that require user-owned retail data" OFF)

set(ZH_RETAIL_ZH_DATA "" CACHE PATH "Local Zero Hour retail data root (retail tests only)")
set(ZH_RETAIL_GENERALS_DATA "" CACHE PATH "Local Generals retail data root (retail tests only)")
set(ZH_RETAIL_LANGUAGE "" CACHE STRING "Retail language name (retail tests only)")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

zh_find_distribution_dependencies()
find_package(Threads REQUIRED)

execute_process(
  COMMAND git rev-parse --short=12 HEAD
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  RESULT_VARIABLE ZH_GIT_RESULT
  OUTPUT_VARIABLE ZH_GIT_REVISION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)
if(NOT ZH_GIT_RESULT EQUAL 0)
  set(ZH_GIT_REVISION "unknown")
endif()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
configure_file(cmake/build_metadata.h.in generated/build_metadata.h @ONLY)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/data/corpus/english-manifest.tsv" ZH_CORPUS_MANIFEST_TEXT)
configure_file(cmake/corpus_manifest.h.in generated/corpus_manifest.h @ONLY)

add_library(zh_build_options INTERFACE)
target_compile_options(zh_build_options INTERFACE
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-fno-fast-math;-ffp-contract=off>
)
if(ZH_ENABLE_ASAN)
  target_compile_options(zh_build_options INTERFACE -fsanitize=address -fno-omit-frame-pointer)
  target_link_options(zh_build_options INTERFACE -fsanitize=address)
endif()

add_library(zh_original_process STATIC
  src/original_runtime/process_services.cpp
  src/original_runtime/utf16_compat.cpp
  GeneralsMD/Code/GameEngine/Source/Common/version.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/GameMemory.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/MemoryInit.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/CriticalSection.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/FPUControl.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/AsciiString.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/UnicodeString.cpp
)
target_include_directories(zh_original_process
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Include"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/original_runtime/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWLib"
)
target_link_libraries(zh_original_process PUBLIC zh_build_options Threads::Threads)
if(ZH_BUILD_TESTS)
  target_compile_definitions(zh_original_process PUBLIC ZH_ORIGINAL_RUNTIME_TEST_HOOKS=1)
endif()
if(ZH_ENABLE_UBSAN)
  target_compile_options(zh_build_options INTERFACE -fsanitize=undefined -fno-omit-frame-pointer)
  target_link_options(zh_build_options INTERFACE -fsanitize=undefined)
endif()

include(cmake/LegacySourceInventory.cmake)

function(zh_add_bootstrap_component target inventory_variable)
  add_library(${target} STATIC src/bootstrap/component.cpp)
  target_compile_definitions(${target} PRIVATE
    ZH_COMPONENT_NAME="${target}"
    ZH_COMPONENT_SYMBOL=${target}_bootstrap_component
  )
  target_link_libraries(${target} PUBLIC zh_build_options)
  set_property(TARGET ${target} PROPERTY ZH_LEGACY_SOURCE_INVENTORY "${${inventory_variable}}")
endfunction()

add_library(zh_compression STATIC
  src/foundation/compression.cpp
  src/original_support/compression.cpp
  GeneralsMD/Code/Libraries/Source/Compression/EAC/refabout.cpp
  GeneralsMD/Code/Libraries/Source/Compression/EAC/refdecode.cpp
  GeneralsMD/Code/Libraries/Source/Compression/EAC/refencode.cpp
)
target_include_directories(zh_compression
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/Compression/EAC"
)
target_link_libraries(zh_compression PUBLIC zh_build_options)
set_property(TARGET zh_compression PROPERTY ZH_LEGACY_SOURCE_INVENTORY "${ZH_LEGACY_ZH_COMPRESSION_SOURCES}")
add_library(zh_wwsupport STATIC
  src/foundation/byte_codec.cpp
  src/foundation/format.cpp
  src/foundation/numeric.cpp
  src/foundation/unicode.cpp
  src/original_support/ww_support.cpp
  GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/FastAllocator.cpp
  GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/chunkio.cpp
  GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/gcd_lcm.cpp
  GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/nstrdup.cpp
  GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/ramfile.cpp
  GeneralsMD/Code/Libraries/Source/WWVegas/WWMath/tri.cpp
  GeneralsMD/Code/Libraries/Source/WWVegas/WWSaveLoad/pointerremap.cpp
)
target_compile_definitions(zh_wwsupport PRIVATE NDEBUG)
target_include_directories(zh_wwsupport
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWLib"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWMath"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWSaveLoad"
)
target_link_libraries(zh_wwsupport PUBLIC zh_build_options)
set_property(TARGET zh_wwsupport PROPERTY ZH_LEGACY_SOURCE_INVENTORY "${ZH_LEGACY_ZH_WWSUPPORT_SOURCES}")
zh_add_bootstrap_component(zh_w3d ZH_LEGACY_ZH_W3D_SOURCES)
zh_add_bootstrap_component(zh_wwshade ZH_LEGACY_ZH_WWSHADE_SOURCES)
zh_add_bootstrap_component(zh_game_engine ZH_LEGACY_ZH_GAME_ENGINE_SOURCES)
zh_add_bootstrap_component(zh_platform_sdl ZH_EMPTY_LEGACY_SOURCES)
zh_add_bootstrap_component(zh_renderer_sdl_gpu ZH_EMPTY_LEGACY_SOURCES)
add_library(zh_audio_miniaudio STATIC
  src/audio/manager.cpp
  src/audio/miniaudio_impl.cpp
  src/audio/probe.cpp
  src/audio/vfs.cpp
)
target_include_directories(zh_audio_miniaudio PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/third_party/miniaudio"
)
target_compile_definitions(zh_audio_miniaudio PRIVATE MA_NO_FLAC MA_NO_ENCODING MA_NO_GENERATION)
target_link_libraries(zh_audio_miniaudio PUBLIC zh_build_options Threads::Threads ${CMAKE_DL_LIBS} zh_data)
add_library(zh_video_ffmpeg STATIC
  src/bootstrap/component.cpp
  src/video/decoder.cpp
  src/video/player.cpp
  src/video/resolver.cpp
)
target_compile_definitions(zh_video_ffmpeg PRIVATE
  ZH_COMPONENT_NAME="zh_video_ffmpeg"
  ZH_COMPONENT_SYMBOL=zh_video_ffmpeg_bootstrap_component
)
set_property(TARGET zh_video_ffmpeg PROPERTY ZH_LEGACY_SOURCE_INVENTORY "${ZH_EMPTY_LEGACY_SOURCES}")
target_include_directories(zh_video_ffmpeg PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_video_ffmpeg PUBLIC zh_build_options zh_data
  FFmpeg::avformat FFmpeg::avcodec FFmpeg::avutil FFmpeg::swscale FFmpeg::swresample)
zh_add_bootstrap_component(zh_os_posix ZH_EMPTY_LEGACY_SOURCES)
zh_add_bootstrap_component(zh_game_device ZH_LEGACY_ZH_GAME_DEVICE_SOURCES)
zh_add_bootstrap_component(zh_null_platform ZH_EMPTY_LEGACY_SOURCES)
zh_add_bootstrap_component(zh_null_renderer ZH_EMPTY_LEGACY_SOURCES)
zh_add_bootstrap_component(zh_null_audio ZH_EMPTY_LEGACY_SOURCES)
zh_add_bootstrap_component(zh_null_video ZH_EMPTY_LEGACY_SOURCES)
zh_add_bootstrap_component(zh_test_support ZH_EMPTY_LEGACY_SOURCES)

add_library(zh_data STATIC src/data/config.cpp src/data/big_archive.cpp src/data/vfs.cpp src/data/inventory.cpp)
target_include_directories(zh_data PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_data PUBLIC zh_os_posix zh_build_options)

add_library(zh_original_data STATIC
  GeneralsMD/Code/GameEngine/Source/Common/System/OriginalFileSystem.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/OriginalINI.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/OriginalCSF.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/OriginalXfer.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/OriginalDataChunk.cpp
  GeneralsMD/Code/GameEngine/Source/Common/OriginalRandomStreams.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/OriginalMapMetadata.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RandomValue.cpp
  GeneralsMD/Code/GameEngine/Source/Common/crc.cpp
)
target_include_directories(zh_original_data
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/original_runtime/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Include"
)
target_link_libraries(zh_original_data PUBLIC zh_data zh_original_process zh_build_options)

add_library(zh_original_resources STATIC
  GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/OriginalCpuPresentation.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/OriginalUiResources.cpp
  GeneralsMD/Code/GameEngine/Source/Common/Audio/OriginalAudioDefinitions.cpp
)
target_include_directories(zh_original_resources PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_original_resources PUBLIC
  zh_original_data zh_renderer_sdl_gpu zh_ui zh_audio_miniaudio zh_build_options)

# Compile the actual source lifecycle bodies from the start of M20.  They are
# an object target until the following slices provide the complete registry and
# device closure; no alternate GameEngine or unresolved-symbol link is used.
add_library(zh_original_lifecycle_spine OBJECT
  GeneralsMD/Code/GameEngine/Source/Common/GameMain.cpp
  GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp
)
target_include_directories(zh_original_lifecycle_spine PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src/original_runtime/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Include"
  "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Include"
  "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas"
  "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWLib"
)
target_link_libraries(zh_original_lifecycle_spine PRIVATE zh_build_options)
target_compile_options(zh_original_lifecycle_spine PRIVATE
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-ffunction-sections;-fdata-sections>)

add_library(zh_original_lifecycle STATIC
  GeneralsMD/Code/GameEngine/Source/Common/System/SubsystemInterface.cpp
)
target_include_directories(zh_original_lifecycle
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Include"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/original_runtime/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Include"
)
target_link_libraries(zh_original_lifecycle PUBLIC zh_original_process zh_build_options)

# M20 forces the production dispatch object into the executable so the complete
# INI type table is live.  Callback and runtime providers stay in a normal
# archive: the link map therefore distinguishes providers actually reached by
# that table from archive members that were merely compiled.
add_library(zh_original_config_spine OBJECT
  GeneralsMD/Code/GameEngine/Source/Common/INI/INI.cpp
)
add_library(zh_original_config_providers STATIC
  GeneralsMD/Code/GameEngine/Source/Common/GlobalData.cpp
  GeneralsMD/Code/GameEngine/Source/Common/Language.cpp
  GeneralsMD/Code/GameEngine/Source/Common/CommandLine.cpp
  GeneralsMD/Code/GameEngine/Source/Common/Dict.cpp
  GeneralsMD/Code/GameEngine/Source/Common/Audio/AudioEventRTS.cpp
  GeneralsMD/Code/GameEngine/Source/Common/Audio/DynamicAudioEventInfo.cpp
  GeneralsMD/Code/GameEngine/Source/Common/DamageFX.cpp
  GeneralsMD/Code/GameEngine/Source/Common/MultiplayerSettings.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RTS/Money.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RTS/MissionStats.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RTS/SpecialPower.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/GameType.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/List.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/Geometry.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/QuotedPrintable.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/Snapshot.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/DataChunk.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/Trig.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/Upgrade.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/registry.cpp
  GeneralsMD/Code/GameEngine/Source/Common/TerrainTypes.cpp
  GeneralsMD/Code/GameEngine/Source/Common/Thing/Thing.cpp
  GeneralsMD/Code/GameEngine/Source/Common/Thing/ThingFactory.cpp
  GeneralsMD/Code/GameEngine/Source/Common/Thing/ThingTemplate.cpp
  GeneralsMD/Code/GameEngine/Source/Common/UserPreferences.cpp
  GeneralsMD/Code/GameEngine/Source/Common/GameLOD.cpp
  GeneralsMD/Code/GameEngine/Source/Common/NameKeyGenerator.cpp
  GeneralsMD/Code/GameEngine/Source/Common/WellKnownKeys.cpp
  GeneralsMD/Code/GameEngine/Source/Common/MapObjectGlobals.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/FileSystem.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/LocalFileSystem.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/LocalFile.cpp
  GeneralsMD/Code/GameEngineDevice/Source/PosixDevice/Common/PosixLocalFile.cpp
  GeneralsMD/Code/GameEngineDevice/Source/PosixDevice/Common/PosixLocalFileSystem.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/File.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/RAMFile.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/ArchiveFile.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/ArchiveFileSystem.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/StreamingArchiveFile.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/Xfer.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/XferCRC.cpp
  GeneralsMD/Code/GameEngine/Source/Common/MessageStream.cpp
  GeneralsMD/Code/GameEngine/Source/Common/Recorder.cpp
  GeneralsMD/Code/GameEngine/Source/Common/PartitionSolver.cpp
  GeneralsMD/Code/GameEngine/Source/Common/StatsCollector.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/CDManager.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/SaveGame/GameState.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/SaveGame/GameStateGlobals.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/SaveGame/GameStateMap.cpp
  GeneralsMD/Code/GameEngine/Source/Common/Audio/AudioRequest.cpp
  GeneralsMD/Code/GameEngine/Source/Common/Audio/GameMusic.cpp
  GeneralsMD/Code/GameEngine/Source/Common/Audio/GameSounds.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GameText.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/Color.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GameFont.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/LanguageFilter.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/Drawable.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/MapUtil.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GameWindow.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/WindowLayout.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/WindowVideoManager.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Gadget/GadgetPushButton.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Gadget/GadgetCheckBox.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Gadget/GadgetHorizontalSlider.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Gadget/GadgetVerticalSlider.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Gadget/GadgetTabControl.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Gadget/GadgetComboBox.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Gadget/GadgetListBox.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Gadget/GadgetProgressBar.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Gadget/GadgetRadioButton.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Gadget/GadgetTextEntry.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/WinInstanceData.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/AnimateWindowManager.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/ProcessAnimateWindow.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/SelectionInfo.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GameClientDispatch.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/MessageStream/CommandXlat.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/MessageStream/GUICommandTranslator.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/MessageStream/HintSpy.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/MessageStream/HotKey.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/MessageStream/LookAtXlat.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/MessageStream/PlaceEventTranslator.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/MessageStream/SelectionXlat.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/MessageStream/WindowXlat.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/RadiusDecal.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/System/Anim2D.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/System/RayEffect.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/System/Image.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/System/ParticleSys.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/Terrain/TerrainRoads.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/Water.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AI.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/ScriptEngine/ScriptEngine.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/ScriptEngine/ScriptActions.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/ScriptEngine/ScriptConditions.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/ObjectTypes.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/CommandButtonHuntUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/DockUpdate/SupplyWarehouseDockUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/DockUpdate/DockUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/ProductionUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/SpecialAbilityUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/StickyBombUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/BattlePlanUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/OCLUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIAiData.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIAnimation.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Armor.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIAudioEventInfo.cpp
  GeneralsMD/Code/GameEngine/Source/Common/Audio/GameAudio.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INITerrainBridge.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/System/CampaignManager.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/ChallengeGenerals.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INICommandButton.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INICommandSet.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIControlBarScheme.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/ControlBar/ControlBarResizer.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INICrate.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/Credits.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIDamageFX.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIDrawGroupInfo.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/Eva.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/FXList.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIGameData.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/HeaderTemplate.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/InGameUI.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GlobalLanguage.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Locomotor.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIMapCache.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIMapData.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIMappedImage.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/MessageStream/MetaEvent.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIMiscAudio.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/Input/Mouse.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIMultiplayer.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIObject.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/ObjectCreationList.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/ExperienceTracker.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Object.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/PartitionManager.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/AIUpdate/DeliverPayloadAIUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/PhysicsUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/RadiusDecalUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Weapon.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/WeaponSet.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/ScriptEngine/Scripts.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/System/CrateSystem.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/System/CaveSystem.cpp
  GeneralsMD/Code/GameEngine/Source/GameNetwork/GameSpy/Chat.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIParticleSys.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RTS/PlayerTemplate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/System/RankInfo.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RTS/Science.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Shell/ShellMenuScheme.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INISpecialPower.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INITerrain.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INITerrainRoad.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIUpgrade.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIVideo.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIWater.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIWeapon.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/Snow.cpp
  GeneralsMD/Code/GameEngine/Source/Common/INI/INIWebpageURL.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GameWindowTransitions.cpp
  GeneralsMD/Code/GameEngine/Source/Common/BitFlags.cpp
  GeneralsMD/Code/GameEngine/Source/Common/DiscreteCircle.cpp
  GeneralsMD/Code/GameEngine/Source/Common/StateMachine.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RTS/AcademyStats.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RTS/ActionManager.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RTS/Handicap.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RTS/Player.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RTS/PlayerList.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RTS/ProductionPrerequisite.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RTS/ResourceGatheringManager.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RTS/Team.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RTS/TunnelTracker.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/DisabledTypes.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/GameCommon.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/KindOf.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/ObjectStatusTypes.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/Radar.cpp
  GeneralsMD/Code/GameEngine/Source/Common/Thing/Module.cpp
  GeneralsMD/Code/GameEngine/Source/Common/Thing/ModuleFactory.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/Display.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/DrawGroupInfo.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GameClient.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/Line2D.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/Terrain/TerrainVisual.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/VideoPlayer.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/View.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/ControlBar/ControlBar.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/ControlBar/ControlBarBeacon.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/ControlBar/ControlBarCommand.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/ControlBar/ControlBarMultiSelect.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/ControlBar/ControlBarOCLTimer.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/ControlBar/ControlBarObserver.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/ControlBar/ControlBarStructureInventory.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/ControlBar/ControlBarUnderConstruction.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/ControlBarCallback.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/ControlBarPopupDescription.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Diplomacy.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/InGameChat.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/QuitMenu.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/ControlBar/ControlBarScheme.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/OptionsMenu.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Shell/Shell.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AIPathfind.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AIDock.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AIGuard.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AIGuardRetaliate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AITNGuard.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/AI/Squad.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Map/TerrainLogic.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/FiringTracker.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/GhostObject.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/SimpleObjectIterator.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Behavior/AutoHealBehavior.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Behavior/BehaviorModule.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Behavior/BridgeBehavior.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Behavior/BridgeTowerBehavior.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Behavior/RebuildHoleBehavior.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Behavior/GenerateMinefieldBehavior.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Die/SpecialPowerCompletionDie.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Helper/ObjectDefectionHelper.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Helper/ObjectHelper.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Helper/ObjectRepulsorHelper.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Helper/ObjectSMCHelper.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Helper/ObjectWeaponStatusHelper.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Helper/StatusDamageHelper.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Helper/SubdualDamageHelper.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Helper/TempWeaponBonusHelper.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/AssistedTargetingUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/LaserUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/LifetimeUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/ProjectileStreamUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/SmartBombTargetHomingUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/UpdateModule.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/System/Damage.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/Drawable/Update/BeaconClientUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameNetwork/FileTransfer.cpp
  GeneralsMD/Code/GameEngine/Source/GameNetwork/GameSpy/Thread/ThreadUtils.cpp
  GeneralsMD/Code/Libraries/Source/Compression/CompressionManager.cpp
  GeneralsMD/Code/Libraries/Source/Compression/EAC/btreedecode.cpp
  GeneralsMD/Code/Libraries/Source/Compression/EAC/huffdecode.cpp
  GeneralsMD/Code/Libraries/Source/WWVegas/WWMath/matrix3d.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RecorderState.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RTS/Energy.cpp
  GeneralsMD/Code/GameEngine/Source/Common/RTS/ScoreKeeper.cpp
  GeneralsMD/Code/GameEngine/Source/Common/System/BuildAssistant.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/DisplayStringManager.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/Shadow.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GameWindow.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GameWindowManager.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GameWindowManagerScript.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GameWindowTransitionsStyles.cpp
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Gadget/GadgetStaticText.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AIGroup.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AIPlayer.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AISkirmishPlayer.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AIStates.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/AI/TurretAI.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Map/PolygonTrigger.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Map/SidesList.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/AIUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/AIUpdate/HackInternetAIUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/AIUpdate/SupplyTruckAIUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/AutoDepositUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/ProneUpdate.cpp
  GeneralsMD/Code/GameEngine/Source/GameLogic/ScriptEngine/VictoryConditions.cpp
  GeneralsMD/Code/GameEngine/Source/GameNetwork/IPEnumeration.cpp
  GeneralsMD/Code/GameEngine/Source/GameNetwork/GameInfo.cpp
  GeneralsMD/Code/GameEngine/Source/GameNetwork/GameMessageParser.cpp
  GeneralsMD/Code/GameEngine/Source/GameNetwork/NetworkLinuxFactory.cpp
  src/original_runtime/linux_legacy_boundary.cpp
  src/original_runtime/posix_crash.cpp
  src/original_runtime/well_known_keys.cpp
)
set_source_files_properties(
  GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/OptionsMenu.cpp
  PROPERTIES COMPILE_DEFINITIONS ZH_OPTION_PREFERENCES_ONLY)
foreach(config_target zh_original_config_spine zh_original_config_providers)
  target_include_directories(${config_target} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/original_runtime/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngineDevice/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/Compression"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWDebug"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWLib")
  target_link_libraries(${config_target} PRIVATE zh_build_options)
  target_compile_options(${config_target} PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-ffunction-sections;-fdata-sections>)
endforeach()

add_library(zh_persistence STATIC src/persistence/save.cpp src/persistence/replay.cpp)
target_include_directories(zh_persistence PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_persistence PUBLIC zh_wwsupport zh_build_options)

add_library(zh_simulation STATIC src/simulation/determinism.cpp)
target_include_directories(zh_simulation PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_simulation PUBLIC zh_persistence zh_build_options)

add_library(zh_singleplayer STATIC src/singleplayer/session.cpp src/singleplayer/game_flow.cpp)
target_include_directories(zh_singleplayer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_singleplayer PUBLIC zh_simulation zh_data zh_os_posix zh_ui zh_world zh_effects
  zh_audio_miniaudio zh_video_ffmpeg zh_build_options)

add_library(zh_lan STATIC
  src/lan/harness.cpp
  src/lan/posix_udp.cpp
  src/lan/protocol.cpp
  src/lan/session.cpp
  src/lan/virtual_transport.cpp
)
target_include_directories(zh_lan PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_lan PUBLIC zh_wwsupport zh_os_posix zh_build_options)

target_sources(zh_renderer_sdl_gpu PRIVATE
  src/renderer/contract.cpp
  src/renderer/dx8_state_cache.cpp
  src/renderer/recording_device.cpp
  src/renderer/sdl_gpu_device.cpp
  src/renderer/shader_registry.cpp
  src/renderer/texture_loader.cpp
)
target_include_directories(zh_renderer_sdl_gpu PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(zh_ui STATIC src/ui/renderer.cpp src/ui/font.cpp src/ui/session.cpp)
target_include_directories(zh_ui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_ui PUBLIC zh_renderer_sdl_gpu zh_platform_sdl zh_data zh_build_options Freetype::Freetype Fontconfig::Fontconfig)

add_library(zh_world STATIC src/world/scene.cpp src/world/recorder.cpp src/world/corpus.cpp)
target_include_directories(zh_world PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_world PUBLIC zh_renderer_sdl_gpu zh_build_options)

add_library(zh_effects STATIC src/effects/registry.cpp src/effects/recorder.cpp)
target_include_directories(zh_effects PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_effects PUBLIC zh_renderer_sdl_gpu zh_wwshade zh_build_options)

target_sources(zh_platform_sdl PRIVATE src/platform/sdl_events.cpp src/platform/sdl_window.cpp)
target_include_directories(zh_platform_sdl PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_platform_sdl PUBLIC zh_wwsupport)

target_sources(zh_os_posix PRIVATE src/foundation/platform.cpp)
target_include_directories(zh_os_posix PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(zh_null_platform PRIVATE src/headless/null_platform.cpp)
target_sources(zh_null_renderer PRIVATE src/headless/null_renderer.cpp)
target_sources(zh_null_audio PRIVATE src/headless/null_audio.cpp)
target_sources(zh_null_video PRIVATE src/headless/null_video.cpp)
foreach(null_target zh_null_platform zh_null_renderer zh_null_audio zh_null_video)
  target_include_directories(${null_target} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
endforeach()

target_link_libraries(zh_compression PUBLIC ZLIB::ZLIB zh_wwsupport)
target_link_libraries(zh_wwsupport PUBLIC Freetype::Freetype Fontconfig::Fontconfig)
target_link_libraries(zh_w3d PUBLIC zh_wwsupport)
target_link_libraries(zh_wwshade PUBLIC zh_w3d)
target_link_libraries(zh_game_engine PUBLIC zh_compression zh_wwsupport)
target_link_libraries(zh_platform_sdl PUBLIC SDL3::SDL3)
target_link_libraries(zh_renderer_sdl_gpu PUBLIC SDL3::SDL3 zh_w3d zh_wwshade)
target_link_libraries(zh_video_ffmpeg PUBLIC zh_audio_miniaudio zh_renderer_sdl_gpu)
target_link_libraries(zh_os_posix PUBLIC Threads::Threads)
target_link_libraries(zh_game_device PUBLIC zh_platform_sdl zh_renderer_sdl_gpu zh_effects zh_audio_miniaudio zh_video_ffmpeg zh_os_posix)

set(ZH_BOOTSTRAP_SHADER "${CMAKE_CURRENT_SOURCE_DIR}/shaders/bootstrap/bootstrap.vert")
set(ZH_BOOTSTRAP_SPIRV "${CMAKE_CURRENT_BINARY_DIR}/generated/shaders/bootstrap.vert.spv")
add_custom_command(
  OUTPUT "${ZH_BOOTSTRAP_SPIRV}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated/shaders"
  COMMAND "${ZH_GLSLC_EXECUTABLE}" -fshader-stage=vert "${ZH_BOOTSTRAP_SHADER}" -o "${ZH_BOOTSTRAP_SPIRV}"
  DEPENDS "${ZH_BOOTSTRAP_SHADER}"
  COMMENT "Compiling bootstrap GLSL to SPIR-V"
  VERBATIM
)
add_custom_target(zh_bootstrap_shaders ALL DEPENDS "${ZH_BOOTSTRAP_SPIRV}")
add_dependencies(zh_renderer_sdl_gpu zh_bootstrap_shaders)

set(ZH_RENDERER_SHADER_NAMES
  acceptance.vert acceptance.frag
  world.vert world.frag
  video.vert video.frag
  ui.vert ui.frag
  terrain.vert terrain.frag
  water.vert water.frag
  points.vert points.frag
  wwshade.vert wwshade.frag
)
set(ZH_RENDERER_SPIRV)
foreach(shader_name IN LISTS ZH_RENDERER_SHADER_NAMES)
  set(shader_source "${CMAKE_CURRENT_SOURCE_DIR}/shaders/renderer/${shader_name}")
  set(shader_output "${CMAKE_CURRENT_BINARY_DIR}/generated/shaders/renderer/${shader_name}.spv")
  add_custom_command(
    OUTPUT "${shader_output}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated/shaders/renderer"
    COMMAND "${ZH_GLSLC_EXECUTABLE}" "${shader_source}" -o "${shader_output}"
    DEPENDS "${shader_source}"
    COMMENT "Compiling renderer shader ${shader_name}"
    VERBATIM
  )
  list(APPEND ZH_RENDERER_SPIRV "${shader_output}")
endforeach()
add_custom_target(zh_renderer_shaders ALL DEPENDS ${ZH_RENDERER_SPIRV})
add_dependencies(zh_renderer_sdl_gpu zh_renderer_shaders)

set(ZH_EFFECT_SHADER_NAMES
  projected.vert projected.frag
  bump_environment.vert bump_environment.frag
  stealth.vert stealth.frag
  post_effect.vert post_effect.frag
)
set(ZH_EFFECT_SPIRV)
foreach(shader_name IN LISTS ZH_EFFECT_SHADER_NAMES)
  set(shader_source "${CMAKE_CURRENT_SOURCE_DIR}/shaders/effects/${shader_name}")
  set(shader_output "${CMAKE_CURRENT_BINARY_DIR}/generated/shaders/effects/${shader_name}.spv")
  add_custom_command(
    OUTPUT "${shader_output}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated/shaders/effects"
    COMMAND "${ZH_GLSLC_EXECUTABLE}" "${shader_source}" -o "${shader_output}"
    DEPENDS "${shader_source}"
    COMMENT "Compiling effect shader ${shader_name}"
    VERBATIM
  )
  list(APPEND ZH_EFFECT_SPIRV "${shader_output}")
endforeach()
add_custom_target(zh_effect_shaders ALL DEPENDS ${ZH_EFFECT_SPIRV})
add_dependencies(zh_effects zh_renderer_shaders zh_effect_shaders)

add_executable(zh_main src/bootstrap/main.cpp src/headless/runtime.cpp)
target_include_directories(zh_main PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated" "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(zh_main PRIVATE
  zh_build_options zh_game_engine zh_game_device zh_null_platform zh_null_renderer
  zh_null_audio zh_null_video zh_test_support zh_data
  zh_lan zh_singleplayer
)
set_property(TARGET zh_main PROPERTY ZH_LEGACY_SOURCE_INVENTORY "${ZH_LEGACY_ZH_MAIN_SOURCES}")

install(TARGETS zh_main RUNTIME DESTINATION bin)
install(FILES "${ZH_BOOTSTRAP_SPIRV}" DESTINATION share/generals-zero-hour/shaders)
install(FILES ${ZH_RENDERER_SPIRV} DESTINATION share/generals-zero-hour/shaders/renderer)
install(FILES ${ZH_EFFECT_SPIRV} DESTINATION share/generals-zero-hour/shaders/effects)
install(FILES LICENSE.md DESTINATION share/licenses/generals-zero-hour)
install(FILES data/corpus/english-manifest.tsv DESTINATION share/generals-zero-hour/corpus)
install(FILES data/corpus/world-rendering-manifest.tsv DESTINATION share/generals-zero-hour/corpus)
install(FILES data/corpus/effects-manifest.tsv DESTINATION share/generals-zero-hour/corpus)
install(FILES data/corpus/video-variants.tsv DESTINATION share/generals-zero-hour/corpus)

if(ZH_BUILD_TESTS)
  enable_testing()
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  add_executable(original_config_link_probe
    tests/original_lifecycle/test_config_link_probe.cpp
    $<TARGET_OBJECTS:zh_original_config_spine>)
  target_include_directories(original_config_link_probe PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/original_runtime/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngineDevice/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWLib")
  target_link_libraries(original_config_link_probe PRIVATE
    zh_original_config_providers zh_original_lifecycle zh_original_data zh_os_posix zh_build_options)
  target_link_options(original_config_link_probe PRIVATE
    -Wl,--gc-sections
    "-Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/original-config-dispatch.link.map")
  add_test(NAME original_config_dispatch_valid COMMAND original_config_link_probe valid
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/original_lifecycle/fixtures/config-valid.ini")
  add_test(NAME original_config_dispatch_invalid COMMAND original_config_link_probe invalid
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/original_lifecycle/fixtures/config-invalid.ini")
  set_tests_properties(original_config_dispatch_valid original_config_dispatch_invalid PROPERTIES
    LABELS "original-config;source-identity")
  add_test(NAME original_config_dispatch_identity COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_m20_config_identity.py"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-config-dispatch.link.map"
    --executable "$<TARGET_FILE:original_config_link_probe>"
    --dispatch-source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/INI/INI.cpp"
    --dispatch-target zh_original_config_spine
    --provider-target zh_original_config_providers
    --provider-archive "$<TARGET_FILE:zh_original_config_providers>"
    --fixture "${CMAKE_CURRENT_SOURCE_DIR}/tests/original_lifecycle/fixtures/config-valid.ini")
  set_tests_properties(original_config_dispatch_identity PROPERTIES LABELS "original-config;source-identity")
  add_test(NAME original_config_provider_removal COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/original_lifecycle/test_config_provider_removal.py"
    --compiler "${CMAKE_CXX_COMPILER}"
    --archiver "${CMAKE_AR}"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-config-dispatch.link.map"
    --dispatch-source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/INI/INI.cpp"
    --dispatch-target zh_original_config_spine
    --probe-source "${CMAKE_CURRENT_SOURCE_DIR}/tests/original_lifecycle/test_config_link_probe.cpp"
    --probe-target original_config_link_probe
    --provider-archive "$<TARGET_FILE:zh_original_config_providers>"
    --library "$<TARGET_FILE:zh_original_lifecycle>"
    --library "$<TARGET_FILE:zh_original_data>"
    --library "$<TARGET_FILE:zh_os_posix>"
    --library "$<TARGET_FILE:zh_original_process>"
    --library "$<TARGET_FILE:zh_data>"
    --library "$<TARGET_FILE:zh_os_posix>")
  set_tests_properties(original_config_provider_removal PROPERTIES LABELS "original-config;source-identity")
  add_executable(original_data_startup_tests
    tests/original_lifecycle/test_data_startup.cpp
    $<TARGET_OBJECTS:zh_original_config_spine>)
  target_include_directories(original_data_startup_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/original_runtime/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngineDevice/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWLib")
  target_link_libraries(original_data_startup_tests PRIVATE
    zh_original_config_providers zh_original_lifecycle zh_original_data zh_os_posix zh_build_options)
  target_link_options(original_data_startup_tests PRIVATE -Wl,--gc-sections
    "-Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/original-data-startup.link.map")
  add_test(NAME original_data_startup COMMAND original_data_startup_tests)
  set_tests_properties(original_data_startup PROPERTIES LABELS "original-data-startup;source-identity")
  add_test(NAME original_data_startup_identity COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_m20_data_identity.py"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-data-startup.link.map"
    --executable "$<TARGET_FILE:original_data_startup_tests>"
    --provider-target zh_original_config_providers)
  set_tests_properties(original_data_startup_identity PROPERTIES LABELS "original-data-startup;source-identity")
  add_test(NAME original_data_startup_provider_removal COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/original_lifecycle/test_config_provider_removal.py"
    --compiler "${CMAKE_CXX_COMPILER}"
    --archiver "${CMAKE_AR}"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-data-startup.link.map"
    --dispatch-source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/INI/INI.cpp"
    --dispatch-target zh_original_config_spine
    --probe-source "${CMAKE_CURRENT_SOURCE_DIR}/tests/original_lifecycle/test_data_startup.cpp"
    --probe-target original_data_startup_tests
    --provider-archive "$<TARGET_FILE:zh_original_config_providers>"
    --member XferCRC.cpp.o
    --required-symbol XferCRC
    --library "$<TARGET_FILE:zh_original_lifecycle>"
    --library "$<TARGET_FILE:zh_original_data>"
    --library "$<TARGET_FILE:zh_os_posix>"
    --library "$<TARGET_FILE:zh_original_process>"
    --library "$<TARGET_FILE:zh_data>"
    --library "$<TARGET_FILE:zh_os_posix>")
  set_tests_properties(original_data_startup_provider_removal PROPERTIES LABELS "original-data-startup;source-identity")
  add_executable(original_headless_update_tests
    tests/original_lifecycle/test_headless_update.cpp
    tests/original_lifecycle/headless_reset_boundary.cpp
    $<TARGET_OBJECTS:zh_original_lifecycle_spine>
    $<TARGET_OBJECTS:zh_original_config_spine>)
  target_include_directories(original_headless_update_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/original_runtime/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngineDevice/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWLib")
  target_link_libraries(original_headless_update_tests PRIVATE
    zh_original_config_providers zh_original_lifecycle zh_original_data zh_os_posix zh_compression zh_build_options)
  target_link_options(original_headless_update_tests PRIVATE -Wl,--gc-sections
    "-Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/original-headless-update.link.map")
  add_test(NAME original_headless_update COMMAND original_headless_update_tests)
  set_tests_properties(original_headless_update PROPERTIES LABELS "original-headless-update;source-identity")
  add_test(NAME original_headless_update_identity COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_m20_headless_identity.py"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-headless-update.link.map"
    --executable "$<TARGET_FILE:original_headless_update_tests>"
    --provider-target zh_original_config_providers
    --spine-target zh_original_lifecycle_spine)
  set_tests_properties(original_headless_update_identity PROPERTIES LABELS "original-headless-update;source-identity")
  add_test(NAME original_headless_update_provider_removal COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/original_lifecycle/test_headless_provider_removal.py"
    --build-dir "${CMAKE_CURRENT_BINARY_DIR}"
    --target original_headless_update_tests
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-headless-update.link.map"
    --provider-archive "$<TARGET_FILE:zh_original_config_providers>"
    --member GameLogic.cpp.o
    --required-symbol TheGameLogic)
  set_tests_properties(original_headless_update_provider_removal PROPERTIES LABELS "original-headless-update;source-identity")
  add_executable(original_lifecycle_subsystem_tests
    tests/original_lifecycle/test_subsystem_lifecycle.cpp)
  target_include_directories(original_lifecycle_subsystem_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/original_runtime/include")
  target_link_libraries(original_lifecycle_subsystem_tests PRIVATE zh_original_lifecycle)
  target_link_options(original_lifecycle_subsystem_tests PRIVATE
    "-Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/original-lifecycle-subsystem.link.map")
  add_test(NAME original_lifecycle_subsystem COMMAND original_lifecycle_subsystem_tests)
  set_tests_properties(original_lifecycle_subsystem PROPERTIES LABELS "original-lifecycle;source-identity")
  add_test(NAME original_lifecycle_subsystem_identity COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_m26_original_identity.py"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-lifecycle-subsystem.link.map"
    --executable "$<TARGET_FILE:original_lifecycle_subsystem_tests>"
    --target zh_original_lifecycle
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/System/SubsystemInterface.cpp"
    --symbol "SubsystemInterfaceList::initSubsystem"
    --symbol "SubsystemInterfaceList::postProcessLoadAll"
    --symbol "SubsystemInterfaceList::resetAll"
    --witness "original-lifecycle subsystem: ok")
  set_tests_properties(original_lifecycle_subsystem_identity PROPERTIES LABELS "original-lifecycle;source-identity")
  add_test(NAME original_lifecycle_spine_objects COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_m20_spine_objects.py"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json")
  set_tests_properties(original_lifecycle_spine_objects PROPERTIES LABELS "original-lifecycle;source-identity")
  add_test(NAME original_lifecycle_enum_abi COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_m20_enum_abi.py"
    --root "${CMAKE_CURRENT_SOURCE_DIR}"
    --manifest "${CMAKE_CURRENT_SOURCE_DIR}/docs/original-runtime-enum-abi.tsv"
    --compiler "${CMAKE_CXX_COMPILER}")
  set_tests_properties(original_lifecycle_enum_abi PROPERTIES LABELS "original-lifecycle;source-identity")
  add_test(NAME original_lifecycle_provider_removal COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/original_lifecycle/test_provider_removal.py"
    "${CMAKE_CXX_COMPILER}"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/original_runtime/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Include")
  set_tests_properties(original_lifecycle_provider_removal PROPERTIES LABELS "original-lifecycle;source-identity")
  add_executable(original_resources_cpu_tests tests/original_resources/test_cpu_presentation.cpp)
  target_link_libraries(original_resources_cpu_tests PRIVATE zh_original_resources)
  target_link_options(original_resources_cpu_tests PRIVATE
    "-Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/original-resources-cpu.link.map")
  add_test(NAME original_resources_cpu COMMAND original_resources_cpu_tests)
  set_tests_properties(original_resources_cpu PROPERTIES LABELS "original-resources;source-identity")
  add_test(NAME original_resources_cpu_identity COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_m26_original_identity.py"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-resources-cpu.link.map"
    --executable "$<TARGET_FILE:original_resources_cpu_tests>"
    --target zh_original_resources
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/OriginalCpuPresentation.cpp"
    --symbol "zh::original_resources::CpuPresentation::initialize"
    --symbol "zh::original_resources::provider_cpu_identity"
    --witness "original-resources CPU: ok")
  set_tests_properties(original_resources_cpu_identity PROPERTIES LABELS "original-resources;source-identity")
  add_executable(original_resources_ui_tests tests/original_resources/test_ui_resources.cpp)
  target_link_libraries(original_resources_ui_tests PRIVATE zh_original_resources)
  target_link_options(original_resources_ui_tests PRIVATE
    "-Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/original-resources-ui.link.map")
  add_test(NAME original_resources_ui COMMAND original_resources_ui_tests)
  set_tests_properties(original_resources_ui PROPERTIES LABELS "original-resources;source-identity")
  add_test(NAME original_resources_ui_identity COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_m26_original_identity.py"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-resources-ui.link.map"
    --executable "$<TARGET_FILE:original_resources_ui_tests>"
    --target zh_original_resources
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/GameClient/GUI/OriginalUiResources.cpp"
    --symbol "zh::original_resources::UiResources::load"
    --symbol "zh::original_resources::provider_ui_identity"
    --witness "original-resources UI: ok")
  set_tests_properties(original_resources_ui_identity PROPERTIES LABELS "original-resources;source-identity")
  add_executable(original_resources_audio_tests tests/original_resources/test_audio_definitions.cpp)
  target_link_libraries(original_resources_audio_tests PRIVATE zh_original_resources)
  target_link_options(original_resources_audio_tests PRIVATE
    "-Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/original-resources-audio.link.map")
  add_test(NAME original_resources_audio COMMAND original_resources_audio_tests)
  set_tests_properties(original_resources_audio PROPERTIES LABELS "original-resources;source-identity")
  add_test(NAME original_resources_audio_identity COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_m26_original_identity.py"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-resources-audio.link.map"
    --executable "$<TARGET_FILE:original_resources_audio_tests>"
    --target zh_original_resources
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/Audio/OriginalAudioDefinitions.cpp"
    --symbol "zh::original_resources::AudioDefinitions::load"
    --symbol "zh::original_resources::provider_audio_identity"
    --witness "original-resources audio: ok")
  set_tests_properties(original_resources_audio_identity PROPERTIES LABELS "original-resources;source-identity")
  add_test(NAME original_resources_provider_removal COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/original_resources/test_provider_removal.py"
    --root "${CMAKE_CURRENT_SOURCE_DIR}"
    --build-dir "${CMAKE_CURRENT_BINARY_DIR}")
  set_tests_properties(original_resources_provider_removal PROPERTIES LABELS "original-resources;source-identity")
  add_test(NAME original_resources_extraction_provenance COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_m28_original_provenance.py"
    --root "${CMAKE_CURRENT_SOURCE_DIR}")
  set_tests_properties(original_resources_extraction_provenance PROPERTIES LABELS "original-resources;source-identity")
  add_test(NAME original_resources_dependency_ledger COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_original_dependency_ledger.py"
    --root "${CMAKE_CURRENT_SOURCE_DIR}"
    --ledger "${CMAKE_CURRENT_SOURCE_DIR}/docs/original-runtime-dependency-ledger.tsv")
  set_tests_properties(original_resources_dependency_ledger PROPERTIES LABELS "original-resources;source-identity")
  add_test(NAME original_resources_dependency_controls COMMAND "${Python3_EXECUTABLE}" -m unittest
    tests.original_runtime.test_dependency_controls)
  set_tests_properties(original_resources_dependency_controls PROPERTIES
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" LABELS "original-resources;source-identity")
  add_executable(original_data_config_tests tests/original_data/test_config.cpp)
  target_link_libraries(original_data_config_tests PRIVATE zh_original_data)
  target_link_options(original_data_config_tests PRIVATE
    "-Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/original-data-config.link.map")
  add_test(NAME original_data_config COMMAND original_data_config_tests)
  set_tests_properties(original_data_config PROPERTIES LABELS "original-data;source-identity")
  add_test(NAME original_data_config_identity COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_m26_original_identity.py"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-data-config.link.map"
    --executable "$<TARGET_FILE:original_data_config_tests>"
    --target zh_original_data
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/System/OriginalFileSystem.cpp"
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/INI/OriginalINI.cpp"
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/GameClient/OriginalCSF.cpp"
    --symbol "zh::original_data::LogicalFiles::read"
    --symbol "zh::original_data::parse_ini"
    --symbol "zh::original_data::parse_csf"
    --symbol "setFPMode()"
    --witness "original-data config: ok")
  set_tests_properties(original_data_config_identity PROPERTIES LABELS "original-data;source-identity")
  add_executable(original_data_codec_tests tests/original_data/test_codecs.cpp)
  target_link_libraries(original_data_codec_tests PRIVATE zh_original_data)
  target_link_options(original_data_codec_tests PRIVATE
    "-Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/original-data-codecs.link.map")
  add_test(NAME original_data_codecs COMMAND original_data_codec_tests)
  set_tests_properties(original_data_codecs PROPERTIES LABELS "original-data;source-identity")
  add_test(NAME original_data_codecs_identity COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_m26_original_identity.py"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-data-codecs.link.map"
    --executable "$<TARGET_FILE:original_data_codec_tests>"
    --target zh_original_data
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/System/OriginalXfer.cpp"
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/System/OriginalDataChunk.cpp"
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/RandomValue.cpp"
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/crc.cpp"
    --symbol "zh::original_data::write_xfer_record"
    --symbol "zh::original_data::read_data_chunks"
    --symbol "GetGameLogicRandomSeedCRC()"
    --symbol "GetGameClientRandomValue"
    --witness "original-data codecs: ok")
  set_tests_properties(original_data_codecs_identity PROPERTIES LABELS "original-data;source-identity")
  add_executable(original_data_map_tests tests/original_data/test_map_metadata.cpp)
  target_link_libraries(original_data_map_tests PRIVATE zh_original_data)
  target_link_options(original_data_map_tests PRIVATE
    "-Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/original-data-map.link.map")
  add_test(NAME original_data_map_metadata COMMAND original_data_map_tests)
  set_tests_properties(original_data_map_metadata PROPERTIES LABELS "original-data;source-identity")
  add_test(NAME original_data_map_identity COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_m26_original_identity.py"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-data-map.link.map"
    --executable "$<TARGET_FILE:original_data_map_tests>"
    --target zh_original_data
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/GameClient/OriginalMapMetadata.cpp"
    --symbol "zh::original_data::load_map_catalog"
    --symbol "setFPMode()"
    --witness "original-data map: ok")
  set_tests_properties(original_data_map_identity PROPERTIES LABELS "original-data;source-identity")
  add_test(NAME original_data_dependency_ledger COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_original_dependency_ledger.py"
    --root "${CMAKE_CURRENT_SOURCE_DIR}"
    --ledger "${CMAKE_CURRENT_SOURCE_DIR}/docs/original-runtime-dependency-ledger.tsv")
  set_tests_properties(original_data_dependency_ledger PROPERTIES LABELS "original-data;source-identity")
  add_test(NAME original_data_extraction_provenance COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_m27_original_provenance.py"
    --root "${CMAKE_CURRENT_SOURCE_DIR}")
  set_tests_properties(original_data_extraction_provenance PROPERTIES LABELS "original-data;source-identity")
  add_test(NAME original_data_provenance_controls COMMAND "${Python3_EXECUTABLE}" -m unittest
    tests.original_data.test_provenance_controls)
  set_tests_properties(original_data_provenance_controls PROPERTIES
    LABELS "original-data;source-identity"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
  add_test(NAME original_data_provider_removal COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/original_data/test_provider_removal.py"
    "${CMAKE_CXX_COMPILER}" "${CMAKE_CURRENT_SOURCE_DIR}")
  set_tests_properties(original_data_provider_removal PROPERTIES LABELS "original-data;source-identity")
  add_executable(original_process_allocator_tests
    tests/original_runtime/test_allocator.cpp
    tests/original_runtime/allocator_provider.cpp
  )
  target_include_directories(original_process_allocator_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/original_runtime/include")
  target_link_libraries(original_process_allocator_tests PRIVATE zh_original_process)
  target_link_options(original_process_allocator_tests PRIVATE
    "-Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/original-process-allocator.link.map")
  add_test(NAME original_process_allocator COMMAND original_process_allocator_tests defaults)
  set_tests_properties(original_process_allocator PROPERTIES LABELS "original-process;source-identity")
  add_test(NAME original_process_pool_config COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/original_runtime/test_pool_config.py"
    "$<TARGET_FILE:original_process_allocator_tests>")
  set_tests_properties(original_process_pool_config PROPERTIES LABELS "original-process" TIMEOUT 30)
  add_executable(original_process_string_tests
    tests/original_runtime/test_strings.cpp
    tests/original_runtime/string_provider.cpp
  )
  target_include_directories(original_process_string_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/original_runtime/include")
  target_link_libraries(original_process_string_tests PRIVATE zh_original_process)
  target_link_options(original_process_string_tests PRIVATE
    "-Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/original-process-strings.link.map")
  add_test(NAME original_process_strings COMMAND original_process_string_tests)
  set_tests_properties(original_process_strings PROPERTIES LABELS "original-process;source-identity")
  add_executable(original_process_fpu_tests tests/original_runtime/test_fpu.cpp)
  target_include_directories(original_process_fpu_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/original_runtime/include")
  target_link_libraries(original_process_fpu_tests PRIVATE zh_original_process)
  target_link_options(original_process_fpu_tests PRIVATE
    "-Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/original-process-fpu.link.map")
  add_test(NAME original_process_fpu COMMAND original_process_fpu_tests)
  set_tests_properties(original_process_fpu PROPERTIES LABELS "original-process;source-identity")
  add_test(NAME original_process_dependency_ledger COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_original_dependency_ledger.py"
    --root "${CMAKE_CURRENT_SOURCE_DIR}"
    --ledger "${CMAKE_CURRENT_SOURCE_DIR}/docs/original-runtime-dependency-ledger.tsv")
  set_tests_properties(original_process_dependency_ledger PROPERTIES LABELS "original-process;source-identity")
  add_test(NAME original_process_dependency_controls COMMAND "${Python3_EXECUTABLE}" -m unittest
    tests.original_runtime.test_dependency_controls)
  set_tests_properties(original_process_dependency_controls PROPERTIES
    LABELS "original-process;source-identity"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
  add_test(NAME original_process_provider_removal COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/original_runtime/test_provider_removal.py"
    "${CMAKE_CXX_COMPILER}")
  set_tests_properties(original_process_provider_removal PROPERTIES LABELS "original-process;source-identity")

  add_test(NAME original_process_identity_allocator COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_m26_original_identity.py"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-process-allocator.link.map"
    --executable "$<TARGET_FILE:original_process_allocator_tests>"
    --target zh_original_process
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/System/GameMemory.cpp"
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/System/MemoryInit.cpp"
    --symbol "initMemoryManager()"
    --witness "original-process allocator: ok"
    --argument defaults)
  set_tests_properties(original_process_identity_allocator PROPERTIES LABELS "original-process;source-identity")
  add_test(NAME original_process_identity_strings COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_m26_original_identity.py"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-process-strings.link.map"
    --executable "$<TARGET_FILE:original_process_string_tests>"
    --target zh_original_process
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/System/AsciiString.cpp"
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/System/UnicodeString.cpp"
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/version.cpp"
    --symbol "AsciiString::ensureUniqueBufferOfSize"
    --symbol "UnicodeString::concat"
    --symbol "Version::getVersionNumber()"
    --witness "original-process strings: ok")
  set_tests_properties(original_process_identity_strings PROPERTIES LABELS "original-process;source-identity")
  add_test(NAME original_process_identity_fpu COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_m26_original_identity.py"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-process-fpu.link.map"
    --executable "$<TARGET_FILE:original_process_fpu_tests>"
    --target zh_original_process
    --source "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/System/FPUControl.cpp"
    --symbol "setFPMode()"
    --witness "original-process fpu: ok")
  set_tests_properties(original_process_identity_fpu PROPERTIES LABELS "original-process;source-identity")
  add_test(NAME m0_bootstrap_smoke COMMAND zh_main --bootstrap-smoke)
  set_tests_properties(m0_bootstrap_smoke PROPERTIES LABELS foundation PASS_REGULAR_EXPRESSION "asset-free bootstrap graph: ok")
  add_test(NAME m0_bootstrap_rejects_gameplay COMMAND zh_main)
  set_tests_properties(m0_bootstrap_rejects_gameplay PROPERTIES LABELS foundation WILL_FAIL TRUE)
  add_test(NAME m0_manifest_inventory COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/legacy_manifest_inventory.py" --check)
  set_tests_properties(m0_manifest_inventory PROPERTIES LABELS foundation)
  add_test(NAME m0_manifest_inventory_negative COMMAND "${Python3_EXECUTABLE}" -m unittest tests.m0.test_legacy_manifest_inventory.LegacyManifestInventoryTest.test_incomplete_inventory_is_rejected)
  set_tests_properties(m0_manifest_inventory_negative PROPERTIES LABELS foundation WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
  add_test(NAME m0_unsupported_architecture_probe COMMAND "${CMAKE_COMMAND}"
    -DZH_PROBE_SCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/tests/m0/probe_architecture.cmake
    -DZH_PROBE_ARGUMENT=-DZH_PROBE_PROCESSOR=aarch64
    "-DZH_EXPECTED=Unsupported architecture 'aarch64'"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/m0/expect_probe_failure.cmake")
  set_tests_properties(m0_unsupported_architecture_probe PROPERTIES LABELS foundation)
  add_test(NAME m0_missing_glslc_probe COMMAND "${CMAKE_COMMAND}"
    -DZH_PROBE_SCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/tests/m0/probe_glslc.cmake
    -DZH_PROBE_ARGUMENT=-DZH_DEPENDENCY_PROBE_EMPTY=ON
    "-DZH_EXPECTED=Missing required program 'glslc'"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/m0/expect_probe_failure.cmake")
  set_tests_properties(m0_missing_glslc_probe PROPERTIES LABELS foundation)

  add_test(NAME original_source_classification COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/original_source_classification.py" --check)
  set_tests_properties(original_source_classification PROPERTIES LABELS "original-support;source-identity")
  add_test(NAME original_source_classification_negative COMMAND "${Python3_EXECUTABLE}" -m unittest
    tests.original_support.test_source_classification)
  set_tests_properties(original_source_classification_negative PROPERTIES
    LABELS "original-support;source-identity"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

  add_test(NAME renderer_inventory COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/renderer_inventory.py" --check)
  set_tests_properties(renderer_inventory PROPERTIES LABELS renderer-contract)
  add_test(NAME renderer_inventory_negative COMMAND "${Python3_EXECUTABLE}" -m unittest tests.m2.test_renderer_inventory)
  set_tests_properties(renderer_inventory_negative PROPERTIES LABELS renderer-contract WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
  add_test(NAME renderer_public_headers COMMAND "${Python3_EXECUTABLE}" -m unittest tests.m2.test_renderer_public_headers)
  set_tests_properties(renderer_public_headers PROPERTIES LABELS renderer-contract WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

  add_executable(renderer_contract_tests tests/renderer/test_renderer_contract.cpp)
  target_link_libraries(renderer_contract_tests PRIVATE zh_renderer_sdl_gpu)
  add_test(NAME renderer_contract COMMAND renderer_contract_tests)
  set_tests_properties(renderer_contract PROPERTIES LABELS renderer-contract)

  add_executable(renderer_recording_device_tests tests/renderer/test_recording_device.cpp)
  target_link_libraries(renderer_recording_device_tests PRIVATE zh_renderer_sdl_gpu)
  add_test(NAME renderer_recording_device COMMAND renderer_recording_device_tests)
  set_tests_properties(renderer_recording_device PROPERTIES LABELS renderer-contract)

  add_executable(renderer_texture_loader_tests tests/renderer/test_texture_loader.cpp)
  target_link_libraries(renderer_texture_loader_tests PRIVATE zh_renderer_sdl_gpu)
  add_test(NAME renderer_texture_loader COMMAND renderer_texture_loader_tests)
  set_tests_properties(renderer_texture_loader PROPERTIES LABELS renderer-contract)

  add_executable(renderer_dx8_state_cache_tests tests/renderer/test_dx8_state_cache.cpp)
  target_link_libraries(renderer_dx8_state_cache_tests PRIVATE zh_renderer_sdl_gpu)
  add_test(NAME renderer_dx8_state_cache COMMAND renderer_dx8_state_cache_tests)
  set_tests_properties(renderer_dx8_state_cache PROPERTIES LABELS renderer-contract)

  add_executable(renderer_shader_registry_tests tests/renderer/test_shader_registry.cpp)
  target_compile_definitions(renderer_shader_registry_tests PRIVATE
    ZH_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    ZH_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
  )
  target_link_libraries(renderer_shader_registry_tests PRIVATE zh_renderer_sdl_gpu)
  add_dependencies(renderer_shader_registry_tests zh_renderer_shaders)
  add_test(NAME renderer_shader_registry COMMAND renderer_shader_registry_tests)
  set_tests_properties(renderer_shader_registry PROPERTIES LABELS renderer-contract)

  add_executable(renderer_sdl_gpu_device_tests tests/renderer/test_sdl_gpu_device.cpp)
  target_link_libraries(renderer_sdl_gpu_device_tests PRIVATE zh_renderer_sdl_gpu)
  add_test(NAME renderer_sdl_gpu_device COMMAND renderer_sdl_gpu_device_tests)
  set_tests_properties(renderer_sdl_gpu_device PROPERTIES LABELS renderer-contract)

  if(ZH_ENABLE_GPU_TESTS)
    add_executable(renderer_gpu_acceptance tests/renderer/gpu_acceptance.cpp)
    target_compile_definitions(renderer_gpu_acceptance PRIVATE
      ZH_GPU_SHADER_DIR="${CMAKE_CURRENT_BINARY_DIR}/generated/shaders")
    target_link_libraries(renderer_gpu_acceptance PRIVATE zh_renderer_sdl_gpu zh_effects zh_ui zh_world zh_video_ffmpeg SDL3::SDL3)
    add_dependencies(renderer_gpu_acceptance zh_renderer_shaders zh_effect_shaders)
    add_test(NAME renderer_gpu_acceptance COMMAND "${Python3_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/tools/run_validation_clean.py" $<TARGET_FILE:renderer_gpu_acceptance>)
    set_tests_properties(renderer_gpu_acceptance PROPERTIES LABELS "gpu;renderer-contract" TIMEOUT 60 RUN_SERIAL TRUE)
    add_test(NAME renderer_gpu_validation_layer COMMAND "${Python3_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_vulkan_validation.py")
    set_tests_properties(renderer_gpu_validation_layer PROPERTIES LABELS gpu TIMEOUT 30 RUN_SERIAL TRUE)
  endif()

  add_test(NAME renderer_gpu_validation_probe_unit COMMAND "${Python3_EXECUTABLE}" -m unittest
    tests.m14.test_validation_layer)
  set_tests_properties(renderer_gpu_validation_probe_unit PROPERTIES
    LABELS renderer-contract WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
  add_test(NAME renderer_gpu_validation_output_unit COMMAND "${Python3_EXECUTABLE}" -m unittest
    tests.m14.test_validation_output)
  set_tests_properties(renderer_gpu_validation_output_unit PROPERTIES
    LABELS renderer-contract WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

  add_executable(ui_renderer_tests tests/ui/test_ui_renderer.cpp)
  target_link_libraries(ui_renderer_tests PRIVATE zh_ui)
  add_test(NAME ui_renderer COMMAND ui_renderer_tests)
  set_tests_properties(ui_renderer PROPERTIES LABELS "renderer-contract;ui")

  add_executable(ui_font_tests tests/ui/test_ui_font.cpp)
  target_link_libraries(ui_font_tests PRIVATE zh_ui)
  add_test(NAME ui_font COMMAND ui_font_tests)
  set_tests_properties(ui_font PROPERTIES LABELS ui)

  add_executable(ui_session_tests tests/ui/test_ui_session.cpp)
  target_compile_definitions(ui_session_tests PRIVATE ZH_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
  target_link_libraries(ui_session_tests PRIVATE zh_ui)
  add_test(NAME ui_session COMMAND ui_session_tests)
  set_tests_properties(ui_session PROPERTIES LABELS "platform;renderer-contract;ui")

  add_executable(audio_miniaudio_dependency_tests tests/audio/test_miniaudio_dependency.cpp)
  target_link_libraries(audio_miniaudio_dependency_tests PRIVATE zh_audio_miniaudio)
  add_test(NAME audio_miniaudio_dependency COMMAND audio_miniaudio_dependency_tests)
  set_tests_properties(audio_miniaudio_dependency PROPERTIES LABELS audio)

  add_test(NAME audio_miniaudio_provenance COMMAND "${Python3_EXECUTABLE}" -m unittest
    tests.audio.test_miniaudio_provenance)
  set_tests_properties(audio_miniaudio_provenance PROPERTIES
    LABELS audio
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )

  add_executable(audio_manager_tests tests/audio/test_audio_manager.cpp)
  target_link_libraries(audio_manager_tests PRIVATE zh_audio_miniaudio)
  add_test(NAME audio_manager COMMAND audio_manager_tests)
  set_tests_properties(audio_manager PROPERTIES LABELS audio)

  add_executable(audio_acceptance_tests tests/audio/test_audio_acceptance.cpp)
  target_compile_definitions(audio_acceptance_tests PRIVATE ZH_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
  target_link_libraries(audio_acceptance_tests PRIVATE zh_audio_miniaudio)
  add_test(NAME audio_acceptance COMMAND audio_acceptance_tests)
  set_tests_properties(audio_acceptance PROPERTIES LABELS audio TIMEOUT 20)

  add_executable(video_decoder_tests tests/video/test_video_decoder.cpp)
  target_link_libraries(video_decoder_tests PRIVATE zh_video_ffmpeg)
  add_test(NAME video_decoder COMMAND video_decoder_tests)
  set_tests_properties(video_decoder PROPERTIES LABELS video)

  add_executable(video_player_tests tests/video/test_video_player.cpp)
  target_link_libraries(video_player_tests PRIVATE zh_video_ffmpeg zh_renderer_sdl_gpu)
  add_test(NAME video_player COMMAND video_player_tests)
  set_tests_properties(video_player PROPERTIES LABELS "video;renderer-contract;audio")

  add_executable(video_acceptance_tests tests/video/test_video_acceptance.cpp)
  target_link_libraries(video_acceptance_tests PRIVATE zh_video_ffmpeg)
  add_test(NAME video_acceptance COMMAND video_acceptance_tests)
  set_tests_properties(video_acceptance PROPERTIES LABELS video)

  add_executable(world_scene_tests tests/world/test_world_scene.cpp)
  target_link_libraries(world_scene_tests PRIVATE zh_world)
  add_test(NAME world_scene COMMAND world_scene_tests)
  set_tests_properties(world_scene PROPERTIES LABELS renderer-contract)

  add_executable(world_recorder_tests tests/world/test_world_recorder.cpp)
  target_link_libraries(world_recorder_tests PRIVATE zh_world)
  add_test(NAME world_recorder COMMAND world_recorder_tests)
  set_tests_properties(world_recorder PROPERTIES LABELS renderer-contract)

  add_executable(world_corpus_tests tests/world/test_world_corpus.cpp)
  target_compile_definitions(world_corpus_tests PRIVATE ZH_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
  target_link_libraries(world_corpus_tests PRIVATE zh_world)
  add_test(NAME world_corpus COMMAND world_corpus_tests)
  set_tests_properties(world_corpus PROPERTIES LABELS renderer-contract)

  add_executable(effects_registry_tests tests/effects/test_effect_registry.cpp)
  target_compile_definitions(effects_registry_tests PRIVATE
    ZH_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    ZH_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
  )
  target_link_libraries(effects_registry_tests PRIVATE zh_effects)
  add_dependencies(effects_registry_tests zh_renderer_shaders zh_effect_shaders)
  add_test(NAME effects_registry COMMAND effects_registry_tests)
  set_tests_properties(effects_registry PROPERTIES LABELS renderer-contract)

  add_executable(effects_recorder_tests tests/effects/test_effect_recorder.cpp)
  target_compile_definitions(effects_recorder_tests PRIVATE ZH_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
  target_link_libraries(effects_recorder_tests PRIVATE zh_effects)
  add_test(NAME effects_recorder COMMAND effects_recorder_tests)
  set_tests_properties(effects_recorder PROPERTIES LABELS renderer-contract)

  add_executable(foundation_abi_codec_tests tests/foundation/test_abi_codec.cpp)
  target_link_libraries(foundation_abi_codec_tests PRIVATE zh_wwsupport)
  add_test(NAME foundation_abi_codec COMMAND foundation_abi_codec_tests)
  set_tests_properties(foundation_abi_codec PROPERTIES LABELS foundation)

  add_executable(original_wwsupport_tests tests/original_support/test_original_wwsupport.cpp)
  target_link_libraries(original_wwsupport_tests PRIVATE zh_wwsupport)
  target_link_options(original_wwsupport_tests PRIVATE
    "-Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/original-wwsupport.link.map")
  add_test(NAME original_wwsupport_runtime COMMAND original_wwsupport_tests)
  set_tests_properties(original_wwsupport_runtime PROPERTIES LABELS "original-support;source-identity")
  add_test(NAME original_wwsupport_identity COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_original_identity.py"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-wwsupport.link.map"
    --executable "$<TARGET_FILE:original_wwsupport_tests>"
    --witness "original-support runtime provider=GeneralsMD WWLib/WWMath/WWSaveLoad"
    --required "GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/FastAllocator.cpp"
    --required "GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/chunkio.cpp"
    --required "GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/gcd_lcm.cpp"
    --required "GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/nstrdup.cpp"
    --required "GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/ramfile.cpp"
    --required "GeneralsMD/Code/Libraries/Source/WWVegas/WWMath/tri.cpp"
    --required "GeneralsMD/Code/Libraries/Source/WWVegas/WWSaveLoad/pointerremap.cpp")
  set_tests_properties(original_wwsupport_identity PROPERTIES LABELS "original-support;source-identity")

  add_executable(foundation_platform_tests tests/foundation/test_platform.cpp)
  target_link_libraries(foundation_platform_tests PRIVATE zh_os_posix)
  add_test(NAME foundation_platform COMMAND foundation_platform_tests)
  set_tests_properties(foundation_platform PROPERTIES LABELS foundation)

  add_executable(foundation_compression_tests tests/foundation/test_compression.cpp)
  target_link_libraries(foundation_compression_tests PRIVATE zh_compression)
  add_test(NAME foundation_compression COMMAND foundation_compression_tests)
  set_tests_properties(foundation_compression PROPERTIES LABELS foundation)

  add_executable(original_compression_tests tests/original_support/test_original_compression.cpp)
  target_link_libraries(original_compression_tests PRIVATE zh_compression)
  target_link_options(original_compression_tests PRIVATE
    "-Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/original-compression.link.map")
  add_test(NAME original_compression_runtime COMMAND original_compression_tests)
  set_tests_properties(original_compression_runtime PROPERTIES LABELS "original-support;source-identity")
  add_test(NAME original_compression_identity COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/check_original_identity.py"
    --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    --link-map "${CMAKE_CURRENT_BINARY_DIR}/original-compression.link.map"
    --executable "$<TARGET_FILE:original_compression_tests>"
    --required "GeneralsMD/Code/Libraries/Source/Compression/EAC/refabout.cpp"
    --required "GeneralsMD/Code/Libraries/Source/Compression/EAC/refdecode.cpp"
    --required "GeneralsMD/Code/Libraries/Source/Compression/EAC/refencode.cpp")
  set_tests_properties(original_compression_identity PROPERTIES LABELS "original-support;source-identity")
  add_test(NAME original_compression_identity_negative COMMAND "${Python3_EXECUTABLE}" -m unittest
    tests.original_support.test_original_identity)
  set_tests_properties(original_compression_identity_negative PROPERTIES
    LABELS "original-support;source-identity"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

  add_executable(headless_runtime_tests
    tests/headless/test_headless_runtime.cpp
    src/headless/runtime.cpp
  )
  target_link_libraries(headless_runtime_tests PRIVATE
    zh_os_posix zh_null_platform zh_null_renderer zh_null_audio zh_null_video zh_lan
  )
  add_test(NAME headless_runtime COMMAND headless_runtime_tests)
  set_tests_properties(headless_runtime PROPERTIES LABELS headless)

  add_test(NAME headless_process_smoke COMMAND zh_main --headless --ticks 2
    --state-dir "${CMAKE_CURRENT_BINARY_DIR}/headless-process-smoke")
  set_tests_properties(headless_process_smoke PROPERTIES
    LABELS headless
    PASS_REGULAR_EXPRESSION "headless: completed ticks=2"
  )

  add_test(NAME headless_process_isolation COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/headless/test_process_isolation.py"
    --executable "$<TARGET_FILE:zh_main>")
  set_tests_properties(headless_process_isolation PROPERTIES LABELS headless TIMEOUT 30)

  add_executable(lan_protocol_tests tests/lan/test_protocol.cpp)
  target_link_libraries(lan_protocol_tests PRIVATE zh_lan)
  add_test(NAME lan_protocol COMMAND lan_protocol_tests)
  set_tests_properties(lan_protocol PROPERTIES LABELS lan)

  add_executable(lan_virtual_tests tests/lan/test_virtual_transport.cpp)
  target_link_libraries(lan_virtual_tests PRIVATE zh_lan)
  add_test(NAME lan_virtual COMMAND lan_virtual_tests)
  set_tests_properties(lan_virtual PROPERTIES LABELS "lan;headless")

  add_executable(lan_posix_udp_tests tests/lan/test_posix_udp.cpp)
  target_link_libraries(lan_posix_udp_tests PRIVATE zh_lan)
  add_test(NAME lan_posix_udp COMMAND lan_posix_udp_tests)
  set_tests_properties(lan_posix_udp PROPERTIES LABELS "lan;headless" TIMEOUT 10)

  add_test(NAME lan_two_processes COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/lan/test_two_processes.py"
    --executable "$<TARGET_FILE:zh_main>")
  set_tests_properties(lan_two_processes PROPERTIES LABELS "lan;headless" TIMEOUT 20 RUN_SERIAL TRUE)

  add_executable(platform_sdl_event_tests tests/platform/test_sdl_events.cpp)
  target_link_libraries(platform_sdl_event_tests PRIVATE zh_platform_sdl)
  add_test(NAME platform_sdl_events COMMAND platform_sdl_event_tests)
  set_tests_properties(platform_sdl_events PROPERTIES LABELS platform)

  add_executable(platform_sdl_window_tests tests/platform/test_sdl_window.cpp)
  target_link_libraries(platform_sdl_window_tests PRIVATE zh_platform_sdl)
  add_test(NAME platform_sdl_window_errors COMMAND platform_sdl_window_tests)
  set_tests_properties(platform_sdl_window_errors PROPERTIES LABELS platform)

  add_executable(zh_platform_smoke tests/platform/platform_smoke.cpp)
  target_link_libraries(zh_platform_smoke PRIVATE zh_platform_sdl)

  add_executable(data_config_tests tests/data/test_data_config.cpp)
  target_link_libraries(data_config_tests PRIVATE zh_data)
  add_test(NAME data_config COMMAND data_config_tests)
  set_tests_properties(data_config PROPERTIES LABELS data)

  add_executable(data_vfs_tests tests/data/test_vfs.cpp)
  target_link_libraries(data_vfs_tests PRIVATE zh_data)
  add_test(NAME data_vfs COMMAND data_vfs_tests)
  set_tests_properties(data_vfs PROPERTIES LABELS data)

  add_executable(data_inventory_tests tests/data/test_inventory.cpp)
  target_link_libraries(data_inventory_tests PRIVATE zh_data)
  add_test(NAME data_inventory COMMAND data_inventory_tests)
  set_tests_properties(data_inventory PROPERTIES LABELS data)

  add_executable(persistence_save_tests tests/persistence/test_save.cpp)
  target_link_libraries(persistence_save_tests PRIVATE zh_persistence)
  add_test(NAME persistence_save COMMAND persistence_save_tests)
  set_tests_properties(persistence_save PROPERTIES LABELS "foundation;determinism")

  add_executable(persistence_replay_tests tests/persistence/test_replay.cpp)
  target_link_libraries(persistence_replay_tests PRIVATE zh_persistence)
  add_test(NAME persistence_replay COMMAND persistence_replay_tests)
  set_tests_properties(persistence_replay PROPERTIES LABELS "foundation;determinism")

  add_executable(determinism_scenario_tests tests/determinism/test_scenario.cpp)
  target_link_libraries(determinism_scenario_tests PRIVATE zh_simulation)
  add_test(NAME determinism_scenario COMMAND determinism_scenario_tests
    --emit-result "${CMAKE_CURRENT_BINARY_DIR}/determinism-result.txt")
  set_tests_properties(determinism_scenario PROPERTIES LABELS determinism)
  add_test(NAME determinism_report_comparison COMMAND "${Python3_EXECUTABLE}" -m unittest
    tests.determinism.test_compare_presets)
  set_tests_properties(determinism_report_comparison PROPERTIES
    LABELS determinism
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )

  add_executable(singleplayer_session_tests tests/singleplayer/test_session.cpp)
  target_compile_definitions(singleplayer_session_tests PRIVATE ZH_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}")
  target_link_libraries(singleplayer_session_tests PRIVATE zh_singleplayer)
  add_test(NAME singleplayer_session COMMAND singleplayer_session_tests)
  set_tests_properties(singleplayer_session PROPERTIES LABELS "singleplayer;determinism;foundation")

  add_executable(singleplayer_game_flow_tests tests/singleplayer/test_game_flow.cpp)
  target_compile_definitions(singleplayer_game_flow_tests PRIVATE ZH_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}")
  target_link_libraries(singleplayer_game_flow_tests PRIVATE zh_singleplayer zh_renderer_sdl_gpu)
  add_test(NAME singleplayer_game_flow COMMAND singleplayer_game_flow_tests)
  set_tests_properties(singleplayer_game_flow PROPERTIES LABELS "singleplayer;ui;renderer-contract;audio;video;determinism")

  add_executable(singleplayer_stress_tests tests/singleplayer/test_acceptance.cpp)
  target_compile_definitions(singleplayer_stress_tests PRIVATE ZH_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}")
  target_link_libraries(singleplayer_stress_tests PRIVATE zh_singleplayer)
  add_test(NAME singleplayer_stress COMMAND singleplayer_stress_tests)
  set_tests_properties(singleplayer_stress PROPERTIES LABELS "singleplayer;determinism;long" TIMEOUT 60)

  add_test(NAME singleplayer_process COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/singleplayer/test_process.py" --executable "$<TARGET_FILE:zh_main>")
  set_tests_properties(singleplayer_process PROPERTIES LABELS "singleplayer;process" TIMEOUT 60)

  if(ZH_ENABLE_RETAIL_TESTS)
    if(NOT IS_ABSOLUTE "${ZH_RETAIL_ZH_DATA}" OR NOT IS_ABSOLUTE "${ZH_RETAIL_GENERALS_DATA}" OR ZH_RETAIL_LANGUAGE STREQUAL "")
      message(FATAL_ERROR "Retail tests require absolute ZH_RETAIL_ZH_DATA, absolute ZH_RETAIL_GENERALS_DATA, and ZH_RETAIL_LANGUAGE")
    endif()
    add_test(NAME retail_data_verification COMMAND zh_main --verify-data
      --zh-data "${ZH_RETAIL_ZH_DATA}" --generals-data "${ZH_RETAIL_GENERALS_DATA}" --language "${ZH_RETAIL_LANGUAGE}")
    set_tests_properties(retail_data_verification PROPERTIES LABELS data
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      PASS_REGULAR_EXPRESSION "devices skipped;corpus verification ok")

    add_executable(audio_corpus_tests tests/audio/test_audio_corpus.cpp)
    target_link_libraries(audio_corpus_tests PRIVATE zh_audio_miniaudio)
    add_test(NAME audio_corpus COMMAND audio_corpus_tests
      "${ZH_RETAIL_ZH_DATA}" "${ZH_RETAIL_GENERALS_DATA}" "${ZH_RETAIL_LANGUAGE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/data/corpus/english-manifest.tsv")
    set_tests_properties(audio_corpus PROPERTIES LABELS audio
      PASS_REGULAR_EXPRESSION "audio corpus verification ok")

    add_executable(video_corpus_tests tests/video/test_video_corpus.cpp)
    target_link_libraries(video_corpus_tests PRIVATE zh_video_ffmpeg)
    add_test(NAME video_corpus COMMAND video_corpus_tests
      "${ZH_RETAIL_ZH_DATA}" "${ZH_RETAIL_GENERALS_DATA}" "${ZH_RETAIL_LANGUAGE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/data/corpus/video-variants.tsv")
    set_tests_properties(video_corpus PROPERTIES LABELS video TIMEOUT 60
      PASS_REGULAR_EXPRESSION "video corpus verification ok")

    add_executable(singleplayer_retail_integration_tests tests/singleplayer/test_retail_acceptance.cpp)
    target_compile_definitions(singleplayer_retail_integration_tests PRIVATE ZH_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(singleplayer_retail_integration_tests PRIVATE zh_singleplayer zh_renderer_sdl_gpu)
    add_test(NAME singleplayer_retail_integration COMMAND singleplayer_retail_integration_tests
      "${ZH_RETAIL_ZH_DATA}" "${ZH_RETAIL_GENERALS_DATA}" "${ZH_RETAIL_LANGUAGE}")
    set_tests_properties(singleplayer_retail_integration PROPERTIES
      LABELS "singleplayer;data;audio;video;retail" TIMEOUT 120
      PASS_REGULAR_EXPRESSION "single-player retail integration ok")
  endif()
endif()
