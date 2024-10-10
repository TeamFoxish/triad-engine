#pragma once

#include <SimpleMath.h>

namespace Math {
	using namespace DirectX::SimpleMath;

	constexpr float Pi = 3.1415926535f;

	template<typename T>
	inline T Min(T a, T b)
	{
		if (a < b) {
			return a;
		}
		return b;
	}

	template<typename T>
	inline T Max(T a, T b) 
	{
		if (a > b) {
			return a;
		}
		return b;
	}

	template<typename T>
	inline T Clamp(T val, T min, T max) 
	{
		return Min(max, Max(val, min));
	}
};
