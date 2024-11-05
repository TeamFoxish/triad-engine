#include "ResourceInit.h"

#include "ResourceSystem.h"

bool InitResource(RuntimeIface* runtime) 
{
	assert(gResourceSys == nullptr);
	gResourceSys = std::make_unique<ResourceSystem>();
	return gResourceSys->Init(runtime);
}

void TermResource(RuntimeIface* runtime) 
{
	assert(gResourceSys);
}
