#include "NavMeshSystem.h"
#include "DetourDebugDraw.h"
#include "logs/Logs.h"
#include <vector>

std::unique_ptr<NavMeshSystem> gNavigation;

bool NavMeshSystem::Init(RuntimeIface *runtime)
{
#ifdef EDITOR
    dbgDraw = std::make_unique<NavMeshDbgDraw>();
#endif // EDITOR

    extern void NavMeshResourcesInit();
    NavMeshResourcesInit();

    this->_builder = std::make_unique<NavMeshBuilder>();
    return true;
}

void NavMeshSystem::Term()
{
    extern void NavMeshResourcesTerm();
    NavMeshResourcesTerm();

    _builder.reset();
#ifdef EDITOR
    dbgDraw.reset();
#endif // EDITOR
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
    NavMesh& navRef = *(_navMeshes[*agent] = std::move(nav));
    navRef.save(agent->name + ".bin");
}

const NavMesh* NavMeshSystem::GetNavMesh(const NavMeshAgent& agent) const
{
    const auto iter = _navMeshes.find(agent);
    return iter != _navMeshes.end() ? iter->second.get() : nullptr;
}

std::vector<float> NavMeshSystem::FindPath(const NavMeshAgent& agent, const float *startPos, const float *endPos)
{
    const NavMesh* nav = GetNavMesh(agent);
    if (!nav) {
        // log error
        return std::vector<float>();
    }
    float extents[3] = {agent.radius, agent.radius, agent.height};
    dtQueryFilter filter;
    dtPolyRef startRef, endRef;
    nav->GetNavMeshQuery()->findNearestPoly(startPos, extents, &filter, &startRef, nullptr);
    nav->GetNavMeshQuery()->findNearestPoly(endPos, extents, &filter, &endRef, nullptr);

    dtPolyRef path[256];
    int pathCount;
    nav->GetNavMeshQuery()->findPath(startRef, endRef, startPos, endPos, &filter, path, &pathCount, 256);
    
    float straightPath[256 * 3]; // Array to hold the straight path (x, y, z for each point)
    unsigned char straightPathFlags[256]; // Flags for each point
    dtPolyRef straightPathPolys[256];     // Polygons associated with each point
    int straightPathCount;

    dtStatus status = nav->GetNavMeshQuery()->findStraightPath(startPos, endPos, path, pathCount, straightPath, straightPathFlags, straightPathPolys, &straightPathCount, 256);
    if (dtStatusFailed(status)) {
        LOG_WARN("Failed to find straight path.");
        return {};
    }

    return std::vector<float>(straightPath, straightPath + (straightPathCount * 3));
}

#ifdef EDITOR
void NavMeshSystem::GenerateTestPath(NavMeshAgent *agent, float *startPos, float *endPos)
{
    _testPath.clear();
    _testPath = FindPath(*agent, startPos, endPos);
}

void NavMeshSystem::DebugDraw()
{
    if (!dbgDraw) {
        return;
    }
    for (const auto& [_, navMesh] : _navMeshes) {
        duDebugDrawNavMesh(dbgDraw.get(), *navMesh->GetNavMesh(), DrawNavMeshFlags{});
    }
    if (!_testPath.empty()) {
        for (int i = 0; i < _testPath.size() - 3; i += 3) {
            duDebugDrawArrow(dbgDraw.get(),
             _testPath[i], _testPath[i + 1], _testPath[i + 2],
            _testPath[i + 3], _testPath[i +  4], _testPath[i + 5], 0, 1, 200000000, 5);
        }
    }
}
#endif // EDITOR

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
