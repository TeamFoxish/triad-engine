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
        const std::unordered_map<std::string, NavMeshAgent*>& GetAgents() { return _agents; }
        NavMeshAgent* GetAgent(const std::string& name) { return _agents[name]; }
        void AddAgent(const std::string& name, NavMeshAgent* agent) { _agents[name] = agent; }
        NavMesh& GetNavMesh(NavMeshAgent* agent) { return *_navMeshes[agent]; }
        std::vector<float> FindPath(NavMeshAgent* agent, float* startPos, float* endPos);

#ifdef EDITOR
        bool IsDebugDrawEnabled() const { return dbgDrawEnabled; }
        void DebugDraw();
#endif // EDITOR

    private:
        std::unique_ptr<NavMeshBuilder> _builder;
        std::unordered_map<std::string, NavMeshAgent*> _agents;
        std::unordered_map<NavMeshAgent*, std::unique_ptr<NavMesh>> _navMeshes;
        
#ifdef EDITOR
        std::unique_ptr<NavMeshDbgDraw> dbgDraw;
        bool dbgDrawEnabled = true;
#endif // EDITOR
};

bool InitNavigation(RuntimeIface* runtime);
void TermNavigation(RuntimeIface* runtime);

extern std::unique_ptr<NavMeshSystem> gNavigation;