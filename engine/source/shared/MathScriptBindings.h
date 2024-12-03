#pragma once

#include "TransformStorage.h"

class CRef {
public:
	CRef() = default;
	// TODO: fix ability to copy?
	CRef(const CRef&) = delete;
	virtual ~CRef() = default;

	void AddRef();
	void Release();

private:
	int refCount = 1;
};

class CTransformHandle : public CRef {
public:
	CTransformHandle();
	~CTransformHandle();

	TransformStorage::Handle GetHandle() const { return handle; }
	const Math::Transform& GetTransform() const;
	Math::Transform& GetTransform();

	Math::Vector3 GetPosition() const { return GetTransform().GetPosition(); }
	void SetPosition(Math::Vector3 pos) { GetTransform().SetPosition(pos); }

	Math::Quaternion GetRotation() const { return GetTransform().GetRotation(); }
	void SetRotation(const Math::Quaternion& rot) { GetTransform().SetRotation(rot); }

	Math::Vector3 GetScale() const { return GetTransform().GetScale(); }
	void SetScale(Math::Vector3 scale) { GetTransform().SetScale(scale); }

	Math::Vector3 GetLocalPosition() const { return GetTransform().GetLocalPosition(); }
	void SetLocalPosition(Math::Vector3 pos) { GetTransform().SetLocalPosition(pos); }

	Math::Quaternion GetLocalRotation() const { return GetTransform().GetLocalRotation(); }
	void SetLocalRotation(const Math::Quaternion& rot) { GetTransform().SetLocalRotation(rot); }

	Math::Vector3 GetLocalScale() const { return GetTransform().GetLocalScale(); }
	void SetLocalScale(Math::Vector3 scale) { GetTransform().SetLocalScale(scale); }

protected:
	TransformStorage::Handle handle;
};

bool MathScriptBindingsInit();
void MathScriptBindingsTerm();
