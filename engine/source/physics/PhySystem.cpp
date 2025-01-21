#include "PhySystem.h"

#include "Physics.h"
#include "logs/Logs.h"


std::unique_ptr<class PhySystem> gPhySys;

// Callback for traces, connect this to your own trace function if you have one
static void TraceImpl(const char* inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	// Print to the TTY
	LOG_INFO("Trace {}", buffer);
}

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
			LOG_ERROR("PhysicsEntity was not found");
			return;
		}

		PhySystem::PhyEvent event;
		event.type = PhySystem::PhyEvenType::OverlapStart;
		event.body1 = entity1;
		event.body2 = entity2;

		gPhySys->AddEventToQueue(event);

		entity1->beginOverlap(*entity2);
		entity2->beginOverlap(*entity1);
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
		if (!entity1 || !entity2) {
			// log error
			LOG_ERROR("PhysicsEntity was not found");
			return;
		}

		PhySystem::PhyEvent event;
		event.type = PhySystem::PhyEvenType::OverlapEnd;
		event.body1 = entity1;
		event.body2 = entity2;

		gPhySys->AddEventToQueue(event);

		entity1->endOverlap(*entity2);
		entity2->endOverlap(*entity1);
	}
};


bool PhySystem::Init()
{
	RegisterDefaultAllocator();

	JPH::Trace = TraceImpl;
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

	static MyBodyActivationListener body_activation_listener;
	physics_system.SetBodyActivationListener(&body_activation_listener);

	// Setup collision actions
	static MyContactListener contact_listener;
	physics_system.SetContactListener(&contact_listener);

	return true;
}

void PhySystem::Update(float deltaTime)
{
	CallEventsFromQueue();
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

void PhySystem::CallEventsFromQueue()
{
	while (!event_queue.empty())
	{
		PhyEvent event = event_queue.front();
		
		uint32_t body1_ind = event.body1->body->GetID().GetIndex();
		uint32_t body2_ind = event.body2->body->GetID().GetIndex();
		std::string event_type = (event.type == PhyEvenType::OverlapStart) ? "OverlapStart" : "OverlapEnd";
		LOG_INFO("Body1:{} and Body2:{} has PhyEvent:{}", body1_ind, body2_ind, event_type);

		// call PhyEvents
		
		event_queue.pop();
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