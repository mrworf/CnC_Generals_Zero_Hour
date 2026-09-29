// Included only by the canonical Linux W3DView translation unit.
namespace {
Int s_cameraStartupFaultOrdinal=-1;
void cameraStartupBoundary()
{
	if (s_cameraStartupFaultOrdinal<0) return;
	if (s_cameraStartupFaultOrdinal--) return;
	s_cameraStartupFaultOrdinal=-1;throw std::bad_alloc();
}
struct CameraStartupProviders {
	W3DView *owner=nullptr;
	CameraClass *camera=nullptr;
	Display *display=nullptr;
	TerrainLogic *logic=nullptr;
	RTS3DScene *scene=nullptr;
	zh::original_runtime::OriginalGpuEdge *edge=nullptr;
	std::uint64_t generation=0;
	const GameLogic *logic_owner=nullptr;
	UnsignedInt64 logic_token=0;
} s_cameraStartupProviders;
struct CameraStartupAccepted {
	W3DView *owner = nullptr;
	CameraClass *camera = nullptr;
	HeightMapRenderObjClass *terrain = nullptr;
	WorldHeightMap *map = nullptr;
	TerrainLogic *logic = nullptr;
	zh::original_runtime::OriginalGpuEdge *edge = nullptr;
	std::uint64_t generation = 0;
	Coord3D position{0, 0, 0};
	Real angle = 0, pitch = 0, zoom = 0;
	Matrix3D transform{1};
} s_cameraStartupAccepted;

bool cameraFinite(const Matrix3D &value)
{
	for (int row=0;row<3;++row) for (int column=0;column<4;++column)
		if (!std::isfinite(value[row][column])) return false;
	return true;
}
bool cameraSame(const Matrix3D &left,const Matrix3D &right)
{
	for (int row=0;row<3;++row) for (int column=0;column<4;++column)
		if (left[row][column]!=right[row][column]) return false;
	return true;
}
void cameraRequire(bool value)
{
	if (!value) throw OriginalW3DDeviceUnavailable("original bounded camera startup rejected");
}
Real cameraGround(Real x,Real y)
{
	cameraStartupBoundary();
	const double ix=static_cast<double>(x)/MAP_XY_FACTOR;
	const double iy=static_cast<double>(y)/MAP_XY_FACTOR;
	cameraRequire(std::isfinite(ix) && std::isfinite(iy) &&
		ix>=std::numeric_limits<Int>::min() && ix<=std::numeric_limits<Int>::max() &&
		iy>=std::numeric_limits<Int>::min() && iy<=std::numeric_limits<Int>::max());
	const Real result=TheTerrainLogic->getGroundHeight(x,y);
	cameraRequire(std::isfinite(result));return result;
}
Real cameraHeightAround(Real x,Real y)
{
	Real height=cameraGround(x,y);
	height=std::max(height,cameraGround(x+40,y-40));
	height=std::max(height,cameraGround(x-40,y-40));
	height=std::max(height,cameraGround(x+40,y+40));
	return std::max(height,cameraGround(x-40,y+40));
}
}

UnsignedInt64 W3DView::cameraStartupLogicToken(const void *owner) noexcept
{
	return owner && owner==s_cameraStartupProviders.owner ? s_cameraStartupProviders.logic_token : 0;
}

