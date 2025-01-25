#pragma once

#include <string>

#include "misc/Hash.hpp"

class NavMeshAgent {
    public:
		std::string name;
		float height = 2.0f;
		float radius = 0.6f;
		float maxClimb = 0.9f;
		float maxSlope = 45.0f;
};

inline bool operator==(const NavMeshAgent& lhs, const NavMeshAgent& rhs) 
{
	return lhs.name == rhs.name && lhs.height == rhs.height && lhs.radius == rhs.radius && lhs.maxClimb == rhs.maxClimb && lhs.maxSlope == rhs.maxSlope;
}

namespace std {
	template <> 
	struct hash<NavMeshAgent> {
		hash() = default;

		std::size_t operator()(const NavMeshAgent& agent) const noexcept {
			std::size_t h{0};
			hashCombine(h, agent.name, agent.height, agent.radius, agent.maxClimb, agent.maxSlope);
			return h;
		}
	};
}
