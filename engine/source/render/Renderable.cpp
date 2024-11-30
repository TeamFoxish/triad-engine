#include "Renderable.h"

#include "shared/SharedStorage.h"

Renderable::Renderable(uint32_t _entityId, TransformStorage::Handle _transform)
	: transform(_transform)
	, entityId(_entityId)
{
}

bool InitRenderableStorage()
{
	RenderableStorage::instance = std::make_unique<RenderableStorage>();
	return true;
}

void TermRenderableStorage()
{
	RenderableStorage::instance.reset();
}
