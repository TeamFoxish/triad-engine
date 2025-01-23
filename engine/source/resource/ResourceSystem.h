#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include "Res.h"

class RuntimeIface;

class ResourceSystem {
public:
	ResourceSystem() = default;
	ResourceSystem(const ResourceSystem&) = delete;
	ResourceSystem(ResourceSystem&&) = delete;
	~ResourceSystem() = default;

	bool Init(RuntimeIface* runtime) { return true; }
	void Term() {}

	bool IsResourceLoaded(ResTag tag) const { return resources.find(tag) != resources.end(); }
	void LoadResource(ResTag tag);
	void UnloadResource(ResTag tag);

	bool ResolveTagToFile(ResTag tag, ResPath& outPath) const;

protected:
	void LoadResourceImpl(ResTag tag);

	bool ResolveTagToFile(ResTag tag, const ResPath& root, ResPath& outPath) const;

protected:
	struct Resource {
		ResTag tag; // TODO: remove. use map key instead?
		int loadRefs = 0;
		Strid type;
		std::vector<ResTag> refs;
	};

	std::unordered_map<ResTag, Resource> resources;
};

extern std::unique_ptr<ResourceSystem> gResourceSys;
