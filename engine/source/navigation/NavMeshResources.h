#pragma once

#include "resource/ResourceLoader.h"
#include "misc/Factory.h"

#include "NavMeshAgent.h"

#include <memory>

class NavMeshResources {
	friend void NavMeshResourcesInit();
	friend void NavMeshResourcesTerm();

public:
	NavMeshResources() = default;
	NavMeshResources(const NavMeshResources&) = delete;
	NavMeshResources(NavMeshResources&&) = delete;
	~NavMeshResources() = default;

	static NavMeshResources& Instance() { return *instance; }

public:
	std::unordered_map<ResTag, NavMeshAgent> agents;

private:
	static inline std::unique_ptr<NavMeshResources> instance;
};

class NavMeshAgentLoader : public ResourceLoader, RegisteredInFactory<ResourceLoader, NavMeshAgentLoader> {
public:
	NavMeshAgentLoader();

	void Load(ResTag tag, const YAML::Node& desc) override;
	void Unload(ResTag tag) override;

	static std::unique_ptr<ResourceLoader> CreateInstance()
	{
		return std::make_unique<NavMeshAgentLoader>();
	}

	static const char* GetFactoryKey()
	{
		return "agent";
	}

private:
	static NavMeshAgent Deserialize(const YAML::Node& desc);
};

void NavMeshResourcesInit();

void NavMeshResourcesTerm();
