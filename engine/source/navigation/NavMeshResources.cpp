#include "NavMeshResources.h"

NavMeshAgentLoader::NavMeshAgentLoader()
{
    FACTORY_INIT;
}

void NavMeshAgentLoader::Load(ResTag tag, const YAML::Node& desc)
{
    const NavMeshAgent agent = Deserialize(desc);
    NavMeshResources::Instance().agents.insert(std::make_pair(tag, agent));
}

void NavMeshAgentLoader::Unload(ResTag tag)
{
    NavMeshResources::Instance().agents.erase(tag);
}

NavMeshAgent NavMeshAgentLoader::Deserialize(const YAML::Node& desc)
{
    NavMeshAgent agent(desc["name"] ? desc["name"].Scalar() : "UnnamedAgent");

    if (desc["height"]) {
        agent.height = desc["height"].as<float>();
    }
    if (desc["radius"]) {
        agent.radius = desc["radius"].as<float>();
    }
    if (desc["maxClimb"]) {
        agent.maxClimb = desc["maxClimb"].as<float>();
    }
    if (desc["maxSlope"]) {
        agent.maxSlope = desc["maxSlope"].as<float>();
    }

    return agent;
}

void NavMeshResourcesInit()
{
    NavMeshResources::instance = std::make_unique<NavMeshResources>();
}

void NavMeshResourcesTerm()
{
    NavMeshResources::instance.reset();
}