struct W3DView::CameraStartupAttempt {
	struct CameraState {
		Matrix3D transform,inverse;
		Matrix4x4 projection_transform;
		FrustumClass frustum,view_frustum;
		OBBoxClass near_box;
		SphereClass sphere;
		AABoxClass box;
		ViewportClass viewport,view_plane;
		CameraClass::ProjectionType projection;
		Real aspect,near_z,far_z,zmin,zmax;
		unsigned long bits;
		bool identity,valid;
		explicit CameraState(const CameraClass &camera)
			:transform(camera.Transform),inverse(camera.CameraInvTransform),
			 projection_transform(camera.ProjectionTransform),frustum(camera.Frustum),
			 view_frustum(camera.ViewSpaceFrustum),near_box(camera.NearClipBBox),
			 sphere(camera.CachedBoundingSphere),box(camera.CachedBoundingBox),
			 viewport(camera.Viewport),view_plane(camera.ViewPlane),projection(camera.Projection),
			 aspect(camera.AspectRatio),near_z(camera.ZNear),far_z(camera.ZFar),
			 zmin(camera.ZBufferMin),zmax(camera.ZBufferMax),bits(camera.Bits),
			 identity(camera.IsTransformIdentity),valid(camera.FrustumValid) {}
		void restore(CameraClass &camera) const noexcept
		{
			camera.Transform=transform;camera.CameraInvTransform=inverse;
			camera.ProjectionTransform=projection_transform;camera.Frustum=frustum;
			camera.ViewSpaceFrustum=view_frustum;camera.NearClipBBox=near_box;
			camera.CachedBoundingSphere=sphere;camera.CachedBoundingBox=box;
			camera.Viewport=viewport;camera.ViewPlane=view_plane;camera.Projection=projection;
			camera.AspectRatio=aspect;camera.ZNear=near_z;camera.ZFar=far_z;
			camera.ZBufferMin=zmin;camera.ZBufferMax=zmax;camera.Bits=bits;
			camera.IsTransformIdentity=identity;camera.FrustumValid=valid;
		}
	};
	struct ViewState {
		Coord3D position,offset;
		Region2D constraint;
		Real angle,pitch,zoom,height,ground,fx_pitch;
		Bool constraint_valid,moved;
		explicit ViewState(const W3DView &view)
			:position(view.m_pos),offset(view.m_cameraOffset),constraint(view.m_cameraConstraint),
			 angle(view.m_angle),pitch(view.m_pitchAngle),zoom(view.m_zoom),
			 height(view.m_heightAboveGround),ground(view.m_groundLevel),fx_pitch(view.m_FXPitch),
			 constraint_valid(view.m_cameraConstraintValid),moved(view.m_cameraHasMovedSinceRequest) {}
		void publish(W3DView &view) const noexcept
		{
			view.m_pos=position;view.m_cameraOffset=offset;view.m_cameraConstraint=constraint;
			view.m_angle=angle;view.m_pitchAngle=pitch;view.m_zoom=zoom;
			view.m_heightAboveGround=height;view.m_groundLevel=ground;view.m_FXPitch=fx_pitch;
			view.m_cameraConstraintValid=constraint_valid;view.m_cameraHasMovedSinceRequest=moved;
		}
	};
	struct Tile {
		DX8VertexBufferClass *source;
		char *backup;
		zh::renderer::BufferHandle mapping;
		std::vector<unsigned char> backup_bytes,cpu_bytes;
	};
	W3DView &view;
	zh::original_runtime::OriginalGpuEdge &edge;
	HeightMapRenderObjClass &terrain;
	ViewState baseline,candidate;
	CameraState camera_baseline;
	CameraClass camera_candidate;
	std::vector<Tile> tiles;
	W3DPropBuffer *props;
	Bool full_update,prop_cull;
	zh::renderer::DeviceTransactionToken token{};
	bool active=false;
	GameLogic::TerrainLogicPublication logical_publication;

