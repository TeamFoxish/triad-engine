#include "UIStorage.h"

#include "shared/SharedStorage.h"

bool InitUIStorage()
{
	UIStorage::instance = std::make_unique<UIStorage>();
	return true;
}

void TermUIStorage()
{
	UIStorage::instance.reset();
}
