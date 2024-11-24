#include "SharedStorage.h"

bool InitSharedStorage()
{
    SharedStorage::instance = std::make_unique<SharedStorage>();
    return true;
}

void TermSharedStorage()
{
    SharedStorage::instance.reset();
}
