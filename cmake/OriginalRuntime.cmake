# Supporting actual original startup owners; no replacement GameLogic fixture.
add_executable(original_service_owners_fixture tests/original/service_owners.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_service_owners_fixture PRIVATE original_data)
foreach(family IN ITEMS lifecycle negative faults)
  add_test(NAME original_service_owners_${family} COMMAND original_service_owners_fixture "${family}")
  set_tests_properties(original_service_owners_${family} PROPERTIES LABELS "original;runtime;bootstrap" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
add_executable(original_subsystems_fixture tests/original/subsystems.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_subsystems_fixture PRIVATE original_data)
foreach(family IN ITEMS lifecycle negative faults)
  add_test(NAME original_subsystems_${family} COMMAND original_subsystems_fixture "${family}")
  set_tests_properties(original_subsystems_${family} PROPERTIES LABELS "original;runtime;bootstrap" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
add_test(NAME original_header_paths COMMAND "${Python3_EXECUTABLE}"
  "${CMAKE_SOURCE_DIR}/tests/toolchain/test_original_header_paths.py")
set_tests_properties(original_header_paths PROPERTIES LABELS "original;graph" TIMEOUT 60)
if(ZH_SANITIZE)
  set(ZH_ENUM_SANITIZER_ARG --sanitize)
else()
  set(ZH_ENUM_SANITIZER_ARG)
endif()
foreach(configuration IN ITEMS default alternate)
  set(ZH_ENUM_VARIANT_ARG)
  if(configuration STREQUAL alternate)
    set(ZH_ENUM_VARIANT_ARG --alternate)
  endif()
  add_test(NAME original_enum_${configuration} COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_SOURCE_DIR}/tests/toolchain/test_original_enum_domains.py"
    --compiler "${CMAKE_CXX_COMPILER}" ${ZH_ENUM_SANITIZER_ARG} ${ZH_ENUM_VARIANT_ARG})
  set_tests_properties(original_enum_${configuration} PROPERTIES LABELS "original;graph;enum" TIMEOUT 60)
endforeach()
add_executable(original_id_fixture tests/original/ids.cpp)
target_link_libraries(original_id_fixture PRIVATE original_core)
add_test(NAME original_id_representations COMMAND original_id_fixture)
set_tests_properties(original_id_representations PROPERTIES LABELS "original;graph" TIMEOUT 60
  ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
find_package(ZLIB REQUIRED)
add_executable(original_list_selection_fixture tests/original/list_selection.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_list_selection_fixture PRIVATE original_core)
foreach(family IN ITEMS functional negative faults)
  add_test(NAME original_list_selection_${family} COMMAND original_list_selection_fixture "${family}")
  set_tests_properties(original_list_selection_${family} PROPERTIES LABELS "original;runtime;widget" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
add_executable(original_message_text_fixture tests/original/message_text.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_message_text_fixture PRIVATE original_core)
foreach(family IN ITEMS functional faults)
  add_test(NAME original_message_text_${family} COMMAND original_message_text_fixture "${family}")
  set_tests_properties(original_message_text_${family} PROPERTIES LABELS "original;runtime;text" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
add_executable(original_graph_headers_fixture tests/original/graph_headers.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_graph_headers_fixture PRIVATE original_core)
foreach(family IN ITEMS matching faults insertion-faults lists)
  add_test(NAME original_graph_headers_${family} COMMAND original_graph_headers_fixture "${family}")
  set_tests_properties(original_graph_headers_${family} PROPERTIES LABELS "original;graph" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
add_executable(original_container_fixture tests/original/containers.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_container_fixture PRIVATE original_core)
foreach(family IN ITEMS functional ordered fault-insert fault-order fault-copy)
  add_test(NAME original_container_${family} COMMAND original_container_fixture "${family}")
  set_tests_properties(original_container_${family} PROPERTIES LABELS "original;graph;container" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
find_package(OpenSSL 3 REQUIRED COMPONENTS Crypto)
find_package(SDL3 REQUIRED CONFIG)
add_library(original_runtime_common STATIC
  "${ZH_CODE}/GameEngine/Source/Common/StatsCollector.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/CommandLine.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/GlobalData.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/GameType.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/OptionPreferences.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/OptionServicePreferences.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/WeaponBonus.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/RTS/MoneyDefinition.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/Snapshot.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/version.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/INI/INIValueParsers.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeInputSettings.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/QuotedPrintable.cpp"
  "${ZH_CODE}/GameEngine/Source/GameNetwork/IPEnumeration.cpp"
  "${ZH_CODE}/GameEngine/Source/GameNetwork/MapCompanionPaths.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeFunctionRegistry.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/UserPreferences.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/Bezier/BezierSegment.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/Bezier/BezFwdIterator.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/DiscreteCircle.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeCalendar.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeClock.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeLODProbe.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeWarningBox.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeSourceStrings.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeMapMetadata.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/INI/INIMapCache.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeWellKnownKeys.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeModuleData.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeAudioEventData.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeThingTemplateData.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/Geometry.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/KindOf.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/Dict.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/CachedFileInputStream.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/DataChunkInput.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/DataChunk.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeMapCompression.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeMemoryProfiles.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeConversionCache.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/NameKeyGenerator.cpp")
target_link_libraries(original_runtime_common PUBLIC original_data ZLIB::ZLIB OpenSSL::Crypto SDL3::SDL3)
target_include_directories(original_runtime_common PRIVATE
  "${ZH_CODE}/Libraries/Source/WWVegas"
  "${ZH_CODE}/Libraries/Source/WWVegas/WWLib"
  "${ZH_CODE}/Libraries/Source/GameSpy"
  "${ZH_CODE}/Libraries/Source/Compression")
target_compile_definitions(original_runtime_common PRIVATE _OPERATOR_NEW_DEFINED_)
# The actual default transfer adapters require gameplay definition owners.
# Configuration consumes their typed abstract contract, not a replacement.
add_library(original_transfer STATIC
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeTransferServices.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/Xfer.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/XferSave.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/XferLoad.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/XferCRC.cpp")
target_link_libraries(original_transfer PUBLIC original_runtime_common)
target_include_directories(original_transfer PRIVATE
  "${ZH_CODE}/Libraries/Source/WWVegas"
  "${ZH_CODE}/Libraries/Source/WWVegas/WWLib")
target_compile_definitions(original_transfer PRIVATE _OPERATOR_NEW_DEFINED_)
# Actual parent/registry sources, without substitute gameplay callback providers.
add_library(original_templates STATIC
  "${ZH_CODE}/GameEngine/Source/Common/Thing/Thing.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/Thing/ThingTemplate.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/Thing/ThingFactory.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/Thing/Module.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/Thing/ModuleFactory.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/Thing/DrawModule.cpp")
target_link_libraries(original_templates PUBLIC original_runtime_common)
target_include_directories(original_templates PRIVATE
  "${ZH_CODE}/Libraries/Source/WWVegas"
  "${ZH_CODE}/Libraries/Source/WWVegas/WWLib"
  "${ZH_CODE}/Libraries/Source/GameSpy"
  "${ZH_CODE}/Libraries/Source/Compression")
target_compile_definitions(original_templates PRIVATE _OPERATOR_NEW_DEFINED_)
target_compile_options(original_templates PRIVATE -Wno-unknown-pragmas -Werror=return-type -ffp-contract=off)
# Their archive is compilation evidence, not complete executable acceptance.
add_library(original_bootstrap STATIC
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeStartupPublications.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/GameEngine.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/GameMain.cpp"
  "${ZH_CODE}/GameEngine/Source/GameClient/GameClient.cpp"
  "${ZH_CODE}/GameEngine/Source/GameClient/MapUtil.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/MessageStream.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/GameMessageValues.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/SaveGame/GameStateMap.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/SaveGame/GameState.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/Recorder.cpp"
  "${ZH_CODE}/GameEngine/Source/GameNetwork/GameInfo.cpp"
  "${ZH_CODE}/GameEngine/Source/GameNetwork/GameInfoPresentation.cpp"
  "${ZH_CODE}/GameEngine/Source/GameNetwork/GameSlotVisibility.cpp"
  "${ZH_CODE}/GameEngine/Source/GameNetwork/GameInfoSerialization.cpp"
  "${ZH_CODE}/GameEngine/Source/GameNetwork/GameMessageParser.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/SubsystemInterface.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/INI/INI.cpp")
target_link_libraries(original_bootstrap PUBLIC original_transfer original_templates)
target_include_directories(original_bootstrap PRIVATE
  "${ZH_CODE}/Libraries/Source/WWVegas"
  "${ZH_CODE}/Libraries/Source/WWVegas/WWLib"
  "${ZH_CODE}/Libraries/Source/GameSpy"
  "${ZH_CODE}/Libraries/Source/Compression")
target_compile_definitions(original_bootstrap PRIVATE _OPERATOR_NEW_DEFINED_)
target_compile_options(original_bootstrap PRIVATE -Wno-unknown-pragmas -Werror=return-type -ffp-contract=off)
# All original Common INI providers, with already-owned definitions excluded.
file(GLOB ZH_ORIGINAL_INI_SOURCES CONFIGURE_DEPENDS
  "${ZH_CODE}/GameEngine/Source/Common/INI/*.cpp")
list(REMOVE_ITEM ZH_ORIGINAL_INI_SOURCES
  "${ZH_CODE}/GameEngine/Source/Common/INI/INI.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/INI/INICore.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/INI/INIMapCache.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/INI/INIMappedImage.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/INI/INIValueParsers.cpp")
add_library(original_definitions STATIC ${ZH_ORIGINAL_INI_SOURCES})
target_link_libraries(original_definitions PUBLIC original_runtime_common)
target_include_directories(original_definitions PRIVATE
  "${ZH_CODE}/Libraries/Source/WWVegas"
  "${ZH_CODE}/Libraries/Source/WWVegas/WWLib"
  "${ZH_CODE}/Libraries/Source/GameSpy"
  "${ZH_CODE}/Libraries/Source/Compression")
target_compile_definitions(original_definitions PRIVATE _OPERATOR_NEW_DEFINED_)
target_compile_options(original_definitions PRIVATE -Wno-unknown-pragmas -Werror=return-type -ffp-contract=off)
target_link_libraries(original_bootstrap PUBLIC original_definitions)
# Actual original simulation/module translation units, not replacement fixtures.
# Linking startup and accepting those owners remains a separate N2 gate.
file(GLOB_RECURSE ZH_ORIGINAL_LOGIC_SOURCES CONFIGURE_DEPENDS
  "${ZH_CODE}/GameEngine/Source/GameLogic/*.cpp")
list(REMOVE_ITEM ZH_ORIGINAL_LOGIC_SOURCES
  "${ZH_CODE}/GameEngine/Source/GameLogic/System/FPUControl.cpp")
add_library(original_logic STATIC ${ZH_ORIGINAL_LOGIC_SOURCES})
target_link_libraries(original_logic PUBLIC original_runtime_common)
target_include_directories(original_logic PRIVATE
  "${ZH_CODE}/Libraries/Source/WWVegas"
  "${ZH_CODE}/Libraries/Source/WWVegas/WWLib"
  "${ZH_CODE}/Libraries/Source/GameSpy"
  "${ZH_CODE}/Libraries/Source/Compression")
target_compile_definitions(original_logic PRIVATE _OPERATOR_NEW_DEFINED_)
target_compile_options(original_logic PRIVATE -Wno-unknown-pragmas -Werror=return-type -ffp-contract=off)
# Retained actual Common gameplay owners exposed by the whole-root link census.
# No platform playback, excluded tools/online service or replacement globals.
set(ZH_ORIGINAL_GAMEPLAY_COMMON
  Audio/AudioEventRTS.cpp Audio/AudioRequest.cpp Audio/DynamicAudioEventInfo.cpp
  Audio/GameAudio.cpp Audio/GameMusic.cpp Audio/GameSounds.cpp
  RTS/AcademyStats.cpp RTS/ActionManager.cpp RTS/Energy.cpp RTS/Handicap.cpp
  RTS/MissionStats.cpp RTS/Money.cpp RTS/Player.cpp RTS/PlayerList.cpp
  RTS/PlayerTemplate.cpp RTS/ProductionPrerequisite.cpp RTS/ResourceGatheringManager.cpp
  RTS/Science.cpp RTS/ScoreKeeper.cpp RTS/SpecialPower.cpp RTS/Team.cpp RTS/TunnelTracker.cpp
  BitFlags.cpp DamageFX.cpp GameLOD.cpp Language.cpp MultiplayerSettings.cpp PartitionSolver.cpp
  System/BuildAssistant.cpp System/FunctionLexicon.cpp
  SkirmishBattleHonors.cpp StateMachine.cpp TerrainTypes.cpp
  System/DisabledTypes.cpp System/GameCommon.cpp
  System/ObjectStatusTypes.cpp System/Radar.cpp System/Upgrade.cpp)
list(TRANSFORM ZH_ORIGINAL_GAMEPLAY_COMMON PREPEND "${ZH_CODE}/GameEngine/Source/Common/")
add_library(original_gameplay_common STATIC ${ZH_ORIGINAL_GAMEPLAY_COMMON})
# Actual CPU map owners extracted from the original device-owned translation unit.
target_sources(original_gameplay_common PRIVATE
  "${ZH_CODE}/GameEngine/Source/Common/System/NativeOriginalStatsSource.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/FunctionLexiconTables.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/MapObject.cpp"
  "${ZH_CODE}/GameEngine/Source/Common/System/MapObjectTeams.cpp")
target_link_libraries(original_gameplay_common PUBLIC original_runtime_common)
target_include_directories(original_gameplay_common PRIVATE
  "${ZH_CODE}/Libraries/Source/WWVegas"
  "${ZH_CODE}/Libraries/Source/WWVegas/WWLib"
  "${ZH_CODE}/Libraries/Source/GameSpy"
  "${ZH_CODE}/Libraries/Source/Compression")
target_compile_definitions(original_gameplay_common PRIVATE _OPERATOR_NEW_DEFINED_)
target_compile_options(original_gameplay_common PRIVATE -Wno-unknown-pragmas -Werror=return-type -ffp-contract=off)
target_link_libraries(original_bootstrap PUBLIC original_gameplay_common)
# Actual logical client providers shared with headless startup/simulation.
# Device rendering, online menu callbacks and platform input remain separate.
set(ZH_ORIGINAL_LOGICAL_CLIENT
  Color.cpp Credits.cpp Display.cpp DisplayString.cpp DisplayStringManager.cpp
  Drawable.cpp DrawGroupInfo.cpp FXList.cpp GameClientDispatch.cpp GlobalLanguage.cpp
  LanguageFilter.cpp Line2D.cpp ParabolicEase.cpp RadiusDecal.cpp SelectionInfo.cpp Snow.cpp Statistics.cpp
  View.cpp Water.cpp VideoPlayer.cpp Terrain/TerrainVisual.cpp Input/Keyboard.cpp Input/Mouse.cpp
  System/Anim2D.cpp System/CampaignManager.cpp System/Image.cpp System/ParticleSys.cpp
  System/RayEffect.cpp Terrain/TerrainRoads.cpp
  Drawable/Update/AnimatedParticleSysBoneClientUpdate.cpp
  Drawable/Update/BeaconClientUpdate.cpp Drawable/Update/SwayClientUpdate.cpp
  Eva.cpp InGameUI.cpp
  MessageStream/WindowXlat.cpp MessageStream/PlaceEventTranslator.cpp
  MessageStream/HotKey.cpp MessageStream/CommandXlat.cpp MessageStream/HintSpy.cpp
  MessageStream/MetaEvent.cpp MessageStream/LookAtXlat.cpp MessageStream/SelectionXlat.cpp
  MessageStream/GUICommandTranslator.cpp
  GUI/AnimateWindowManager.cpp GUI/WindowLayout.cpp GUI/GameWindow.cpp
  GUI/GameWindowManager.cpp GUI/GameWindowManagerScript.cpp GUI/ChallengeGenerals.cpp
  GUI/GameWindowTransitions.cpp GUI/GameWindowTransitionsStyles.cpp
  GUI/WinInstanceData.cpp GUI/GameWindowGlobal.cpp GUI/ProcessAnimateWindow.cpp
  GUI/WindowVideoManager.cpp GUI/GameFont.cpp GUI/HeaderTemplate.cpp GUI/LoadScreen.cpp
  GUI/Shell/Shell.cpp GUI/Shell/ShellMenuScheme.cpp
  GUI/ControlBar/ControlBar.cpp GUI/ControlBar/ControlBarBeacon.cpp
  GUI/ControlBar/ControlBarCommand.cpp GUI/ControlBar/ControlBarCommandProcessing.cpp
  GUI/ControlBar/ControlBarMultiSelect.cpp GUI/ControlBar/ControlBarOCLTimer.cpp
  GUI/ControlBar/ControlBarObserver.cpp GUI/ControlBar/ControlBarPrintPositions.cpp
  GUI/ControlBar/ControlBarResizer.cpp GUI/ControlBar/ControlBarScheme.cpp
  GUI/ControlBar/ControlBarStructureInventory.cpp GUI/ControlBar/ControlBarUnderConstruction.cpp
  GUI/Gadget/GadgetPushButton.cpp GUI/Gadget/GadgetCheckBox.cpp
  GUI/Gadget/GadgetVerticalSlider.cpp GUI/Gadget/GadgetHorizontalSlider.cpp
  GUI/Gadget/GadgetListBox.cpp GUI/Gadget/GadgetComboBox.cpp
  GUI/Gadget/GadgetStaticText.cpp GUI/Gadget/GadgetProgressBar.cpp
  GUI/Gadget/GadgetTabControl.cpp GUI/Gadget/GadgetTextEntry.cpp
  GUI/Gadget/GadgetRadioButton.cpp
  GUI/GUICallbacks/ControlBarCallback.cpp GUI/GUICallbacks/ControlBarPopupDescription.cpp
  GUI/GUICallbacks/InGameChat.cpp GUI/GUICallbacks/Menus/QuitMenu.cpp)
list(TRANSFORM ZH_ORIGINAL_LOGICAL_CLIENT PREPEND "${ZH_CODE}/GameEngine/Source/GameClient/")
add_library(original_logical_client STATIC ${ZH_ORIGINAL_LOGICAL_CLIENT})
target_sources(original_logical_client PRIVATE
  "${ZH_CODE}/GameEngine/Source/Common/INI/INIMappedImage.cpp"
  "${ZH_CODE}/GameEngine/Source/GameClient/NativeMapPreviewLayout.cpp"
  "${ZH_CODE}/GameEngine/Source/GameClient/NativeMapPreviewStorage.cpp"
  "${ZH_CODE}/GameEngine/Source/GameClient/GUI/NativeMapPreview.cpp"
  "${ZH_CODE}/GameEngine/Source/GameClient/GUI/NativeDiplomacyBriefing.cpp"
  "${ZH_CODE}/GameEngine/Source/GameClient/NativePresentationState.cpp"
  "${ZH_CODE}/GameEngine/Source/GameClient/GUI/DisconnectMenu/DisconnectMenuLifetime.cpp"
  "${ZH_CODE}/GameEngine/Source/GameClient/GUI/GUICallbacks/Diplomacy.cpp"
  "${ZH_CODE}/GameEngine/Source/GameClient/GUI/NativeScoreScreenState.cpp")
target_link_libraries(original_logical_client PUBLIC original_runtime_common)
target_include_directories(original_logical_client PRIVATE
  "${ZH_CODE}/Libraries/Source/WWVegas"
  "${ZH_CODE}/Libraries/Source/WWVegas/WWLib"
  "${ZH_CODE}/Libraries/Source/GameSpy"
  "${ZH_CODE}/Libraries/Source/Compression")
target_compile_definitions(original_logical_client PRIVATE _OPERATOR_NEW_DEFINED_)
target_compile_options(original_logical_client PRIVATE -Wno-unknown-pragmas -Werror=return-type -ffp-contract=off)
target_link_libraries(original_bootstrap PUBLIC original_logical_client)
# Keep actual source providers while allowing standalone CPU fixtures to discard
# genuinely unreferenced presentation methods from a shared original TU.
foreach(owner IN ITEMS original_transfer original_bootstrap original_templates original_definitions
    original_logic original_gameplay_common original_logical_client original_runtime_common
    original_data original_core)
  target_compile_options(${owner} PRIVATE -ffunction-sections -fdata-sections)
endforeach()
add_executable(original_transfer_fixture tests/original/transfer.cpp tests/original/AllocationFault.cpp)
add_executable(original_message_parser_fixture tests/original/message_parser.cpp tests/original/AllocationFault.cpp)
add_executable(original_game_info_fixture tests/original/game_info.cpp tests/original/AllocationFault.cpp)
add_executable(original_map_object_fixture tests/original/map_object.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_map_object_fixture PRIVATE
  "$<LINK_GROUP:RESCAN,original_bootstrap,original_transfer,original_templates,original_definitions,original_logic,original_gameplay_common,original_logical_client,original_runtime_common,original_data,original_core>")
target_link_options(original_map_object_fixture PRIVATE -Wl,--gc-sections)
foreach(family IN ITEMS values references constructor-faults names name-faults pool-failure template-binding)
  add_test(NAME original_map_object_${family} COMMAND original_map_object_fixture "${family}")
  set_tests_properties(original_map_object_${family} PROPERTIES LABELS "original;runtime;map" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
# Explicit diagnostic only. Its expected unresolved providers must not prevent
# configured acceptance targets from building, or be mistaken for runtime tests.
add_executable(original_runtime_link_probe EXCLUDE_FROM_ALL
  tests/toolchain/original_runtime_link_probe.cpp)
target_link_libraries(original_runtime_link_probe PRIVATE
  "$<LINK_GROUP:RESCAN,original_bootstrap,original_transfer,original_templates,original_definitions,original_logic,original_gameplay_common,original_logical_client,original_runtime_common,original_data,original_core>")
target_link_options(original_runtime_link_probe PRIVATE -Wl,--gc-sections)
add_executable(original_seismic_filter_fixture tests/original/seismic_filter.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_seismic_filter_fixture PRIVATE
  "$<LINK_GROUP:RESCAN,original_bootstrap,original_transfer,original_templates,original_definitions,original_logic,original_gameplay_common,original_logical_client,original_runtime_common,original_data,original_core>")
target_link_options(original_seismic_filter_fixture PRIVATE -Wl,--gc-sections)
foreach(family IN ITEMS oracle failures)
  add_test(NAME original_seismic_filter_${family} COMMAND original_seismic_filter_fixture "${family}")
  set_tests_properties(original_seismic_filter_${family} PROPERTIES LABELS "original;runtime;terrain" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
target_link_libraries(original_game_info_fixture PRIVATE
  "$<LINK_GROUP:RESCAN,original_bootstrap,original_transfer,original_templates,original_definitions,original_logic,original_gameplay_common,original_logical_client,original_runtime_common,original_data,original_core>")
target_link_options(original_game_info_fixture PRIVATE -Wl,--gc-sections)
foreach(family IN ITEMS transactions faults setter-faults adoption)
  add_test(NAME original_game_info_${family} COMMAND original_game_info_fixture "${family}")
  set_tests_properties(original_game_info_${family} PROPERTIES LABELS "original;runtime;setup" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
target_link_libraries(original_message_parser_fixture PRIVATE
  "$<LINK_GROUP:RESCAN,original_bootstrap,original_transfer,original_templates,original_definitions,original_logic,original_gameplay_common,original_logical_client,original_runtime_common,original_data,original_core>")
target_link_options(original_message_parser_fixture PRIVATE -Wl,--gc-sections)
foreach(family IN ITEMS values faults text command command_faults)
  add_test(NAME original_message_parser_${family} COMMAND original_message_parser_fixture "${family}")
  set_tests_properties(original_message_parser_${family} PROPERTIES LABELS "original;runtime;replay" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
target_link_libraries(original_transfer_fixture PRIVATE
  "$<LINK_GROUP:RESCAN,original_transfer,original_bootstrap,original_templates,original_definitions,original_logic,original_gameplay_common,original_logical_client,original_runtime_common,original_data,original_core>")
target_link_options(original_transfer_fixture PRIVATE -Wl,--gc-sections)
target_include_directories(original_transfer_fixture PRIVATE
  "${ZH_CODE}/Libraries/Source/WWVegas" "${ZH_CODE}/Libraries/Source/WWVegas/WWLib")
target_compile_definitions(original_transfer_fixture PRIVATE _OPERATOR_NEW_DEFINED_)
foreach(family IN ITEMS wire functional malformed poisoning faults collections masks aggregates io-faults services replay-session replay-session-faults replay-startup)
  add_test(NAME original_transfer_${family} COMMAND original_transfer_fixture "${family}")
  set_tests_properties(original_transfer_${family} PROPERTIES LABELS "original;runtime;transfer" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
add_executable(original_bezier_fixture tests/original/bezier.cpp tests/original/AllocationFault.cpp)
add_executable(original_source_boundaries_fixture tests/original/source_boundaries.cpp tests/original/AllocationFault.cpp)
add_executable(original_metadata_fixture tests/original/metadata.cpp tests/original/AllocationFault.cpp)
add_executable(original_module_data_fixture tests/original/module_data.cpp tests/original/AllocationFault.cpp)
add_executable(original_template_backing_fixture tests/original/template_backing.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_template_backing_fixture PRIVATE original_runtime_common)
foreach(family IN ITEMS values fault-copy fault-assign template-values template-faults)
  add_test(NAME original_template_backing_${family} COMMAND original_template_backing_fixture "${family}")
  set_tests_properties(original_template_backing_${family} PROPERTIES LABELS "original;template-backing" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
target_link_libraries(original_module_data_fixture PRIVATE original_runtime_common)
foreach(family IN ITEMS values base macro fault-base fault-macro keys fault-keys owner-values owner-faults)
  add_test(NAME original_module_data_${family} COMMAND original_module_data_fixture "${family}")
  set_tests_properties(original_module_data_${family} PROPERTIES LABELS "original;module-data" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
target_link_libraries(original_metadata_fixture PRIVATE original_runtime_common)
foreach(family IN ITEMS values parsing rejection fault-insert fault-replace serialization fault-serialization persistence fault-persistence optional-cache fault-optional-cache)
  add_test(NAME original_metadata_${family} COMMAND original_metadata_fixture "${family}")
  set_tests_properties(original_metadata_${family} PROPERTIES LABELS "original;metadata" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
target_link_libraries(original_source_boundaries_fixture PRIVATE original_runtime_common)
target_include_directories(original_source_boundaries_fixture PRIVATE
  "${ZH_CODE}/Libraries/Source/WWVegas" "${ZH_CODE}/Libraries/Source/WWVegas/WWLib")
target_compile_options(original_source_boundaries_fixture PRIVATE -ffp-contract=off)
target_compile_definitions(original_source_boundaries_fixture PRIVATE _OPERATOR_NEW_DEFINED_)
foreach(family IN ITEMS strings spans circles faults math)
  add_test(NAME original_source_${family} COMMAND original_source_boundaries_fixture "${family}")
  set_tests_properties(original_source_${family} PROPERTIES LABELS "original;runtime;source" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
target_link_libraries(original_bezier_fixture PRIVATE original_runtime_common)
foreach(family IN ITEMS evaluate subdivision sampling faults)
  add_test(NAME original_bezier_${family} COMMAND original_bezier_fixture "${family}")
  set_tests_properties(original_bezier_${family} PROPERTIES LABELS "original;runtime;math" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
add_executable(original_calendar_fixture tests/original/calendar.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_calendar_fixture PRIVATE original_runtime_common)
foreach(family IN ITEMS functional faults clocks)
  add_test(NAME original_calendar_${family} COMMAND original_calendar_fixture "${family}")
  set_tests_properties(original_calendar_${family} PROPERTIES LABELS "original;runtime;calendar" TIMEOUT 60
    ENVIRONMENT "TZ=UTC;LC_ALL=C;ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
target_compile_options(original_runtime_common PRIVATE -Wall -Wextra -Wno-unknown-pragmas -ffp-contract=off)
add_executable(original_runtime_fixture tests/original/runtime.cpp tests/original/AllocationFault.cpp)
add_executable(original_storage_fixture tests/original/storage.cpp tests/original/AllocationFault.cpp)
add_executable(original_statistics_fixture tests/original/statistics.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_statistics_fixture PRIVATE
  "$<LINK_GROUP:RESCAN,original_bootstrap,original_transfer,original_templates,original_definitions,original_logic,original_gameplay_common,original_logical_client,original_runtime_common,original_data,original_core>")
target_link_options(original_statistics_fixture PRIVATE -Wl,--gc-sections)
foreach(family IN ITEMS functional negative io-faults reset-faults update-faults end-faults)
  add_test(NAME original_statistics_${family} COMMAND original_statistics_fixture "${family}")
  set_tests_properties(original_statistics_${family} PROPERTIES LABELS "original;runtime;storage;statistics" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
add_executable(original_preferences_fixture tests/original/preferences.cpp tests/original/AllocationFault.cpp)
add_executable(original_configuration_fixture tests/original/configuration.cpp tests/original/AllocationFault.cpp)
add_executable(original_quoted_fixture tests/original/quoted.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_quoted_fixture PRIVATE original_runtime_common)
foreach(family IN ITEMS functional negative faults)
  add_test(NAME original_quoted_${family} COMMAND original_quoted_fixture "${family}")
  set_tests_properties(original_quoted_${family} PROPERTIES LABELS "original;runtime;format" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
target_link_libraries(original_configuration_fixture PRIVATE original_definitions original_logical_client original_gameplay_common)
target_include_directories(original_configuration_fixture PRIVATE
  "${ZH_CODE}/Libraries/Source/WWVegas" "${ZH_CODE}/Libraries/Source/WWVegas/WWLib")
target_compile_definitions(original_configuration_fixture PRIVATE _OPERATOR_NEW_DEFINED_)
foreach(family IN ITEMS functional negative fault-override fault-overwrite fault-constructor platform interfaces values bootstrap webpage locale locale-faults images images-negative images-fault-new images-fault-replace images-fault-collection colors colors-negative colors-faults commands commands-negative commands-faults)
  add_test(NAME original_configuration_${family} COMMAND original_configuration_fixture "${family}")
  set_tests_properties(original_configuration_${family} PROPERTIES LABELS "original;runtime;configuration" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
add_executable(original_function_registry_fixture tests/original/function_registry.cpp tests/original/AllocationFault.cpp)
add_executable(original_lexicon_fixture tests/original/lexicon.cpp tests/original/AllocationFault.cpp)
add_executable(original_map_preview_fixture tests/original/map_preview.cpp tests/original/AllocationFault.cpp)
add_executable(original_diplomacy_fixture tests/original/diplomacy.cpp tests/original/AllocationFault.cpp)
add_executable(original_lod_fixture tests/original/lod.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_lod_fixture PRIVATE
  "$<LINK_GROUP:RESCAN,original_bootstrap,original_transfer,original_templates,original_definitions,original_logic,original_gameplay_common,original_logical_client,original_runtime_common,original_data,original_core>")
target_include_directories(original_lod_fixture PRIVATE
  "${ZH_CODE}/Libraries/Source/WWVegas" "${ZH_CODE}/Libraries/Source/WWVegas/WWLib")
target_compile_definitions(original_lod_fixture PRIVATE _OPERATOR_NEW_DEFINED_)
target_link_options(original_lod_fixture PRIVATE -Wl,--gc-sections)
foreach(family IN ITEMS functional negative native fault-init fault-report fault-storage probes recommend-alloc recommend-io)
  add_test(NAME original_lod_${family} COMMAND original_lod_fixture "${family}")
  set_tests_properties(original_lod_${family} PROPERTIES LABELS "original;runtime;lod" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
add_executable(original_native_warning_fixture tests/original/native_warning.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_native_warning_fixture PRIVATE original_runtime_common)
target_include_directories(original_native_warning_fixture PRIVATE
  "${ZH_CODE}/Libraries/Source/WWVegas" "${ZH_CODE}/Libraries/Source/WWVegas/WWLib")
target_link_options(original_native_warning_fixture PRIVATE -Wl,--gc-sections)
target_compile_definitions(original_native_warning_fixture PRIVATE _OPERATOR_NEW_DEFINED_)
foreach(family IN ITEMS functional negative faults)
  add_test(NAME original_native_warning_${family} COMMAND original_native_warning_fixture "${family}")
  set_tests_properties(original_native_warning_${family} PROPERTIES LABELS "original;runtime;native" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
add_executable(original_animation_fixture tests/original/animation.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_animation_fixture PRIVATE
  "$<LINK_GROUP:RESCAN,original_bootstrap,original_transfer,original_templates,original_definitions,original_logic,original_gameplay_common,original_logical_client,original_runtime_common,original_data,original_core>")
target_link_options(original_animation_fixture PRIVATE -Wl,--gc-sections)
add_test(NAME original_animation_constructor COMMAND original_animation_fixture)
set_tests_properties(original_animation_constructor PROPERTIES LABELS "original;runtime;animation" TIMEOUT 60
  ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
target_link_libraries(original_diplomacy_fixture PRIVATE
  "$<LINK_GROUP:RESCAN,original_bootstrap,original_transfer,original_templates,original_definitions,original_logic,original_gameplay_common,original_logical_client,original_runtime_common,original_data,original_core>")
target_link_options(original_diplomacy_fixture PRIVATE -Wl,--gc-sections)
foreach(family IN ITEMS functional negative globals disconnect fault-append fault-clear fault-attach)
  add_test(NAME original_diplomacy_${family} COMMAND original_diplomacy_fixture "${family}")
  set_tests_properties(original_diplomacy_${family} PROPERTIES LABELS "original;runtime;diplomacy" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
target_link_libraries(original_map_preview_fixture PRIVATE
  "$<LINK_GROUP:RESCAN,original_bootstrap,original_transfer,original_templates,original_definitions,original_logic,original_gameplay_common,original_logical_client,original_runtime_common,original_data,original_core>")
target_link_options(original_map_preview_fixture PRIVATE -Wl,--gc-sections)
foreach(family IN ITEMS geometry negative labels layout-faults copy copy-allocation-faults copy-storage-faults)
  add_test(NAME original_map_preview_${family} COMMAND original_map_preview_fixture "${family}")
  set_tests_properties(original_map_preview_${family} PROPERTIES LABELS "original;runtime;map" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
target_link_libraries(original_lexicon_fixture PRIVATE
  "$<LINK_GROUP:RESCAN,original_bootstrap,original_transfer,original_templates,original_definitions,original_logic,original_gameplay_common,original_logical_client,original_runtime_common,original_data,original_core>")
target_link_options(original_lexicon_fixture PRIVATE -Wl,--gc-sections)
foreach(family IN ITEMS functional negative fault-warm fault-cold fault-device)
  add_test(NAME original_lexicon_${family} COMMAND original_lexicon_fixture "${family}")
  set_tests_properties(original_lexicon_${family} PROPERTIES LABELS "original;runtime;callbacks" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
add_test(NAME original_lexicon_source_tables COMMAND "${Python3_EXECUTABLE}"
  "${CMAKE_SOURCE_DIR}/tests/toolchain/test_original_lexicon_tables.py")
set_tests_properties(original_lexicon_source_tables PROPERTIES LABELS "original;runtime;callbacks" TIMEOUT 60)
add_executable(original_lexicon_default_link_probe EXCLUDE_FROM_ALL tests/toolchain/original_lexicon_default_link_probe.cpp)
target_link_libraries(original_lexicon_default_link_probe PRIVATE
  "$<LINK_GROUP:RESCAN,original_bootstrap,original_transfer,original_templates,original_definitions,original_logic,original_gameplay_common,original_logical_client,original_runtime_common,original_data,original_core>")
target_link_options(original_lexicon_default_link_probe PRIVATE -Wl,--gc-sections)
target_link_libraries(original_function_registry_fixture PRIVATE original_runtime_common)
foreach(family IN ITEMS functional malformed fault-warm fault-cold)
  add_test(NAME original_function_registry_${family} COMMAND original_function_registry_fixture "${family}")
  set_tests_properties(original_function_registry_${family} PROPERTIES LABELS "original;runtime;callbacks" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
target_link_libraries(original_preferences_fixture PRIVATE original_runtime_common)
foreach(family IN ITEMS parsing persistence fault-load fault-write)
  add_test(NAME original_preferences_${family} COMMAND original_preferences_fixture "${family}")
  set_tests_properties(original_preferences_${family} PROPERTIES LABELS "original;runtime;preferences" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
target_link_libraries(original_storage_fixture PRIVATE original_runtime_common)
add_executable(original_mods_fixture tests/original/mods.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_mods_fixture PRIVATE original_runtime_common)
foreach(family IN ITEMS precedence protection faults)
  add_test(NAME original_mods_${family} COMMAND original_mods_fixture "${family}")
  set_tests_properties(original_mods_${family} PROPERTIES LABELS "original;runtime;storage" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
foreach(family IN ITEMS paths atomic io-faults allocation-faults cache cache-faults cache-miss-faults namespace namespace-faults scratch scratch-faults map-identities map-identity-faults)
  add_test(NAME original_storage_${family} COMMAND original_storage_fixture "${family}")
  set_tests_properties(original_storage_${family} PROPERTIES LABELS "original;runtime;storage" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
add_executable(original_startup_services_fixture tests/original/startup_services.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_startup_services_fixture PRIVATE original_runtime_common)
foreach(family IN ITEMS profiles profile-faults filter filter-faults timestamps)
  add_test(NAME original_startup_${family} COMMAND original_startup_services_fixture "${family}")
  set_tests_properties(original_startup_${family} PROPERTIES LABELS "original;runtime;services" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
target_link_libraries(original_runtime_fixture PRIVATE original_runtime_common)
foreach(family IN ITEMS names names-exact-fault names-lower-fault names-transactions names-transaction-exact-fault names-transaction-lower-fault floating-point dictionary dictionary-fault-insert dictionary-fault-replace dictionary-fault-remove dictionary-fault-copy dictionary-fault-unique-insert dictionary-fault-unique-replace)
  add_test(NAME original_runtime_${family} COMMAND original_runtime_fixture "${family}")
  set_tests_properties(original_runtime_${family} PROPERTIES LABELS "original;runtime" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
add_executable(original_output_fixture tests/original/output.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_output_fixture PRIVATE original_runtime_common)
foreach(family IN ITEMS functional rejection boundaries faults)
  add_test(NAME original_output_${family} COMMAND original_output_fixture "${family}")
  set_tests_properties(original_output_${family} PROPERTIES LABELS "original;map;chunk;output" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
add_executable(original_chunk_fixture tests/original/chunks.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_chunk_fixture PRIVATE original_runtime_common)
foreach(family IN ITEMS functional malformed fault-toc fault-values cold-names fault-cold-names)
  add_test(NAME original_chunk_${family} COMMAND original_chunk_fixture "${family}")
  set_tests_properties(original_chunk_${family} PROPERTIES LABELS "original;map;chunk" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
add_executable(original_map_fixture tests/original/maps.cpp tests/original/ZlibFixture.cpp tests/original/AllocationFault.cpp)
target_link_libraries(original_map_fixture PRIVATE original_runtime_common)
foreach(family IN ITEMS compression legacy-codecs stream cached-stream fault-raw fault-ref fault-zlib fault-pair fault-huff fault-cached-hit fault-cached-miss)
  add_test(NAME original_map_${family} COMMAND original_map_fixture "${family}")
  set_tests_properties(original_map_${family} PROPERTIES LABELS "original;map" TIMEOUT 60
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
endforeach()
