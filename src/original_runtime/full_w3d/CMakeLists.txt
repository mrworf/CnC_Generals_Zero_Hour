set(draw_root "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw")
set(client_root "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient")
add_library(zh_original_w3d_draw_full OBJECT EXCLUDE_FROM_ALL
  "${draw_root}/W3DDefaultDraw.cpp"
  "${draw_root}/W3DModelDraw.cpp"
  "${draw_root}/W3DTankDraw.cpp"
  "${draw_root}/W3DTankTruckDraw.cpp"
  "${draw_root}/W3DTruckDraw.cpp"
  "${draw_root}/W3DSupplyDraw.cpp"
  "${draw_root}/W3DOverlordTankDraw.cpp"
  "${draw_root}/W3DDependencyModelDraw.cpp"
  "${draw_root}/W3DOverlordAircraftDraw.cpp"
  "${draw_root}/W3DOverlordTruckDraw.cpp"
  "${client_root}/W3DAssetManager.cpp"
  "${client_root}/W3DFileSystem.cpp"
  "${client_root}/W3DDisplay.cpp"
  "${client_root}/W3DView.cpp"
  "${client_root}/BaseHeightMap.cpp"
  "${client_root}/HeightMap.cpp"
  "${client_root}/Water/W3DWater.cpp"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/GameClient/System/Smudge.cpp"
  "${client_root}/W3DSmudge.cpp"
  "${client_root}/W3DTerrainVisual.cpp"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/GameClient/ParabolicEase.cpp"
  "${client_root}/W3DScene.cpp"
  "${client_root}/W3DStatusCircle.cpp"
  "${client_root}/W3DShroud.cpp"
  "${client_root}/W3DTerrainTracks.cpp"
  "${client_root}/Shadow/W3DShadow.cpp")
target_include_directories(zh_original_w3d_draw_full PRIVATE
  "${PROJECT_SOURCE_DIR}/include"
  "${PROJECT_SOURCE_DIR}/src/original_runtime/include"
  "${PROJECT_SOURCE_DIR}/src/original_runtime"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Include"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngineDevice/Include"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Include"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWMath"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWLib"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWDebug"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWSaveLoad")
target_compile_definitions(zh_original_w3d_draw_full PRIVATE ZH_WW3D_CPU_ONLY=1 NDEBUG)
target_compile_options(zh_original_w3d_draw_full PRIVATE
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-fpermissive;-ffunction-sections;-fdata-sections>)
target_link_libraries(zh_original_w3d_draw_full PRIVATE zh_build_options)

# The same canonical factory source has mutually exclusive executable
# configurations. The M20/M21 archive retains its schema/headless factory;
# full presentation binaries explicitly select this implementation first.
add_library(zh_original_w3d_factory_full OBJECT EXCLUDE_FROM_ALL
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/Common/Thing/W3DModuleFactory.cpp")
target_include_directories(zh_original_w3d_factory_full PRIVATE
  "${PROJECT_SOURCE_DIR}/include"
  "${PROJECT_SOURCE_DIR}/src/original_runtime/include"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Include"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngineDevice/Include"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Include"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWMath"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWLib"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWDebug"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas/WWSaveLoad")
target_compile_definitions(zh_original_w3d_factory_full PRIVATE NDEBUG ZH_W3D_FULL_INSTANCE=1)
target_compile_options(zh_original_w3d_factory_full PRIVATE
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-fpermissive;-ffunction-sections;-fdata-sections>)
target_link_libraries(zh_original_w3d_factory_full PRIVATE zh_build_options)
