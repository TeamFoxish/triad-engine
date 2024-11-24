#pragma once

#include "game/Component.h"

#include <vector>
#include <initializer_list>

class CompositeComponent : public Component, public Compositer {
public:
	CompositeComponent(Game* game, Compositer* compositer = nullptr);
	~CompositeComponent();

	Game* GetGamePtr() const override { return GetGame(); }

	// TODO: check if component is already in children list
	void AddChild(const std::initializer_list<Component*>& batch) override { children.insert(children.end(), batch.begin(), batch.end()); }
	void RemoveChild(Component* comp) override;

	void AddChild(CompositeComponent* comp);

	void ProceedInput(InputDevice* inpDevice) override;
	void Initialize(Compositer* parent = nullptr) override;
	void Update(float deltaTime, Compositer* parent = nullptr) override;

	Math::Vector3 GetPosition() const override;
	void SetPosition(Math::Vector3 pos) override;

	Math::Quaternion GetRotation() const override;
	void SetRotation(Math::Quaternion rot) override;

	Math::Vector3 GetScale() const override;
	void SetScale(Math::Vector3 scale) override;

	Math::Vector3 GetForward() const override;
	Math::Vector3 GetRight() const override;

	const Math::Matrix& GetWorldTransform(Compositer* parent = nullptr) override;
	void SetWorldTransform(const Math::Matrix& matr);
	void SetLocalTransform(const Math::Matrix& matr);

	std::vector<Component*> GetChildren() const { return children; }

	TransformStorage::Handle GetTransformHandle() const override { return transformHandle; }

public:
	float boundingSphereRadius = 0.0f;

protected:
	std::vector<Component*> children;

	TransformStorage::Handle transformHandle;
};