	static zh::original_runtime::OriginalGpuEdge &admit(W3DView &view)
	{
		auto *edge=zh::original_runtime::OriginalGpuEdge::active();
		if (edge) edge->guard_nonstage_mutation();
		const auto &providers=s_cameraStartupProviders;
		const auto logical=GameLogic::peekTerrainLogicPublication(TheGameLogic,TheTerrainLogic);
		cameraRequire(logical.owner && logical.provider && logical.token);
		cameraRequire(providers.owner==&view && providers.camera==view.m_3DCamera &&
			providers.display==TheDisplay &&
			(!providers.logic ? !providers.logic_owner && !providers.logic_token :
				providers.logic==logical.provider && providers.logic_owner==logical.owner &&
				providers.logic_token==logical.token) &&
			providers.scene==W3DDisplay::m_3DScene && providers.edge==edge && edge &&
			providers.generation==edge->generation());
		// Compare raw borrowed globals with the registered map/scene before
		// any terrain or prop-buffer dereference, including the no-prop state.
		cameraRequire(W3DTerrainVisual::cameraStartupOwnerMatches(TheTerrainVisual,TheHeightMap));
		auto *display=dynamic_cast<W3DDisplay *>(TheDisplay);
		cameraRequire(edge && display && TheTacticalView==&view && display->getFirstView()==&view &&
			!view.getNextView() && view.m_3DCamera && view.m_2DCamera && TheGlobalData &&
			TheTerrainLogic && W3DDisplay::m_3DScene &&
			dynamic_cast<W3DTerrainVisual *>(TheTerrainVisual) && TheHeightMap &&
			TheTerrainRenderObject==TheHeightMap && TheHeightMap->getMap() &&
			TheHeightMap->Peek_Scene()==W3DDisplay::m_3DScene &&
			edge->idle_preparation_ready() && !edge->tree_source_frame_pending() &&
			!WW3D::Is_Rendering() && !TheHeightMap->preparedTreePhaseIdentity() &&
			view.getCameraLock()==INVALID_ID && !view.m_cameraLockDrawable &&
			!view.m_doingMoveCameraOnWaypointPath && !view.m_doingRotateCamera &&
			!view.m_doingPitchCamera && !view.m_doingZoomCamera && !view.m_doingScriptedCameraLock &&
			!view.m_isCameraSlaved && !view.m_useRealZoomCam && !view.m_freezeTimeForCameraMovement &&
			view.m_viewFilterMode==FM_VIEW_DEFAULT && view.m_viewFilter==FT_VIEW_DEFAULT &&
			!view.m_isWireFrameEnabled && !view.m_nextWireFrameEnabled &&
			!view.m_shakeIntensity && !view.m_shakeOffset.x && !view.m_shakeOffset.y &&
			!CameraShakerSystem.IsCameraShaking() && view.m_width>0 && view.m_height>0 &&
			!display->m_letterBoxEnabled && display->m_letterBoxFadeLevel==0);
		if (s_cameraStartupAccepted.owner==&view)
			cameraRequire(s_cameraStartupAccepted.camera==view.m_3DCamera &&
				s_cameraStartupAccepted.terrain==TheHeightMap &&
				s_cameraStartupAccepted.map==TheHeightMap->getMap() &&
				s_cameraStartupAccepted.logic==TheTerrainLogic &&
				s_cameraStartupAccepted.edge==edge && s_cameraStartupAccepted.generation==edge->generation());
		TheHeightMap->preflightTreeRemoval();
		return *edge;
	}
	explicit CameraStartupAttempt(W3DView &source)
		:view(source),edge(admit(source)),terrain(*TheHeightMap),baseline(source),candidate(source),
		 camera_baseline(*source.m_3DCamera),props(terrain.m_propBuffer),
		 full_update(terrain.m_needFullUpdate),prop_cull(props ? props->m_doCull : FALSE),
		 logical_publication(GameLogic::peekTerrainLogicPublication(TheGameLogic,TheTerrainLogic))
	{
		camera_baseline.restore(camera_candidate);
		using Capacity=zh::renderer::CameraStartupTransactionLimits;
		static_assert(VERTEX_BUFFER_TILE_LENGTH==Capacity::tile_length && sizeof(VERTEX_FORMAT)==Capacity::vertex_bytes,
			"camera capacity must match authored terrain layout");
		cameraRequire(terrain.m_numVertexBufferTiles>0 && terrain.m_numVertexBufferTiles<=Capacity::maximum_tiles &&
			terrain.m_vertexBufferTiles && terrain.m_vertexBufferBackup && terrain.m_x>1 && terrain.m_y>1 &&
			terrain.m_x<=Capacity::maximum_tile_axis*VERTEX_BUFFER_TILE_LENGTH+1 &&
			terrain.m_y<=Capacity::maximum_tile_axis*VERTEX_BUFFER_TILE_LENGTH+1 &&
			camera_candidate.Projection==CameraClass::PERSPECTIVE);
		const Int tiles_x=(terrain.m_x-2)/VERTEX_BUFFER_TILE_LENGTH+1;
		const Int tiles_y=(terrain.m_y-2)/VERTEX_BUFFER_TILE_LENGTH+1;
		cameraRequire(terrain.m_numVBTilesX==tiles_x && terrain.m_numVBTilesY==tiles_y &&
			terrain.m_numVertexBufferTiles==tiles_x*tiles_y);
		constexpr std::size_t bytes=VERTEX_BUFFER_TILE_LENGTH*VERTEX_BUFFER_TILE_LENGTH*4*sizeof(VERTEX_FORMAT);
		constexpr std::size_t maximum_source_bytes=std::size_t(Capacity::maximum_tiles)*(2*bytes+sizeof(Tile))+
			sizeof(CameraStartupAttempt);
		cameraRequire(static_cast<std::size_t>(terrain.m_numVertexBufferTiles)*(2*bytes+sizeof(Tile))+
			sizeof(CameraStartupAttempt)<=maximum_source_bytes);
		cameraStartupBoundary();tiles.reserve(terrain.m_numVertexBufferTiles);
		for (Int i=0;i<terrain.m_numVertexBufferTiles;++i) {
			auto *buffer=terrain.m_vertexBufferTiles[i];
			const auto found=edge.vertices_.find(buffer);
			cameraRequire(buffer && found!=edge.vertices_.end() && terrain.m_vertexBufferBackup[i]);
			cameraRequire(buffer->Get_CPU_Vertex_Buffer() && !buffer->Engine_Refs() &&
				static_cast<std::size_t>(buffer->Get_Vertex_Count())*buffer->FVF_Info().Get_FVF_Size()==bytes);
			Tile tile{buffer,terrain.m_vertexBufferBackup[i],found->second,{},{}};
			cameraStartupBoundary();
			tile.backup_bytes.assign(reinterpret_cast<unsigned char *>(tile.backup),
				reinterpret_cast<unsigned char *>(tile.backup)+bytes);
			cameraStartupBoundary();
			tile.cpu_bytes.assign(buffer->Get_CPU_Vertex_Buffer(),buffer->Get_CPU_Vertex_Buffer()+bytes);
			cameraStartupBoundary();
			tiles.push_back(std::move(tile));
		}
	}
	~CameraStartupAttempt()
	{
		if (!active) return;
		if (!edge.abort_device_transaction(token)) std::terminate();
		baseline.publish(view);camera_baseline.restore(*view.m_3DCamera);
		terrain.m_needFullUpdate=full_update;
		if (props) props->m_doCull=prop_cull;
		for (const auto &tile:tiles) {
			const auto found=edge.vertices_.find(tile.source);
			if (found==edge.vertices_.end() || found->second!=tile.mapping) std::terminate();
			std::memcpy(tile.backup,tile.backup_bytes.data(),tile.backup_bytes.size());
			std::memcpy(tile.source->Get_CPU_Vertex_Buffer(),tile.cpu_bytes.data(),tile.cpu_bytes.size());
		}
	}
	void offsetForHeight(Real height)
	{
		const Real pitch=std::tan(TheGlobalData->m_cameraPitch*(PI/180.0));
		const Real yaw=std::tan(TheGlobalData->m_cameraYaw*(PI/180.0));
		cameraRequire(std::isfinite(height) && height!=0 && std::isfinite(pitch) && pitch!=0 && std::isfinite(yaw));
		candidate.offset.z=height;candidate.offset.y=-(height/pitch);
		candidate.offset.x=-(candidate.offset.y*yaw);
		cameraRequire(std::isfinite(candidate.offset.x) && std::isfinite(candidate.offset.y));
	}
	Matrix3D transform() const
	{
		cameraRequire(std::isfinite(candidate.zoom) && std::isfinite(candidate.angle) &&
			std::isfinite(candidate.pitch) && std::isfinite(candidate.ground) && candidate.fx_pitch==1);
		Vector3 source(candidate.offset.x*candidate.zoom,candidate.offset.y*candidate.zoom,
			candidate.offset.z*candidate.zoom);
		cameraRequire(std::isfinite(source.Z) && source.Z!=0);
		const Real factor=1-candidate.ground/source.Z;
		Matrix3D pitch(Vector3(1,0,0),candidate.pitch),angle(Vector3(0,0,1),candidate.angle);
		pitch.mulVector3(source);angle.mulVector3(source);source*=factor;
		Coord3D position=candidate.position;
		if (candidate.constraint_valid) {
			position.x=std::min(candidate.constraint.hi.x,std::max(candidate.constraint.lo.x,position.x));
			position.y=std::min(candidate.constraint.hi.y,std::max(candidate.constraint.lo.y,position.y));
		}
		source+=Vector3(position.x,position.y,candidate.ground);
		Vector3 target(position.x,position.y,candidate.ground);
		const Vector3 distance=source-target;
		cameraRequire(std::isfinite(distance.Length2()) && distance.Length2()>0);
		Matrix3D result(1);result.Look_At(source,target,0);cameraRequire(cameraFinite(result));return result;
	}
	Vector3 rayAt(Real x,Real y) const
	{
		const Real logical_x=(2.0f*x)/view.m_width-1.0f,logical_y=-((2.0f*y)/view.m_height-1.0f);
		Vector3 end;camera_candidate.Un_Project(end,Vector2(logical_x,logical_y));
		Vector3 ray=end-camera_candidate.Get_Position();
		cameraRequire(std::isfinite(ray.Length2()) && ray.Length2()>0);
		ray.Normalize();return ray;
	}
	void build()
	{
		if (!candidate.offset.z) offsetForHeight(TheGlobalData->m_cameraHeight);
		camera_candidate.Set_Clip_Planes(MAP_XY_FACTOR,
			TheGlobalData->m_drawEntireTerrain || candidate.zoom>1.05f ? 1200.0f*MAP_XY_FACTOR : 1200.0f);
		bool constrained=true;
#if defined(_DEBUG) || defined(_INTERNAL)
		constrained=TheGlobalData->m_useCameraConstraints;
#endif
		if (constrained && !candidate.constraint_valid) {
			camera_candidate.Set_Transform(transform());
			cameraStartupBoundary();
			Region3D extent;TheTerrainLogic->getExtent(&extent);
			const auto position=camera_candidate.Get_Position();
			const auto center_ray=rayAt(static_cast<Int>(view.m_width*0.5f+view.m_originX)-view.m_originX,
				static_cast<Int>(view.m_height*0.5f+view.m_originY)-view.m_originY)*camera_candidate.Get_Depth();
			const auto bottom_ray=rayAt(static_cast<Int>(view.m_width*0.5f+view.m_originX)-view.m_originX,
				static_cast<Int>(view.m_height*0.95f+view.m_originY)-view.m_originY)*camera_candidate.Get_Depth();
			cameraRequire(center_ray.Z!=0 && bottom_ray.Z!=0);
			const auto center_end=position+center_ray,bottom_end=position+bottom_ray;
			Vector3 center(Vector3::Find_X_At_Z(candidate.ground,position,center_end),
				Vector3::Find_Y_At_Z(candidate.ground,position,center_end),candidate.ground);
			Vector3 bottom(Vector3::Find_X_At_Z(candidate.ground,position,bottom_end),
				Vector3::Find_Y_At_Z(candidate.ground,position,bottom_end),candidate.ground);
			Real amount=(center-bottom).Length();
			if (TheGlobalData->m_debugAI) amount=-1000;
			cameraRequire(std::isfinite(amount) && std::isfinite(extent.lo.x) && std::isfinite(extent.hi.x) &&
				std::isfinite(extent.lo.y) && std::isfinite(extent.hi.y));
			candidate.constraint.lo.x=extent.lo.x+amount;candidate.constraint.hi.x=extent.hi.x-amount;
			candidate.constraint.lo.y=extent.lo.y+amount;candidate.constraint.hi.y=extent.hi.y-amount;
			cameraRequire(std::isfinite(candidate.constraint.lo.x) && std::isfinite(candidate.constraint.hi.x) &&
				std::isfinite(candidate.constraint.lo.y) && std::isfinite(candidate.constraint.hi.y));
			candidate.constraint_valid=TRUE;
		}
		if (constrained && candidate.constraint_valid) {
			candidate.position.x=std::min(candidate.constraint.hi.x,std::max(candidate.constraint.lo.x,candidate.position.x));
			candidate.position.y=std::min(candidate.constraint.hi.y,std::max(candidate.constraint.lo.y,candidate.position.y));
		}
#if defined(_DEBUG) || defined(_INTERNAL)
		cameraRequire(std::isfinite(view.m_FOV) && view.m_FOV>0 && view.m_FOV<PI);
		camera_candidate.Set_View_Plane(view.m_FOV,-1);
#endif
		camera_candidate.Set_Transform(transform());candidate.moved=TRUE;
	}
	void commit(bool change_camera=true)
	{
		if (change_camera) build();
		cameraStartupBoundary();
		auto *lights=W3DDisplay::m_3DScene->createLightsIterator();
		try {
			cameraStartupBoundary();
			const zh::renderer::DeviceTransactionDesc desc{zh::renderer::DeviceTransactionMode::idle_preparation,
				edge.generation(),4096,4096,zh::renderer::CameraStartupTransactionLimits::maximum_bytes,0,
				zh::renderer::DeviceTransactionCapacity::camera_startup};
			cameraRequire(static_cast<bool>(edge.begin_device_transaction(desc,token)));active=true;
			terrain.updateCenter(&camera_candidate,lights);
			CameraState accepted(camera_candidate);
			if (!change_camera) edge.record_source_state("original W3DView::updateView terrain center");
			cameraStartupBoundary();
			const auto logical=GameLogic::peekTerrainLogicPublication(TheGameLogic,TheTerrainLogic);
			cameraRequire(logical.owner==logical_publication.owner && logical.provider==logical_publication.provider &&
				logical.token && logical.token==logical_publication.token);
			cameraRequire(edge.commit_device_transaction(token));active=false;
			if (change_camera) {
				candidate.publish(view);accepted.restore(*view.m_3DCamera);
				s_cameraStartupProviders.logic=logical_publication.provider;
				s_cameraStartupProviders.logic_owner=logical_publication.owner;
				s_cameraStartupProviders.logic_token=logical_publication.token;
				s_cameraStartupAccepted={&view,view.m_3DCamera,&terrain,terrain.getMap(),logical_publication.provider,
					&edge,edge.generation(),candidate.position,candidate.angle,candidate.pitch,candidate.zoom,
					camera_candidate.Get_Transform()};
			}
			if (lights) W3DDisplay::m_3DScene->destroyLightsIterator(lights);
		} catch (...) {
			if (lights) W3DDisplay::m_3DScene->destroyLightsIterator(lights);
			throw;
		}
	}
};

