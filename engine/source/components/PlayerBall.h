#pragma once

#include "CompositeComponent.h"
#include "physics/PhySystem.h"


class PlayerBall : public CompositeComponent {
public:
	PlayerBall(Game* game);

	void Initialize(Compositer* parent = nullptr) override;
	void ProceedInput(InputDevice* inpDevice) override;

	static void StartOverlap(PhySystem::PhysicsEntity& other);
	static void EndOverlap(PhySystem::PhysicsEntity& other);

public:
	// temp list for collision test
	std::vector<CompositeComponent*> sceneObjects;

	class MeshComponent* meshComp = nullptr;
	class Body* body = nullptr;

protected:
	class ThirdPersonCamera* camera = nullptr;
	CompositeComponent* meshSocket = nullptr;
	CompositeComponent* attachSocket = nullptr;
};
