#include "CompositeComponent.h"

#include "shared/SharedStorage.h"

#include <algorithm>

CompositeComponent::CompositeComponent(Game* game, Compositer* compositer)
	: Component(game, compositer)
{
#ifdef EDITOR
	isComposite = true;
#endif // EDITOR

	if (compositer) {
		transformHandle = SharedStorage::Instance().transforms.Add(compositer->GetTransformHandle());
	} else {
		transformHandle = SharedStorage::Instance().transforms.Add();
	}
}

CompositeComponent::~CompositeComponent()
{
	for (Component* child : children) {
		delete child;
	}
	children.clear();
	SharedStorage::Instance().transforms.Remove(transformHandle);
}

void CompositeComponent::RemoveChild(Component* comp)
{
	// TODO: allow removing child in update by add pending list
	auto iter = std::find(children.begin(), children.end(), comp);
	if (iter != children.end()) {
		children.erase(iter);
	}
}

void CompositeComponent::AddChild(CompositeComponent* comp)
{
	children.push_back(comp);
	SharedStorage::Instance().transforms.Attach(transformHandle, comp->GetTransformHandle());
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
}

void CompositeComponent::Update(float deltaTime, Compositer* parent)
{
	for (Component* child : children) {
		child->Update(deltaTime, this);
	}
}

Math::Vector3 CompositeComponent::GetPosition() const
{
	return SharedStorage::Instance().transforms.AccessRead(transformHandle).GetLocalPosition();
}

void CompositeComponent::SetPosition(Math::Vector3 pos)
{
	SharedStorage::Instance().transforms.AccessWrite(transformHandle).SetLocalPosition(pos);
}

Math::Quaternion CompositeComponent::GetRotation() const
{
	return SharedStorage::Instance().transforms.AccessRead(transformHandle).GetLocalRotation();
}

void CompositeComponent::SetRotation(Math::Quaternion rot)
{
	SharedStorage::Instance().transforms.AccessWrite(transformHandle).SetLocalRotation(rot);
}

Math::Vector3 CompositeComponent::GetScale() const
{
	return SharedStorage::Instance().transforms.AccessRead(transformHandle).GetLocalScale();
}

void CompositeComponent::SetScale(Math::Vector3 scale)
{
	SharedStorage::Instance().transforms.AccessWrite(transformHandle).SetLocalScale(scale);
}

Math::Vector3 CompositeComponent::GetForward() const
{
	const Math::Quaternion rotation = SharedStorage::Instance().transforms.AccessRead(transformHandle).GetRotation();
	//const Math::Quaternion rotation = GetRotation();
	return Math::Vector3::Transform(Math::Vector3::Forward, rotation);
}

Math::Vector3 CompositeComponent::GetRight() const
{
	const Math::Quaternion rotation = GetRotation();
	return Math::Vector3::Transform(Math::Vector3::Right, rotation);
}

const Math::Matrix& CompositeComponent::GetWorldTransform(Compositer* parent)
{
	return SharedStorage::Instance().transforms.AccessRead(transformHandle).GetMatrix();
}

void CompositeComponent::SetWorldTransform(const Math::Matrix& matr)
{
	SharedStorage::Instance().transforms.AccessWrite(transformHandle).SetMatrix(matr);
}

void CompositeComponent::SetLocalTransform(const Math::Matrix& matr)
{
	SharedStorage::Instance().transforms.AccessWrite(transformHandle).SetLocalMatrix(matr);
}
