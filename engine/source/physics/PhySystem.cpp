#include "PhySystem.h"

#include "Physics.h"
#include "logs/Logs.h"

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

		entity1.beginOverlap(entity2);
		entity2.beginOverlap(entity1);
	}

	virtual void OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
	{
		LOG_INFO("A contact was persisted");
	}

	virtual void OnContactRemoved(const SubShapeIDPair& inSubShapePair) override
	{
		LOG_INFO("A contact was removed");

		auto entity1 = gPhySys->GetEntityByBodyID(inSubShapePair.GetBody1ID());
		auto entity2 = gPhySys->GetEntityByBodyID(inSubShapePair.GetBody2ID());

		entity1.endOverlap(entity2);
		entity2.endOverlap(entity1);
	}
};

// An example activation listener
class MyBodyActivationListener : public BodyActivationListener
{
public:
	virtual void OnBodyActivated(const BodyID& inBodyID, uint64 inBodyUserData) override
	{
		//cout << "A body got activated" << endl;
	}

	virtual void OnBodyDeactivated(const BodyID& inBodyID, uint64 inBodyUserData) override
	{
		//cout << "A body went to sleep" << endl;
	}
};



bool PhySystem::Init()
{
	RegisterDefaultAllocator();
	Factory::sInstance = new Factory();
	RegisterTypes();

	temp_allocator = new TempAllocatorImpl(10 * 1024 * 1024);
	job_system = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);

	const uint cMaxBodies = 1024; // For a real project: 65536
	const uint cNumBodyMutexes = 0;
	const uint cMaxBodyPairs = 1024; // For a real project: 65536
	const uint cMaxContactConstraints = 1024; // For a real project: 65536

	// Get from Physics.h
	BPLayerInterfaceImpl broad_phase_layer_interface;
	ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
	ObjectLayerPairFilterImpl object_vs_object_layer_filter;

	// Setup physics system
	physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
		broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

	// Setup collision actions
	MyContactListener contact_listener;
	physics_system.SetContactListener(&contact_listener);

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
	delete Factory::sInstance;
	Factory::sInstance = nullptr;
}

auto PhySystem::Add(PhysicsEntity&& entity) -> PhysicsHandle
{
	PhysicsHandle handle = phy_storage.Add(std::move(entity));

	physics_system.OptimizeBroadPhase();  // Call when create new body. Not every frame!

	return handle;
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