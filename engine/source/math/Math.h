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
	inline T Abs(T val)
	{
		return std::abs(val);
	}

	template<typename T>
	inline T Clamp(T val, T min, T max) 
	{
		return Min(max, Max(val, min));
	}

	inline constexpr float DegToRad(float deg)
	{
		constexpr float mult = Pi / 180;
		return deg * mult;
	}

	inline constexpr Vector3 DegToRad(Vector3 euler)
	{
		return Vector3(DegToRad(euler.x), DegToRad(euler.y), DegToRad(euler.z));
	}

	inline constexpr float RadToDeg(float rad)
	{
		constexpr float mult = 180 / Pi;
		return rad * mult;
	}

	inline constexpr Vector3 RadToDeg(Vector3 eulerRad)
	{
		return Vector3(RadToDeg(eulerRad.x), RadToDeg(eulerRad.y), RadToDeg(eulerRad.z));
	}
};
