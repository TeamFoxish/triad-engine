#pragma once

#include "game/Component.h"
#include "math/Math.h"

class MoveComponent : public Component {
public:
	MoveComponent(Game* game, Compositer* compositer);

	void Move(Math::Vector3 dir);

	void Update(float deltaTime, Compositer* parent = nullptr) override;

	float GetMoveSpeed() const { return moveSpeed; }
	void SetMoveSpeed(float _moveSpeed) { moveSpeed = _moveSpeed; }

protected:
	Math::Vector3 direction = Math::Vector3::Zero;
	float moveSpeed = 1.0f;
};
