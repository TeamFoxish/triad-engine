#include "NavMeshSystem.h"
#include "scripts/ScriptSystem.h"
#include "logs/Logs.h"
#include <vector>

std::unique_ptr<NavMeshSystem> gNavigation;

bool NavMeshSystem::Init(RuntimeIface *runtime)
{
    this->_builder = std::make_unique<NavMeshBuilder>();
    return true;
}

void NavMeshSystem::Term()
{
    _builder.reset();
}

std::vector<const Renderable*> NavMeshSystem::CollectStaticObjects()
{
    std::vector<const Renderable*> staticObjs;

    for (const Renderable& obj : RenderableStorage::Instance().GetStorage()) {
        if (!obj.params.isStatic) {
            continue;
        }
        staticObjs.push_back(&obj);
    }

    return staticObjs;
}

void NavMeshSystem::Build(BuildConfig config, NavMeshAgent *agent)
{
    std::unique_ptr<NavMesh> nav;
    const std::vector<const Renderable*> staticObjs = CollectStaticObjects();
    if (staticObjs.empty()) {
        LOG_WARN("failed to build navmesh. no static object were found on scene");
        return;
    }
    _builder->buildNavMesh(staticObjs, agent, config, std::vector<ConvexVolume> {}, nav);
    if (!nav) {
        return;
    }
    NavMesh& navRef = *(_navMeshes[agent] = std::move(nav));
    navRef.save(agent->name + ".bin");
}

std::vector<float> NavMeshSystem::FindPath(NavMeshAgent *agent, float *startPos, float *endPos)
{
    NavMesh& nav = GetNavMesh(agent);
    float extents[3] = {agent->radius, agent->radius, agent->heigth};
    dtQueryFilter filter;
    dtPolyRef startRef, endRef;
    nav.GetNavMeshQuery()->findNearestPoly(startPos, extents, &filter, &startRef, nullptr);
    nav.GetNavMeshQuery()->findNearestPoly(endPos, extents, &filter, &endRef, nullptr);

    dtPolyRef path[256];
    int pathCount;
    nav.GetNavMeshQuery()->findPath(startRef, endRef, startPos, endPos, &filter, path, &pathCount, 256);
    
    float straightPath[256 * 3]; // Array to hold the straight path (x, y, z for each point)
    unsigned char straightPathFlags[256]; // Flags for each point
    dtPolyRef straightPathPolys[256];     // Polygons associated with each point
    int straightPathCount;

    dtStatus status = nav.GetNavMeshQuery()->findStraightPath(startPos, endPos, path, pathCount, straightPath, straightPathFlags, straightPathPolys, &straightPathCount, 256);
    if (dtStatusFailed(status)) {
        LOG_WARN("Failed to find straight path.");
        return {};
    }

    return std::vector<float>(straightPath, straightPath + (sizeof(straightPath) / sizeof(straightPath[0])));
}

bool InitNavigation(RuntimeIface *runtime)
{
    assert(gNavigation == nullptr);
    gNavigation = std::make_unique<NavMeshSystem>();
    return gNavigation->Init(runtime);
}

void TermNavigation(RuntimeIface *runtime)
{
    assert(gNavigation);
    gNavigation->Term();
    gNavigation = nullptr;
}
