#pragma once

#include "render/GeometryData.h"
#include "NavMeshAgent.h"
#include "NavMeshBuilder.h"
#include "NavMesh.h"
#include "runtime/RuntimeIface.h"
#include <vector>
#include <unordered_map>
#include <string>

#ifdef EDITOR
#include "NavMeshDbgDraw.h"
#endif

class NavMeshSystem {
    public:
        NavMeshSystem() {};
        ~NavMeshSystem() {};
        bool Init(RuntimeIface* runtime);
        void Term();
        std::vector<const Renderable*> CollectStaticObjects();
        const NavMeshBuilder& GetBuilder() const { return *_builder; }
        NavMeshBuilder& GetBuilder() { return *_builder; }
        void Build(BuildConfig config, NavMeshAgent* agent);
        const NavMesh* GetNavMesh(const NavMeshAgent& agent) const;
        std::vector<float> FindPath(const NavMeshAgent& agent, const float* startPos, const float* endPos);

#ifdef EDITOR
        bool IsDebugDrawEnabled() const { return dbgDrawEnabled; }
        void GenerateTestPath(NavMeshAgent* agent, float* startPos, float* endPos);
        void DebugDraw();
#endif // EDITOR

    private:
        std::unique_ptr<NavMeshBuilder> _builder;
        std::unordered_map<NavMeshAgent, std::unique_ptr<NavMesh>> _navMeshes;
        
#ifdef EDITOR
        std::unique_ptr<NavMeshDbgDraw> dbgDraw;
        std::vector<float> _testPath;
        bool dbgDrawEnabled = true;
#endif // EDITOR
};

bool InitNavigation(RuntimeIface* runtime);
void TermNavigation(RuntimeIface* runtime);

extern std::unique_ptr<NavMeshSystem> gNavigation;