#pragma once

#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include <string>

static const int NAVMESHSET_MAGIC = 'M'<<24 | 'S'<<16 | 'E'<<8 | 'T'; //'MSET';
static const int NAVMESHSET_VERSION = 1;

struct NavMeshSetHeader
{
	int magic;
	int version;
	int numTiles;
	dtNavMeshParams params;
};

struct NavMeshTileHeader
{
	dtTileRef tileRef;
	int dataSize;
};

class NavMesh {
    private:
        dtNavMesh* _navMesh;
		dtNavMeshQuery* _query;

	public:
		NavMesh(dtNavMesh* navMesh, dtNavMeshQuery* query);
		~NavMesh();
		void save(std::string path);
		dtNavMesh* GetNavMesh() { return _navMesh; }
		void SetNavMesh(dtNavMesh* navMesh) { _navMesh = navMesh; }
		dtNavMeshQuery* GetNavMeshQuery() const { return _query; }
		void SetNavMeshQuery(dtNavMeshQuery* query) { _query = query; }
};
