#include "Transform.h"

using namespace Math;

Transform::Transform(const Matrix& _matr)
	: hasParent(false)
	, updateMatrix(true)
{
	SetMatrix(_matr);
}

Transform::Transform(const Matrix& origin, const Matrix& localToOrigin)
	: hasParent(true)
{
	worldMatr = localToOrigin * origin;
	const_cast<Matrix&>(localToOrigin).Decompose(localScale, localRot, localPos);
	localToWorld = localToOrigin.Invert();
}

const Matrix& Transform::GetMatrix() const
{
	if (updateMatrix) {
		UpdateMatrix();
	}
	return worldMatr;
}

void Transform::SetMatrix(const Matrix& _matr)
{
	if (!hasParent) {
		const_cast<Matrix&>(_matr).Decompose(localScale, localRot, localPos);
		worldMatr = _matr;
		updateMatrix = false;
		return;
	}
	UpdateMatrix();
	const Matrix origin = localToWorld * GetMatrix();
	worldMatr = _matr;
	Matrix localMatr = worldMatr * origin.Invert();
	localMatr.Decompose(localScale, localRot, localPos);
	localToWorld = localMatr.Invert();
}

Math::Vector3 Math::Transform::GetPosition() const
{
	if (!hasParent) {
		return localPos;
	}
	UpdateMatrix();
	return worldMatr.Translation();
}

void Math::Transform::SetPosition(Math::Vector3 pos)
{
	if (!hasParent) {
		SetLocalPosition(pos);
		return;
	}
	Matrix matr = GetMatrix();
	matr.Translation(pos);
	SetMatrix(matr);
}

Math::Quaternion Math::Transform::GetRotation() const
{
	if (!hasParent) {
		return localRot;
	}
	UpdateMatrix();
	Math::Vector3 dummyScale;
	Math::Quaternion rot;
	Math::Vector3 dummyPos;
	worldMatr.Decompose(dummyScale, rot, dummyPos);
	return rot;
}

void Transform::SetRotation(const Quaternion& rot)
{
	if (!hasParent) {
		SetLocalRotation(rot);
		return;
	}
	UpdateMatrix();
	Math::Vector3 scale;
	Math::Quaternion oldRot;
	Math::Vector3 pos;
	worldMatr.Decompose(scale, oldRot, pos);
	Matrix matr = Matrix::CreateScale(scale);
	matr *= Matrix::CreateFromQuaternion(rot);
	matr *= Matrix::CreateTranslation(pos);
	SetMatrix(matr);
}

Math::Vector3 Math::Transform::GetScale() const
{
	if (!hasParent) {
		return localScale;
	}
	UpdateMatrix();
	Math::Vector3 scale;
	Math::Quaternion dummyRot;
	Math::Vector3 dummyPos;
	worldMatr.Decompose(scale, dummyRot, dummyPos);
	return scale;
}

void Transform::SetScale(Vector3 scale)
{
	if (!hasParent) {
		SetLocalScale(scale);
		return;
	}
	UpdateMatrix();
	Math::Vector3 oldScale;
	Math::Quaternion rot;
	Math::Vector3 pos;
	worldMatr.Decompose(oldScale, rot, pos);
	Matrix matr = Matrix::CreateScale(scale);
	matr *= Matrix::CreateFromQuaternion(rot);
	matr *= Matrix::CreateTranslation(pos);
	SetMatrix(matr);
}

Math::Matrix Math::Transform::GetLocalMatrix() const
{
	if (!hasParent) {
		return GetMatrix();
	}
	Matrix localMatr = Matrix::CreateScale(localScale);
	localMatr *= Matrix::CreateFromQuaternion(localRot);
	localMatr *= Matrix::CreateTranslation(localPos);
	return localMatr;
}

void Transform::SetLocalMatrix(const Matrix& matr)
{
	const_cast<Matrix&>(matr).Decompose(localScale, localRot, localPos);
	if (!hasParent) {
		worldMatr = matr;
		updateMatrix = false;
		return;
	}
	updateMatrix = true;
}

void Transform::SetLocalPosition(Vector3 pos)
{
	localPos = pos;
	updateMatrix = true;
}

void Math::Transform::SetLocalRotation(const Math::Quaternion& rot)
{
	localRot = rot;
	updateMatrix = true;
}

void Math::Transform::SetLocalScale(Math::Vector3 scale)
{
	localScale = scale;
	updateMatrix = true;
}

void Transform::UpdateMatrix() const
{
	if (!updateMatrix) {
		return;
	}
	if (!hasParent) {
		// TODO: replace with GetLocalMatrix
		worldMatr = Matrix::CreateScale(localScale);
		worldMatr *= Matrix::CreateFromQuaternion(localRot);
		worldMatr *= Matrix::CreateTranslation(localPos);
		updateMatrix = false;
		return;
	}
	const Matrix origin = localToWorld * worldMatr;
	const Matrix local = GetLocalMatrix();
	worldMatr = local * origin;
	localToWorld = local.Invert();
	updateMatrix = false;
}
