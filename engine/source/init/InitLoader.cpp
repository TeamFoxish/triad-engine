#include "InitLoader.h"

InitLoader::InitLoader()
{
	FACTORY_INIT;
}

InitLoader::~InitLoader()
{
}

void InitLoader::Load(ResTag tag, const YAML::Node& desc)
{
	if (startUpSceneTag == 0) {
		const std::string sceneTag = desc["startup-scene"].Scalar();
		startUpSceneTag = ToStrid(sceneTag);
	}
}
