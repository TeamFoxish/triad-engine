#include "RenderStorage.h"

bool InitRenderStorage()
{
    RenderStorage::instance = std::make_unique<RenderStorage>();
    return true;
}

void TermRenderStorage()
{
    RenderStorage::instance.reset();
}
