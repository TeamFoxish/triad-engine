#include "LightsStorage.h"

bool InitLightsStorage()
{
    LightsStorage::instance = std::make_unique<LightsStorage>();
    return true;
}

void TermLightsStorage()
{
    LightsStorage::instance.reset();
}