Bool W3DView::cameraStartupRayWorkAllowed(UnsignedInt64 visits)
{
	return visits<=4194304;
}

void W3DView::setCameraStartupFaultOrdinal(Int ordinal)
{
	s_cameraStartupFaultOrdinal=ordinal;
}

Bool W3DView::castCameraStartupRay(const Vector3 &start,const Vector3 &end,Vector3 *point) const
{
	cameraRequire(point && TheHeightMap && TheHeightMap->getMap());
	for (int axis=0;axis<3;++axis) cameraRequire(std::isfinite(start[axis]) && std::isfinite(end[axis]));
	cameraRequire(std::isfinite((end-start).Length2()) && (end-start).Length2()>0);
	auto &terrain=*TheHeightMap;auto *map=terrain.getMap();
	const Int border=map->getBorderSizeInline();
	cameraRequire(border>=0 && border<=std::numeric_limits<Int>::max()-72 &&
		map->getXExtent()>1 && map->getYExtent()>1);
	const Int overhang=64+border;
	cameraRequire(map->getXExtent()<=std::numeric_limits<Int>::max()-overhang &&
		map->getYExtent()<=std::numeric_limits<Int>::max()-overhang);
	AABoxClass box;box.Init(MinMaxAABoxClass(
		Vector3(MAP_XY_FACTOR*(-overhang),MAP_XY_FACTOR*(-overhang),-MAP_XY_FACTOR),
		Vector3(MAP_XY_FACTOR*(map->getXExtent()+overhang),MAP_XY_FACTOR*(map->getYExtent()+overhang),
			MAP_HEIGHT_SCALE*map->getMaxHeightValue()+MAP_XY_FACTOR)));
	LineSegClass ray(start,end),segment=ray,reverse;
	CastResultStruct clipped;
	Vector3 first=start,last=end;
	Int x0=0,x1=0,y0=0,y1=0;
	UnsignedInt64 visits=0;
	auto consume=[&](Int lo_x,Int hi_x,Int lo_y,Int hi_y) {
		const auto width=static_cast<UnsignedInt64>(static_cast<long long>(hi_x)-lo_x+1);
		const auto height=static_cast<UnsignedInt64>(static_cast<long long>(hi_y)-lo_y+1);
		cameraRequire(hi_x>=lo_x && hi_y>=lo_y && height && width<=4194304/height);
		const auto count=width*height;
		cameraRequire(cameraStartupRayWorkAllowed(visits+count));visits+=count;
	};
	auto floor_cell=[&](Real value) {
		const double cell=std::floor(static_cast<double>(value)/MAP_XY_FACTOR);
		cameraRequire(std::isfinite(cell) && cell>=std::numeric_limits<Int>::min()+static_cast<double>(border)+8 &&
			cell<=std::numeric_limits<Int>::max()-static_cast<double>(border)-8);
		return REAL_TO_INT_FLOOR(value/MAP_XY_FACTOR);
	};
	auto ceil_cell=[&](Real value) {
		const double cell=std::ceil(static_cast<double>(value)/MAP_XY_FACTOR);
		cameraRequire(std::isfinite(cell) && cell>=std::numeric_limits<Int>::min()+static_cast<double>(border)+8 &&
			cell<=std::numeric_limits<Int>::max()-static_cast<double>(border)-8);
		return REAL_TO_INT_CEIL(value/MAP_XY_FACTOR);
	};
	for (Int pass=0;pass<3;++pass) {
		clipped.Reset();clipped.ComputeContactPoint=true;
		if (CollisionMath::Collide(segment,box,&clipped)) {
			if (!clipped.StartBad) first=clipped.ContactPoint;
			clipped.Fraction=1;clipped.StartBad=false;reverse.Set(segment.Get_P1(),segment.Get_P0());
			if (CollisionMath::Collide(reverse,box,&clipped) && !clipped.StartBad) last=clipped.ContactPoint;
		} else { if (!pass) return FALSE;break; }
		x0=floor_cell(std::min(first.X,last.X));x1=ceil_cell(std::max(first.X,last.X));
		y0=floor_cell(std::min(first.Y,last.Y));y1=ceil_cell(std::max(first.Y,last.Y));
		consume(x0,x1,y0,y1);
		Int minimum=map->getMaxHeightValue(),maximum=0;
		for (Int y=y0;y<=y1;++y) for (Int x=x0;x<=x1;++x) {
			const Int value=terrain.getClipHeight(x+border,y+border);
			minimum=std::min(minimum,value);maximum=std::max(maximum,value);
		}
		box.Init(MinMaxAABoxClass(
			Vector3(MAP_XY_FACTOR*(x0-1),MAP_XY_FACTOR*(y0-1),MAP_HEIGHT_SCALE*(minimum-1)),
			Vector3(MAP_XY_FACTOR*(x1+1),MAP_XY_FACTOR*(y1+1),MAP_HEIGHT_SCALE*(maximum+1))));
	}
	x0+=border;x1+=border;y0+=border;y1+=border;
	// Both source triangle passes are admitted before any triangle collision.
	consume(x0-1,x1+1,y0-1,y1+1);consume(x0-3,x1+3,y0-3,y1+3);
	CastResultStruct result;result.ComputeContactPoint=true;
	Bool hit=FALSE;TriClass triangle;Vector3 normal,vertices[4];triangle.N=&normal;
	for (Int offset=1;offset<5;offset*=3) {
		for (Int y=y0-offset;y<=y1+offset;++y) for (Int x=x0-offset;x<=x1+offset;++x) {
			vertices[0].Set((x-border)*MAP_XY_FACTOR,(y-border)*MAP_XY_FACTOR,
				MAP_HEIGHT_SCALE*terrain.getClipHeight(x,y));
			vertices[1].Set((x+1-border)*MAP_XY_FACTOR,(y-border)*MAP_XY_FACTOR,
				MAP_HEIGHT_SCALE*terrain.getClipHeight(x+1,y));
			vertices[2].Set((x+1-border)*MAP_XY_FACTOR,(y+1-border)*MAP_XY_FACTOR,
				MAP_HEIGHT_SCALE*terrain.getClipHeight(x+1,y+1));
			vertices[3].Set((x-border)*MAP_XY_FACTOR,(y+1-border)*MAP_XY_FACTOR,
				MAP_HEIGHT_SCALE*terrain.getClipHeight(x,y+1));
			triangle.V[0]=&vertices[0];triangle.V[1]=&vertices[1];triangle.V[2]=&vertices[2];
			triangle.Compute_Normal();hit=hit || CollisionMath::Collide(ray,triangle,&result);
			if (result.StartBad) { *point=result.ContactPoint;return TRUE; }
			triangle.V[0]=&vertices[2];triangle.V[1]=&vertices[3];triangle.V[2]=&vertices[0];
			triangle.Compute_Normal();hit=hit || CollisionMath::Collide(ray,triangle,&result);
		}
	}
	if (hit) *point=result.ContactPoint;
	return hit;
}

