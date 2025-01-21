#include "NavMesh.h"
#include "Recast.h"
#include <filesystem>
#include "logs/Logs.h"

NavMesh::NavMesh(dtNavMesh *navMesh, dtNavMeshQuery *query) : _navMesh(navMesh), _query(query)
{
}

NavMesh::~NavMesh()
{
    dtFreeNavMesh(_navMesh);
    dtFreeNavMeshQuery(_query);
}

void NavMesh::save(std::string path)
{
    FILE* file = fopen(path.c_str(), "wb");
    if (!file) {
        LOG_ERROR("Could not write nav mesh data to {}", path);
    }

    if (!_navMesh) return;

	// Store header.
	NavMeshSetHeader header;
	header.magic = NAVMESHSET_MAGIC;
	header.version = NAVMESHSET_VERSION;
	header.numTiles = 0;
	for (int i = 0; i < _navMesh->getMaxTiles(); ++i)
	{
		const dtMeshTile* tile = const_cast<const dtNavMesh*>(_navMesh)->getTile(i);
		if (!tile || !tile->header || !tile->dataSize) continue;
		header.numTiles++;
	}
	memcpy(&header.params, _navMesh->getParams(), sizeof(dtNavMeshParams));
	fwrite(&header, sizeof(NavMeshSetHeader), 1, file);

	// Store tiles.
	for (int i = 0; i < _navMesh->getMaxTiles(); ++i)
	{
		const dtMeshTile* tile = const_cast<const dtNavMesh*>(_navMesh)->getTile(i);
		if (!tile || !tile->header || !tile->dataSize) continue;

		NavMeshTileHeader tileHeader;
		tileHeader.tileRef = _navMesh->getTileRef(tile);
		tileHeader.dataSize = tile->dataSize;
		fwrite(&tileHeader, sizeof(tileHeader), 1, file);

		fwrite(tile->data, tile->dataSize, 1, file);
	}

	fclose(file);
}
