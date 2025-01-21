#pragma once

#include "render/GeometryData.h"
#include "NavMeshAgent.h"
#include "NavMesh.h"
#include <vector>
#include <unordered_map>
#include <string>

class NavMeshSystem {
    public:
        NavMeshSystem();
        ~NavMeshSystem();
        std::vector<GeometryData> collectStaticGeometry();
        
    
    private:
        std::unordered_map<std::string, NavMeshAgent*> _agents;
        std::unordered_map<NavMeshAgent*, NavMesh*> _navMeshes;
};