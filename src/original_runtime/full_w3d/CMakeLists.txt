set(draw_root "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw")
set(client_root "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient")

# Keep the imported source tree read-only.  The Linux port needs validation to
# remain retryable when a model is not yet available, so produce a narrowly
# reviewed build-tree overlay instead of editing the linked upstream file.
set(model_draw_source "${draw_root}/W3DModelDraw.cpp")
file(READ "${model_draw_source}" model_draw_linux)
set(model_draw_validation_before [=[
	m_validated |= mode;

	for (ModelConditionVector::iterator c_it = m_conditionStates.begin(); c_it != m_conditionStates.end(); ++c_it)
]=])
set(model_draw_validation_after [=[
	if (!draw || !W3DDisplay::m_assetManager)
		return;

	for (ModelConditionVector::iterator c_it = m_conditionStates.begin(); c_it != m_conditionStates.end(); ++c_it)
]=])
string(REPLACE "${model_draw_validation_before}" "${model_draw_validation_after}"
  model_draw_linux "${model_draw_linux}")
set(model_draw_condition_before [=[
		c_it->validateStuff(NULL, draw->getScale(), m_extraPublicBones);
	}

	for (TransitionMap::iterator t_it = m_transitionMap.begin(); t_it != m_transitionMap.end(); ++t_it)
]=])
set(model_draw_condition_after [=[
		c_it->validateStuff(NULL, draw->getScale(), m_extraPublicBones);
		if (!c_it->m_modelName.isEmpty() &&
			!(c_it->m_validStuff & ModelConditionInfo::PRISTINE_BONES_VALID))
			return;
	}

	for (TransitionMap::iterator t_it = m_transitionMap.begin(); t_it != m_transitionMap.end(); ++t_it)
]=])
string(REPLACE "${model_draw_condition_before}" "${model_draw_condition_after}"
  model_draw_linux "${model_draw_linux}")
set(model_draw_transition_before [=[
			t_it->second.validateStuff(NULL, draw->getScale(), m_extraPublicBones);
]=])
set(model_draw_transition_after [=[
			t_it->second.validateStuff(NULL, draw->getScale(), m_extraPublicBones);
			if (!t_it->second.m_modelName.isEmpty() &&
				!(t_it->second.m_validStuff & ModelConditionInfo::PRISTINE_BONES_VALID))
				return;
]=])
string(REPLACE "${model_draw_transition_before}" "${model_draw_transition_after}"
  model_draw_linux "${model_draw_linux}")
set(model_draw_validation_end_before [=[
		}
	}
}

//-------------------------------------------------------------------------------------------------
#endif
W3DModelDrawModuleData::~W3DModelDrawModuleData()
]=])
set(model_draw_validation_end_after [=[
		}
	}

	// Publish completion only after every matching state and transition has
	// validated.  A failed acquisition can then be retried deterministically.
	m_validated |= mode;
}

//-------------------------------------------------------------------------------------------------
#endif
W3DModelDrawModuleData::~W3DModelDrawModuleData()
]=])
string(REPLACE "${model_draw_validation_end_before}" "${model_draw_validation_end_after}"
  model_draw_linux "${model_draw_linux}")
set(model_draw_bones_before [=[
	m_pristineBones.clear();

	// go ahead and set this here, in case we return early.
	m_validStuff |= PRISTINE_BONES_VALID;

	Bool tossRobj = false;
	if (robj == NULL)
	{
		if (m_modelName.isEmpty())
		{
			//BONEPOS_LOG(("Bailing: model name is empty\n"));
			return;
		}

		robj = W3DDisplay::m_assetManager->Create_Render_Obj(m_modelName.str(), scale, 0);
]=])
set(model_draw_bones_after [=[
	m_pristineBones.clear();

	class TemporaryRenderOwner
	{
	public:
		RenderObjClass *m_renderObject = NULL;
		~TemporaryRenderOwner()
		{
			if (m_renderObject)
				m_renderObject->Release_Ref();
		}
	} temporaryRenderOwner;
	if (robj == NULL)
	{
		if (m_modelName.isEmpty())
		{
			// An intentionally empty condition has no model acquisition to retry.
			m_validStuff |= PRISTINE_BONES_VALID;
			return;
		}
		if (!W3DDisplay::m_assetManager)
			return;

		robj = W3DDisplay::m_assetManager->Create_Render_Obj(m_modelName.str(), scale, 0);
]=])
string(REPLACE "${model_draw_bones_before}" "${model_draw_bones_after}"
  model_draw_linux "${model_draw_linux}")
set(model_draw_bones_owner_before [=[
		tossRobj = true;
	}

	Matrix3D			originalTransform = robj->Get_Transform();	// save the transform
]=])
set(model_draw_bones_owner_after [=[
		temporaryRenderOwner.m_renderObject = robj;
	}

	Matrix3D			originalTransform = robj->Get_Transform();	// save the transform
]=])
string(REPLACE "${model_draw_bones_owner_before}" "${model_draw_bones_owner_after}"
  model_draw_linux "${model_draw_linux}")
set(model_draw_bones_end_before [=[
	if (tossRobj)
	{
		REF_PTR_RELEASE(robj);
	}
}

//-------------------------------------------------------------------------------------------------
void ModelConditionInfo::validateWeaponBarrelInfo() const
]=])
set(model_draw_bones_end_after [=[
	// Commit the cache only after the acquired owner has been completely
	// inspected and restored.
	m_validStuff |= PRISTINE_BONES_VALID;
}

//-------------------------------------------------------------------------------------------------
void ModelConditionInfo::validateWeaponBarrelInfo() const
]=])
string(REPLACE "${model_draw_bones_end_before}" "${model_draw_bones_end_after}"
  model_draw_linux "${model_draw_linux}")
foreach(required_marker
    "if (!draw || !W3DDisplay::m_assetManager)"
    "!(c_it->m_validStuff & ModelConditionInfo::PRISTINE_BONES_VALID)"
    "!(t_it->second.m_validStuff & ModelConditionInfo::PRISTINE_BONES_VALID)"
    "temporaryRenderOwner.m_renderObject = robj"
    "m_validStuff |= PRISTINE_BONES_VALID;")
  string(FIND "${model_draw_linux}" "${required_marker}" marker_index)
  if(marker_index EQUAL -1)
    message(FATAL_ERROR "W3DModelDraw Linux overlay missing marker: ${required_marker}")
  endif()
endforeach()
set(model_draw_generated "${CMAKE_CURRENT_BINARY_DIR}/generated/W3DModelDraw.cpp")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(WRITE "${model_draw_generated}" "${model_draw_linux}")
add_library(zh_original_w3d_draw_full OBJECT EXCLUDE_FROM_ALL
  "${draw_root}/W3DDefaultDraw.cpp"
  "${model_draw_generated}"
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
  "${client_root}/WorldHeightMap.cpp"
  "${client_root}/TileData.cpp"
  "${client_root}/TerrainTex.cpp"
  "${client_root}/W3DShaderManager.cpp"
  "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/GameClient/ParabolicEase.cpp"
  "${client_root}/W3DScene.cpp"
  "${client_root}/W3DStatusCircle.cpp"
  "${client_root}/W3DShroud.cpp"
  "${client_root}/W3DParticleSys.cpp"
  "${client_root}/W3DTerrainTracks.cpp"
  "${client_root}/Shadow/W3DBufferManager.cpp"
  "${PROJECT_SOURCE_DIR}/src/original_runtime/full_w3d/volume_geometry_cpu.cpp"
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
