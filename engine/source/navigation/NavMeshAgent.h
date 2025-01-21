#pragma once

#include <string>
class NavMeshAgent {
    public:
		const std::string name;
		float heigth = 2.0f;
		float radius = 0.6f;
		float maxClimb = 0.9f;
		float maxSlope = 45.0f;
};