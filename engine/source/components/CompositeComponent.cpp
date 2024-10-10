#include "CompositeComponent.h"

#include <algorithm>

CompositeComponent::CompositeComponent(Game* game, Compositer* compositer)
	: Component(game, compositer)
{
}

CompositeComponent::~CompositeComponent()
{
	for (Component* child : children) {
		delete child;
	}
	children.clear();
}

void CompositeComponent::RemoveChild(Component* comp)
{
	// TODO: allow removing child in update by add pending list
	auto iter = std::find(children.begin(), children.end(), comp);
	if (iter != children.end()) {
		children.erase(iter);
	}
}

void CompositeComponent::ProceedInput(InputDevice* inpDevice)
{
	for (Component* child : children) {
		child->ProceedInput(inpDevice);
	}
}

void CompositeComponent::Initialize(Compositer* parent)
{
	for (Component* child : children) {
		child->Initialize(this);
	}
	ComputeWorldTransform(static_cast<CompositeComponent*>(parent));
}

void CompositeComponent::Update(float deltaTime, Compositer* parent)
{
	// TODO: remove casts
	ComputeWorldTransform(static_cast<CompositeComponent*>(parent));
	for (Component* child : children) {
		child->Update(deltaTime, this);
	}
	TEMP_PendingComputeWT();
	ComputeWorldTransform(static_cast<CompositeComponent*>(parent));
}

void CompositeComponent::SetPosition(Math::Vector3 pos)
{
	position = pos;
	recomputeWorldTransform = true;
}

void CompositeComponent::SetRotation(Math::Quaternion rot)
{
	rotation = rot;
	recomputeWorldTransform = true;
}

void CompositeComponent::SetScale(Math::Vector3 scale)
{
	this->scale = scale;
	recomputeWorldTransform = true;
}

Math::Vector3 CompositeComponent::GetForward() const
{
	return Math::Vector3::Transform(Math::Vector3::UnitX, rotation);
}

Math::Vector3 CompositeComponent::GetRight() const
{
	return Math::Vector3::Transform(Math::Vector3::UnitY, rotation);
}

const Math::Matrix& CompositeComponent::GetWorldTransform(Compositer* parent)
{
	// TODO: replace this call with non-recursive version
	ComputeWorldTransform(parent, false);
	return worldTransform;
}

void CompositeComponent::SetWorldTransform(Math::Matrix matr)
{
	//worldTransform = matr;
	if (!matr.Decompose(scale, rotation, position)) {
		assert(false); // wtf?
	}
	recomputeWorldTransform = true;
}

void CompositeComponent::SetLocalTransform(Math::Matrix matr)
{
	matr.Decompose(scale, rotation, position);
}

void CompositeComponent::ComputeWorldTransform(Compositer* parent, bool recursive)
{
	if (recomputeWorldTransform) {
		recomputeWorldTransform = false;
		worldTransform = Math::Matrix::CreateScale(scale);
		worldTransform *= Math::Matrix::CreateFromQuaternion(rotation);
		worldTransform *= Math::Matrix::CreateTranslation(position);
		if (parent) {
			// TODO: where do i get parent for GetWorldTransform() ???
			worldTransform *= parent->GetWorldTransform();
		}
		for (Component* comp : children) {
			comp->TEMP_PendingComputeWT();
		}
	}
	if (!recursive) {
		return;
	}
	for (Component* comp : children) {
		comp->ComputeWorldTransform(this);
	}
}