void W3DView::applyCameraStartup(Int operation,const Coord3D *position)
{
	CameraStartupAttempt attempt(*this);
	auto &candidate=attempt.candidate;
	if (operation==0) {
		candidate.angle=m_defaultAngle;candidate.pitch=m_defaultPitchAngle;candidate.fx_pitch=1;
	} else if (operation==1) {
		cameraRequire(position && std::isfinite(position->x) && std::isfinite(position->y) && std::isfinite(position->z));
		candidate.position=*position;
		if (position->z>PATHFIND_CELL_SIZE_F+cameraGround(position->x,position->y)) {
			Vector3 start(position->x,position->y,position->z),point;
			Vector3 direction;
			attempt.camera_candidate.Un_Project(direction,Vector2(0,0));
			direction-=attempt.camera_candidate.Get_Position();
			cameraRequire(std::isfinite(direction.Length2()) && direction.Length2()>0);
			direction.Normalize();direction*=attempt.camera_candidate.Get_Depth();
			if (castCameraStartupRay(start,start+direction,&point)) {
				candidate.position.x=point.X;candidate.position.y=point.Y;
			}
		}
		candidate.position.z=0;
	} else if (operation==2) {
		candidate.ground=std::min(cameraGround(candidate.position.x,candidate.position.y),120.0f);
		attempt.offsetForHeight(candidate.ground+TheGlobalData->m_cameraHeight);candidate.constraint_valid=FALSE;
	} else if (operation==3) {
		if (!candidate.offset.z) attempt.offsetForHeight(TheGlobalData->m_cameraHeight);
		candidate.zoom=(cameraHeightAround(candidate.position.x,candidate.position.y)+m_maxHeightAboveGround)/candidate.offset.z;
		candidate.height=m_maxHeightAboveGround;candidate.constraint_valid=FALSE;
	} else cameraRequire(false);
	attempt.commit();
}
