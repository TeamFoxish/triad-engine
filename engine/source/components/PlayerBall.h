#pragma once

#include "CompositeComponent.h"

class PlayerBall : public CompositeComponent {
public:
	PlayerBall(Game* game);

	void Initialize(Compositer* parent = nullptr) override;
	void ProceedInput(InputDevice* inpDevice) override;

public:
	// temp list for collision test
	std::vector<CompositeComponent*> sceneObjects;

protected:
	class ThirdPersonCamera* camera = nullptr;
	CompositeComponent* meshSocket = nullptr;
	CompositeComponent* attachSocket = nullptr;
};
