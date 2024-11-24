#pragma once

#include "Math.h"

namespace Math {
class Transform {
public:
	Transform() = default;
	Transform(const Math::Matrix& matr); // transform at root
	Transform(const Math::Matrix& origin, const Math::Matrix& localToOrigin); // transform with parent

	const Math::Matrix& GetMatrix() const;
	void SetMatrix(const Math::Matrix& matr);

	Math::Vector3 GetPosition() const;
	void SetPosition(Math::Vector3 pos);

	Math::Quaternion GetRotation() const;
	void SetRotation(const Math::Quaternion& rot);

	Math::Vector3 GetScale() const;
	void SetScale(Math::Vector3 scale);

	Math::Matrix GetLocalMatrix() const;
	void SetLocalMatrix(const Math::Matrix& matr);

	Math::Vector3 GetLocalPosition() const { return localPos; }
	void SetLocalPosition(Math::Vector3 pos);

	Math::Quaternion GetLocalRotation() const { return localRot; }
	void SetLocalRotation(const Math::Quaternion& rot);

	Math::Vector3 GetLocalScale() const { return localScale; }
	void SetLocalScale(Math::Vector3 scale);

private:
	void UpdateMatrix() const;

private:
	mutable Math::Matrix worldMatr;    // model matrix
	mutable Math::Matrix localToWorld; // view matrix
	Math::Vector3 localPos;
	Math::Quaternion localRot;
	Math::Vector3 localScale = Math::Vector3::One;

	mutable bool updateMatrix = false;
	bool hasParent = false;
};
}
