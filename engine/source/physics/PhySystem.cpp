#include "PhySystem.h"

#include "Physics.h"
#include "logs/Logs.h"

#include "Jolt/Core/Factory.h"

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
	//cout << buffer << endl;
}

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
		LOG_INFO("A contact was persisted");
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

// Callback for asserts, connect this to your own assert handler if you have one
static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint inLine)
{
	// Print to the TTY
	//cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << endl;

	// Breakpoint
	return true;
};

bool PhySystem::Init()
{
	RegisterDefaultAllocator();
	//JPH::Trace = TraceImpl;

	JPH::Factory::sInstance = new JPH::Factory();
	
	{
		//if (!VerifyJoltVersionIDInternal(JPH_VERSION_ID))
		//{
		//	Trace("Version mismatch, make sure you compile the client code with the same Jolt version and compiler definitions!");
		//	uint64 mismatch = JPH_VERSION_ID ^ JPH_VERSION_ID;
		//	auto check_bit = [mismatch](int inBit, const char* inLabel) { if (mismatch & (uint64(1) << (inBit + 23))) Trace("Mismatching define %s.", inLabel); };
		//	check_bit(1, "JPH_DOUBLE_PRECISION");
		//	check_bit(2, "JPH_CROSS_PLATFORM_DETERMINISTIC");
		//	check_bit(3, "JPH_FLOATING_POINT_EXCEPTIONS_ENABLED");
		//	check_bit(4, "JPH_PROFILE_ENABLED");
		//	check_bit(5, "JPH_EXTERNAL_PROFILE");
		//	check_bit(6, "JPH_DEBUG_RENDERER");
		//	check_bit(7, "JPH_DISABLE_TEMP_ALLOCATOR");
		//	check_bit(8, "JPH_DISABLE_CUSTOM_ALLOCATOR");
		//	check_bit(9, "JPH_OBJECT_LAYER_BITS");
		//	check_bit(10, "JPH_ENABLE_ASSERTS");
		//	check_bit(11, "JPH_OBJECT_STREAM");
		//	std::abort();
		//}
	}

	//JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)

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
	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;
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