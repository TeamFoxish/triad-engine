#pragma once

#include "render/GeometryData.h"
#include "render/Renderable.h"
#include "NavMeshAgent.h"
#include "NavMesh.h"
#include <memory>

enum PolyAreas
{
	POLYAREA_GROUND,
	POLYAREA_WATER,
	POLYAREA_ROAD,
	POLYAREA_DOOR,
	POLYAREA_GRASS,
	POLYAREA_JUMP
};

enum PolyFlags
{
	POLYFLAGS_WALK		= 0x01,		// Ability to walk (ground, grass, road)
	POLYFLAGS_SWIM		= 0x02,		// Ability to swim (water).
	POLYFLAGS_DOOR		= 0x04,		// Ability to move through doors.
	POLYFLAGS_JUMP		= 0x08,		// Ability to jump.
	POLYFLAGS_DISABLED	= 0x10,		// Disabled polygon
	POLYFLAGS_ALL		= 0xffff	// All abilities.
};

enum NavMeshPartitionType
{
	PARTITION_WATERSHED,
	PARTITION_MONOTONE,
	PARTITION_LAYERS
};

struct RasterizationConfig {
	float cellSize = 0.30f;
	float cellHeigth = 0.20f;
};

struct RegionConfig {
	float minRegionSize = 8.0f;
	float mergedRegionSize = 20.0f;
};

struct PolygonizationConfig {
	float maxEdgeLength = 12.0f;
	float maxEdgeError = 1.3f;
	float vertsPerPoly = 6.0f;
};

struct DetailMesh {
	float sampleDistance = 6.0f;
	float maxSampleError = 1.0f;
};

struct OffMeshConnection {
	float start[3];
	float end[3];
	float radius;
	unsigned char direction = DT_OFFMESH_CON_BIDIR;
	unsigned char area;
	unsigned short flag;
	unsigned int userId;
};

struct BuildConfig {
    float bMin[3] = {-1000, -1000, -1000};
    float bMax[3] = {1000, 1000, 1000};
    bool filterLowHangingObstacles = true;
    bool filterLedgeSpans = true;
    bool filterWalkableLowHeightSpans = true;
    NavMeshPartitionType partitionType = PARTITION_WATERSHED;
	RasterizationConfig rasterization = {};
	RegionConfig region = {};
	PolygonizationConfig polygonization = {};
	DetailMesh detail = {};
	std::vector<OffMeshConnection> OffMeshConnections = {};
};

static const int MAX_CONVEXVOL_PTS = 12;
// As far as I undertand - this is like Unreal Engine VolumeModifier 
// (mark area as more difficult for navigate or for swimming and etc.)
struct ConvexVolume
{
	float verts[MAX_CONVEXVOL_PTS*3];
	float hmin, hmax;
	int nverts;
	int area;
};

class NavMeshBuilder {
    public:
		NavMeshBuilder();
		~NavMeshBuilder();
       	bool buildNavMesh(std::vector<const Renderable*> meshes, NavMeshAgent* agent, BuildConfig bconfig, std::vector<ConvexVolume> convexVolumes, std::unique_ptr<NavMesh>& navOut);
		// Not thread-safe
		BuildConfig& GetCurrentConfig() { return currentConfig; }
    private:
		BuildConfig currentConfig;
};