#include "PhySystem.h"

#include "Physics.h"
#include "logs/Logs.h"

#include "Jolt/Core/Factory.h"

static JPH::Color COLLISION_COLLOR(189, 195, 199, 225); // Silver Sand

std::unique_ptr<class PhySystem> gPhySys;

// An example contact listener
// Interface with functions to call onOverlap Begin, Continue and End
class MyContactListener : public ContactListener
{
public:
	// See: ContactListener
	virtual ValidateResult OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult) override
	{
		//cout << "Contact validate callback" << endl;

		// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
		return ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	virtual void OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
	{
		LOG_INFO("A contact was added");

		auto entity1 = gPhySys->GetEntityByBodyID(inBody1.GetID());
		auto entity2 = gPhySys->GetEntityByBodyID(inBody2.GetID());
		if (!entity1 || !entity2) {
			// log error
			return;
		}

		entity1->beginOverlap(*entity2);
		entity2->beginOverlap(*entity1);
	}

	virtual void OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
	{
		//LOG_INFO("A contact was persisted");
	}

	virtual void OnContactRemoved(const SubShapeIDPair& inSubShapePair) override
	{
		LOG_INFO("A contact was removed");

		auto entity1 = gPhySys->GetEntityByBodyID(inSubShapePair.GetBody1ID());
		auto entity2 = gPhySys->GetEntityByBodyID(inSubShapePair.GetBody2ID());
		if (!entity1 || !entity2) {
			// log error
			return;
		}

		entity1->endOverlap(*entity2);
		entity2->endOverlap(*entity1);
	}
};


bool PhySystem::Init()
{
	RegisterDefaultAllocator();

	JPH::Factory::sInstance = new JPH::Factory();

	RegisterTypes();

	temp_allocator = new TempAllocatorImpl(10 * 1024 * 1024);
	job_system = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);

	const uint cMaxBodies = 1024; // For a real project: 65536
	const uint cNumBodyMutexes = 0;
	const uint cMaxBodyPairs = 1024; // For a real project: 65536
	const uint cMaxContactConstraints = 1024; // For a real project: 65536

	// Get from Physics.h
	// TODO: move somewhere more appropriate
	static BPLayerInterfaceImpl broad_phase_layer_interface;
	static ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
	static ObjectLayerPairFilterImpl object_vs_object_layer_filter;

	// Setup physics system
	physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
		broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

	// Setup collision actions
	static MyContactListener contact_listener;
	physics_system.SetContactListener(&contact_listener);

	mdd = std::make_unique<MyDebugDraw>();

	return true;
}

void PhySystem::Update(float deltaTime)
{
	physics_system.Update(deltaTime, 1, temp_allocator, job_system);
}

void PhySystem::Term()
{
	BodyInterface& body_interface = physics_system.GetBodyInterface();

	for (auto& entity : phy_storage)
	{
		body_interface.RemoveBody(entity.body->GetID());
		body_interface.DestroyBody(entity.body->GetID());
	}

	UnregisterTypes();
	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;
}

auto PhySystem::Add(PhysicsEntity&& entity) -> PhysicsHandle
{
	PhysicsHandle handle = phy_storage.Add(std::move(entity));

	physics_system.OptimizeBroadPhase();  // Call when create new body. Not every frame!

	return handle;
}

void PhySystem::DebugDraw()
{
	for (auto& entity : phy_storage)
	{
		auto& body = entity.body;

		switch (body->GetShape()->GetSubType())
		{
		case JPH::EShapeSubType::Box:
		{
			JPH::BoxShape* box = static_cast<JPH::BoxShape*>(const_cast<JPH::Shape*>(body->GetShape()));
			mdd->DrawWireBox(box->GetLocalBounds(), COLLISION_COLLOR);
			break;
		}

		case JPH::EShapeSubType::Sphere:
		{
			JPH::SphereShape* sphere = static_cast<JPH::SphereShape*>(const_cast<JPH::Shape*>(body->GetShape()));
			mdd->DrawWireSphere(body->GetCenterOfMassPosition(), sphere->GetRadius(), COLLISION_COLLOR, 2/*inLevel*/);
			break;
		}

		case JPH::EShapeSubType::Capsule:
		{
			JPH::CapsuleShape* capsule = static_cast<JPH::CapsuleShape*>(const_cast<JPH::Shape*>(body->GetShape()));
			mdd->DrawCapsule(
				body->GetWorldTransform(),
				capsule->GetHalfHeightOfCylinder(),
				capsule->GetRadius(),
				COLLISION_COLLOR,
				JPH::DebugRenderer::ECastShadow::Off,
				JPH::DebugRenderer::EDrawMode::Wireframe
			);
			break;
		}

		case JPH::EShapeSubType::Cylinder:
		{
			JPH::CylinderShape* cylinder = static_cast<JPH::CylinderShape*>(const_cast<JPH::Shape*>(body->GetShape()));
			mdd->DrawCylinder(
				body->GetWorldTransform(),
				cylinder->GetHalfHeight(),
				cylinder->GetRadius(),
				COLLISION_COLLOR,
				JPH::DebugRenderer::ECastShadow::Off,
				JPH::DebugRenderer::EDrawMode::Wireframe
			);
			break;
		}

		case JPH::EShapeSubType::TaperedCylinder: // Could be used as Perception
		{
			JPH::TaperedCylinderShape* taperedCylinder = static_cast<JPH::TaperedCylinderShape*>(const_cast<JPH::Shape*>(body->GetShape()));
			mdd->DrawTaperedCylinder(
				body->GetWorldTransform(),
				body->GetPosition().GetY() + taperedCylinder->GetHalfHeight(),
				body->GetPosition().GetY() - taperedCylinder->GetHalfHeight(),
				taperedCylinder->GetTopRadius(),
				taperedCylinder->GetTopRadius(),
				COLLISION_COLLOR,
				JPH::DebugRenderer::ECastShadow::Off,
				JPH::DebugRenderer::EDrawMode::Wireframe
			);
			break;
		}

		default:
			break;
		}
	}
}

bool InitPhysicsSystem()
{
	gPhySys = std::make_unique<PhySystem>();
	return gPhySys->Init();
}

void TermPhysicsSystem()
{
	gPhySys->Term();
	gPhySys.reset();
}