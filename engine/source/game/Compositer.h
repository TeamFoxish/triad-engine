#pragma once

#include "math/Math.h"
#include <initializer_list>

template<typename CHILD_T>
class Compositer_T {
public:
	//BASE_T* CastToBaseType() { return static_cast<BASE_T*>(this); }
	virtual class Game* GetGamePtr() const = 0;

	virtual void AddChild(const std::initializer_list<CHILD_T*>& batch) = 0;
	virtual void RemoveChild(CHILD_T* comp) = 0;

	virtual Math::Vector3 GetPosition() const = 0;
	virtual void SetPosition(Math::Vector3 pos) = 0;

	virtual Math::Quaternion GetRotation() const = 0;
	virtual void SetRotation(Math::Quaternion rot) = 0;

	virtual Math::Vector3 GetScale() const = 0;
	virtual void SetScale(Math::Vector3 scale) = 0;

	virtual Math::Vector3 GetForward() const = 0;
	virtual Math::Vector3 GetRight() const = 0;

	virtual const Math::Matrix& GetWorldTransform(Compositer_T* parent = nullptr) = 0;
	//virtual void ComputeWorldTransform() = 0;
};
