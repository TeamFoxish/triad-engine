#include "NavMeshBuilder.h"

#include "logs/Logs.h"
#include "Recast.h"
#include "RecastDebugDraw.h"
#include "RecastDump.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourDebugDraw.h"
#include "render/Renderable.h"
#include "shared/TransformStorage.h"
#include "shared/SharedStorage.h"
#include "render/mesh/Mesh.h"
#include <algorithm>

NavMeshBuilder::NavMeshBuilder()
{
}

NavMeshBuilder::~NavMeshBuilder()
{
}

bool NavMeshBuilder::buildNavMesh(
    std::vector<const Renderable*> meshes,
    NavMeshAgent *agent,
    BuildConfig config,
    std::vector<ConvexVolume> convexVolumes,
	std::unique_ptr<NavMesh>& navOut)
{
    if (meshes.empty())
	{
		LOG_ERROR("buildNavigation: Input mesh is not specified.");
		return false;
	}

    std::vector<float> sceneVerts;
	std::vector<int> sceneIndices;

	// TODO cull verts outside building volume
	// We need to combine all static meshes to unite scene mesh
#ifdef EDITOR
	uint32_t maxIndex = 0;
    for (const Renderable* data : meshes) {
		
		const Math::Transform& meshTransform = SharedStorage::Instance().transforms.AccessRead(data->transform);
        for (const auto& node : data->mesh->GetNodes()) {
			for (const auto& geometry : node.geoms) {

				// Transforming vertices according to mesh transform and node transform inside mesh
				const std::vector<float>& vertSrc = geometry->vertices;
				for (int i = 0; i < vertSrc.size(); i+=3) {
					const Math::Vector3 vertex = {vertSrc[i], vertSrc[i + 1], vertSrc[i + 2]};
					// TODO: apply mesh node transform
					Math::Vector3 worldV = 
						Math::Vector3::Transform(vertex, meshTransform.GetMatrix());
					sceneVerts.push_back(worldV.x);
					sceneVerts.push_back(worldV.y);
					sceneVerts.push_back(worldV.z);
				}

				const std::vector<uint32_t>& idxSrc = geometry->indices;

				// we need to adjust indexes for each new mesh, so indexes for each pf them will be unique
				uint32_t maxLocalIndex = *std::max_element(idxSrc.begin(), idxSrc.end());
				for (uint32_t index : idxSrc) {
					sceneIndices.emplace_back(maxIndex + index);
				}
				maxIndex += maxLocalIndex + 1;
			}
		}
    }
#endif // EDITOR
	
	const float* verts = sceneVerts.data();
	const int nverts = (int)sceneVerts.size();
	// reverese indicies (invert triangles winding order) since we use left-handed cs, while recast uses right-handed
	std::reverse(sceneIndices.begin(), sceneIndices.end());
	const int* tris = sceneIndices.data();
	assert(sceneIndices.size() % 3 == 0);
	const int ntris = (int)sceneIndices.size() / 3;
	
	//
	// Step 1. Initialize build config.
	//
	
    rcConfig cfg = rcConfig();
	// Init build configuration from GUI
	memset(&cfg, 0, sizeof(cfg));
	cfg.cs = config.rasterization.cellSize;
	cfg.ch = config.rasterization.cellHeigth;
	cfg.walkableSlopeAngle = agent->maxSlope;
	cfg.walkableHeight = (int)ceilf(agent->heigth / cfg.ch);
	cfg.walkableClimb = (int)floorf(agent->maxClimb / cfg.ch);
	cfg.walkableRadius = (int)ceilf(agent->radius / cfg.cs);
	cfg.maxEdgeLen = (int)(config.polygonization.maxEdgeLength / config.rasterization.cellSize);
	cfg.maxSimplificationError = config.detail.maxSampleError;
	cfg.minRegionArea = (int)rcSqr(config.region.minRegionSize);		// Note: area = size*size
	cfg.mergeRegionArea = (int)rcSqr(config.region.mergedRegionSize);	// Note: area = size*size
	cfg.maxVertsPerPoly = (int) config.polygonization.vertsPerPoly;
	cfg.detailSampleDist = config.detail.sampleDistance < 0.9f 
		? 0 
		: config.rasterization.cellSize * config.detail.sampleDistance;
	cfg.detailSampleMaxError = config.rasterization.cellHeigth * config.detail.maxSampleError;
	
	// Set the area where the navigation will be build.
	// Here the bounds of the input mesh are used, but the
	// area could be specified by an user defined box, etc.
	rcVcopy(cfg.bmin, config.bMin);
	rcVcopy(cfg.bmax, config.bMax);
	rcCalcGridSize(cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height);

	class rcContextCustom : public rcContext {
	protected:
		/// Logs a message.
		/// @param[in]		category	The category of the message.
		/// @param[in]		msg			The formatted message.
		/// @param[in]		len			The length of the formatted message.
		virtual void doLog(const rcLogCategory category, const char* msg, const int len) override { 
			switch (category) {
				case RC_LOG_PROGRESS:
					LOG_INFO(std::string_view(msg, len));
					break;
				case RC_LOG_WARNING:
					LOG_WARN(std::string_view(msg, len));
					break;
				case RC_LOG_ERROR:
					LOG_WARN(std::string_view(msg, len));
					break;
			}
		}
	} ctxInst;
	rcContext* ctx = &ctxInst;

	// Reset build times gathering.
	ctx->resetTimers();

	// Start the build process.	
	ctx->startTimer(RC_TIMER_TOTAL);
	
	ctx->log(RC_LOG_PROGRESS, "Building navigation:");
	ctx->log(RC_LOG_PROGRESS, " - %d x %d cells", cfg.width, cfg.height);
	ctx->log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", nverts/1000.0f, ntris/1000.0f);
	
	//
	// Step 2. Rasterize input polygon soup.
	//
	
	// Allocate voxel heightfield where we rasterize our input data to.
	rcHeightfield* solid = rcAllocHeightfield();
	if (!solid)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
		return false;
	}
	if (!rcCreateHeightfield(ctx, *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch))
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
		return false;
	}
	
	// Allocate array that can hold triangle area types.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	unsigned char* triareas = new unsigned char[ntris];
	if (!triareas)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'm_triareas' (%d).", ntris);
		return false;
	}
	
	// Find triangles which are walkable based on their slope and rasterize them.
	// If your input data is multiple meshes, you can transform them here, calculate
	// the are type for each of the meshes and rasterize them.
	memset(triareas, 0, ntris*sizeof(unsigned char));
	rcMarkWalkableTriangles(ctx, cfg.walkableSlopeAngle, verts, nverts, tris, ntris, triareas);
	if (!rcRasterizeTriangles(ctx, verts, nverts, tris, triareas, ntris, *solid, cfg.walkableClimb))
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not rasterize triangles.");
		return false;
	}

	delete [] triareas;
	triareas = nullptr;
	
	//
	// Step 3. Filter walkable surfaces.
	//
	
	// Once all geometry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	if (config.filterLowHangingObstacles)
		rcFilterLowHangingWalkableObstacles(ctx, cfg.walkableClimb, *solid);
	if (config.filterLedgeSpans)
		rcFilterLedgeSpans(ctx, cfg.walkableHeight, cfg.walkableClimb, *solid);
	if (config.filterWalkableLowHeightSpans)
		rcFilterWalkableLowHeightSpans(ctx, cfg.walkableHeight, *solid);


	//
	// Step 4. Partition walkable surface to simple regions.
	//

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	rcCompactHeightfield* compactHf = rcAllocCompactHeightfield();
	if (!compactHf)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
		return false;
	}
	if (!rcBuildCompactHeightfield(ctx, cfg.walkableHeight, cfg.walkableClimb, *solid, *compactHf))
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
		return false;
	}
	
	rcFreeHeightField(solid);
	solid = 0;
		
	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(ctx, cfg.walkableRadius, *compactHf))
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
		return false;
	}

	// (Optional) Mark areas.
	for (int i  = 0; i < convexVolumes.size(); ++i)
		rcMarkConvexPolyArea(ctx, convexVolumes[i].verts, convexVolumes[i].nverts, convexVolumes[i].hmin, convexVolumes[i].hmax, (unsigned char)convexVolumes[i].area, *compactHf);

	
	// Partition the heightfield so that we can use simple algorithm later to triangulate the walkable areas.
	// There are 3 partitioning methods, each with some pros and cons:
	// 1) Watershed partitioning
	//   - the classic Recast partitioning
	//   - creates the nicest tessellation
	//   - usually slowest
	//   - partitions the heightfield into nice regions without holes or overlaps
	//   - the are some corner cases where this method creates produces holes and overlaps
	//      - holes may appear when a small obstacles is close to large open area (triangulation can handle this)
	//      - overlaps may occur if you have narrow spiral corridors (i.e stairs), this make triangulation to fail
	//   * generally the best choice if you precompute the navmesh, use this if you have large open areas
	// 2) Monotone partitioning
	//   - fastest
	//   - partitions the heightfield into regions without holes and overlaps (guaranteed)
	//   - creates long thin polygons, which sometimes causes paths with detours
	//   * use this if you want fast navmesh generation
	// 3) Layer partitoining
	//   - quite fast
	//   - partitions the heighfield into non-overlapping regions
	//   - relies on the triangulation code to cope with holes (thus slower than monotone partitioning)
	//   - produces better triangles than monotone partitioning
	//   - does not have the corner cases of watershed partitioning
	//   - can be slow and create a bit ugly tessellation (still better than monotone)
	//     if you have large open areas with small obstacles (not a problem if you use tiles)
	//   * good choice to use for tiled navmesh with medium and small sized tiles
	
	if (config.partitionType == PARTITION_WATERSHED)
	{
		// Prepare for region partitioning, by calculating distance field along the walkable surface.
		if (!rcBuildDistanceField(ctx, *compactHf))
		{
			ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
			return false;
		}
		
		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildRegions(ctx, *compactHf, 0, cfg.minRegionArea, cfg.mergeRegionArea))
		{
			ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
			return false;
		}
	}
	else if (config.partitionType == PARTITION_MONOTONE)
	{
		// Partition the walkable surface into simple regions without holes.
		// Monotone partitioning does not need distancefield.
		if (!rcBuildRegionsMonotone(ctx, *compactHf, 0, cfg.minRegionArea, cfg.mergeRegionArea))
		{
			ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build monotone regions.");
			return false;
		}
	}
	else // SAMPLE_PARTITION_LAYERS
	{
		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildLayerRegions(ctx, *compactHf, 0, cfg.minRegionArea))
		{
			ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build layer regions.");
			return false;
		}
	}
	
	//
	// Step 5. Trace and simplify region contours.
	//
	
	// Create contours.
	rcContourSet* contours = rcAllocContourSet();
	if (!contours)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
		return false;
	}
	if (!rcBuildContours(ctx, *compactHf, cfg.maxSimplificationError, cfg.maxEdgeLen, *contours))
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
		return false;
	}
	
	//
	// Step 6. Build polygons mesh from contours.
	//
	
	// Build polygon navmesh from the contours.
	rcPolyMesh* polyMesh = rcAllocPolyMesh();
	if (!polyMesh)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
		return false;
	}
	if (!rcBuildPolyMesh(ctx, *contours, cfg.maxVertsPerPoly, *polyMesh))
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
		return false;
	}
	
	//
	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	//
	
	rcPolyMeshDetail* pmDetail = rcAllocPolyMeshDetail();
	if (!pmDetail)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmdtl'.");
		return false;
	}

	if (!rcBuildPolyMeshDetail(ctx, *polyMesh, *compactHf, cfg.detailSampleDist, cfg.detailSampleMaxError, *pmDetail))
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build detail mesh.");
		return false;
	}

	rcFreeCompactHeightfield(compactHf);
	compactHf = 0;
	rcFreeContourSet(contours);
	contours = 0;

	// At this point the navigation mesh data is ready, you can access it from polyMesh.
	// See duDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.
	
	//
	// (Optional) Step 8. Create Detour data from Recast poly mesh.
	//
	
	// The GUI may allow more max points per polygon than Detour can handle.
	// Only build the detour navmesh if we do not exceed the limit.

	if (cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
	{
		unsigned char* navData = 0;
		int navDataSize = 0;

		// Update poly flags from areas.
		for (int i = 0; i < polyMesh->npolys; ++i)
		{
			if (polyMesh->areas[i] == RC_WALKABLE_AREA)
				polyMesh->areas[i] = POLYAREA_GROUND;
				
			if (polyMesh->areas[i] == POLYAREA_GROUND ||
				polyMesh->areas[i] == POLYAREA_GRASS ||
				polyMesh->areas[i] == POLYAREA_ROAD)
			{
				polyMesh->flags[i] = POLYFLAGS_WALK;
			}
			else if (polyMesh->areas[i] == POLYAREA_WATER)
			{
				polyMesh->flags[i] = POLYFLAGS_SWIM;
			}
			else if (polyMesh->areas[i] == POLYAREA_DOOR)
			{
				polyMesh->flags[i] = POLYFLAGS_WALK | POLYFLAGS_DOOR;
			}
		}


		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = polyMesh->verts;
		params.vertCount = polyMesh->nverts;
		params.polys = polyMesh->polys;
		params.polyAreas = polyMesh->areas;
		params.polyFlags = polyMesh->flags;
		params.polyCount = polyMesh->npolys;
		params.nvp = polyMesh->nvp;
		params.detailMeshes = pmDetail->meshes;
		params.detailVerts = pmDetail->verts;
		params.detailVertsCount = pmDetail->nverts;
		params.detailTris = pmDetail->tris;
		params.detailTriCount = pmDetail->ntris;
		
		// TODO Is nav mesh links is required for mvp ?
		// params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
		// params.offMeshConRad = m_geom->getOffMeshConnectionRads();
		// params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
		// params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
		// params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
		// params.offMeshConUserID = m_geom->getOffMeshConnectionId();
		// params.offMeshConCount = m_geom->getOffMeshConnectionCount();
		params.walkableHeight = agent->heigth;
		params.walkableRadius = agent->radius;
		params.walkableClimb = agent->maxClimb;
		rcVcopy(params.bmin, polyMesh->bmin);
		rcVcopy(params.bmax, polyMesh->bmax);
		params.cs = cfg.cs;
		params.ch = cfg.ch;
		params.buildBvTree = true;
		
		if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
		{
			ctx->log(RC_LOG_ERROR, "Could not build Detour navmesh.");
			return false;
		}
		
		dtNavMesh* navMesh = dtAllocNavMesh();
		if (!navMesh)
		{
			dtFree(navData);
			ctx->log(RC_LOG_ERROR, "Could not create Detour navmesh");
			return false;
		}
		
		dtStatus status;
		
		status = navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
		if (dtStatusFailed(status))
		{
			dtFree(navData);
			ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh");
			return false;
		}
		
        dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
		if (!navQuery)
		{
			dtFree(navQuery);
			ctx->log(RC_LOG_ERROR, "Could not create Detour query");
			return false;
		}

		status = navQuery->init(navMesh, 2048);
		if (dtStatusFailed(status))
		{
			ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh query");
			return false;
		}
		navOut = std::make_unique<NavMesh>(navMesh, navQuery);
	} else {
		navOut.reset();
	}
	
	ctx->stopTimer(RC_TIMER_TOTAL);

	// Show performance stats.
	duLogBuildTimes(*ctx, ctx->getAccumulatedTime(RC_TIMER_TOTAL));
	ctx->log(RC_LOG_PROGRESS, ">> Polymesh: %d vertices  %d polygons", polyMesh->nverts, polyMesh->npolys);
	
	float totalBuildTimeMs = ctx->getAccumulatedTime(RC_TIMER_TOTAL)/1000.0f;

	return true;
}