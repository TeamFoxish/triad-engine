#pragma once

#include <vector>
#include "misc/Strid.h"

class Resource {
	Strid tag;
	int loadRefs = 0;
	std::vector<Strid> refs;
};
