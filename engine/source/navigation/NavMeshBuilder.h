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
	float cellSize;
	float cellHeigth;
};

struct RegionConfig {
	float minRegionSize;
	float mergedRegionSize;
};

struct PolygonizationConfig {
	float maxEdgeLength;
	float maxEdgeError;
	uint16_t vertsPerPoly;
};

struct DetailMesh {
	uint16_t sampleDistance;
	uint16_t maxSampleError;
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
    float bMin[3];
    float bMax[3];
    bool filterLowHangingObstacles;
    bool filterLedgeSpans;
    bool filterWalkableLowHeightSpans;
    NavMeshPartitionType partitionType;
	RasterizationConfig rasterization;
	RegionConfig region;
	PolygonizationConfig polygonization;
	DetailMesh detail;
	std::vector<OffMeshConnection> OffMeshConnections;
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
       	bool buildNavMesh(std::vector<Renderable> meshes, NavMeshAgent* agent, BuildConfig bconfig, std::vector<ConvexVolume> convexVolumes, NavMesh* nav);
    private:

};