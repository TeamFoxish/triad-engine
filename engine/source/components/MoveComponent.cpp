#include "MoveComponent.h"

#include "core/Compositer.h"

MoveComponent::MoveComponent(Game* game, Compositer* compositer)
	: Component(game, compositer)
{
}

void MoveComponent::Move(Math::Vector3 dir)
{
	direction = dir;
}

void MoveComponent::Update(float deltaTime, Compositer* parent)
{
	if (!parent || direction.LengthSquared() < 0.001f) {
		return;
	}
	const Math::Vector3 pos = parent->GetPosition() + direction * moveSpeed * deltaTime;
	parent->SetPosition(pos);
	direction = Math::Vector3::Zero;
}
