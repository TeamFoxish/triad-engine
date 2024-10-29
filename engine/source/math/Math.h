#pragma once

#include <SimpleMath.h>

namespace Math {
	using namespace DirectX::SimpleMath;

	using Sphere = DirectX::BoundingSphere;
	using Box = DirectX::BoundingBox;
	using OrientedBox = DirectX::BoundingOrientedBox;
	using Frustum = DirectX::BoundingFrustum;

	constexpr float Pi = DirectX::XM_PI;

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
