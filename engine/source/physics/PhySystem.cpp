#include "PhySystem.h"

#include "Physics.h"
#include "shared/SharedStorage.h"
#include "logs/Logs.h"

#include "scripts/ScriptSystem.h" // TEMP
#include <scripthandle.h> // TEMP


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
		auto entity1 = gPhySys->GetHandleByBodyID(inBody1.GetID());
		auto entity2 = gPhySys->GetHandleByBodyID(inBody2.GetID());
		if (entity1 == PhySystem::PhysicsHandle{} || !gPhySys->IsValidHandle(entity1) ||
			entity2 == PhySystem::PhysicsHandle{} || !gPhySys->IsValidHandle(entity2)) {
			LOG_ERROR("PhysicsEntity was not found");
			return;
		}

		PhySystem::PhyEvent event;
		event.type = PhySystem::PhyEvenType::OverlapStart;
		event.body1 = entity1;
		event.body2 = entity2;

		gPhySys->AddEventToQueue(event);
	}

	virtual void OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
	{
	}

	virtual void OnContactRemoved(const SubShapeIDPair& inSubShapePair) override
	{
		auto entity1 = gPhySys->GetHandleByBodyID(inSubShapePair.GetBody1ID());
		auto entity2 = gPhySys->GetHandleByBodyID(inSubShapePair.GetBody2ID());
		if (entity1 == PhySystem::PhysicsHandle{} || !gPhySys->IsValidHandle(entity1) ||
			entity2 == PhySystem::PhysicsHandle{} || !gPhySys->IsValidHandle(entity2)) {
			LOG_ERROR("PhysicsEntity was not found");
			return;
		}

		PhySystem::PhyEvent event;
		event.type = PhySystem::PhyEvenType::OverlapEnd;
		event.body1 = entity1;
		event.body2 = entity2;
		gPhySys->AddEventToQueue(event);
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
	pendingEvents.clear();
	UpdateBodyTransforms();
	physics_system.Update(deltaTime, 1, temp_allocator, job_system);
}

void PhySystem::ProceedPendingEvents()
{
	if (pendingEvents.empty()) {
		return;
	}
	for (const PhySystem::PhyEvent& event : pendingEvents) {
		const PhysicsEntity& physBodyA = phy_storage[event.body1];
		const SceneTree::Entity& entityA = gSceneTree->Get(physBodyA.entity);

		const PhysicsEntity& physBodyB = phy_storage[event.body2];
		const SceneTree::Entity& entityB = gSceneTree->Get(physBodyB.entity);

		if (event.type == PhyEvenType::OverlapStart) {
			gScriptSys->CallFunction("Engine", "void Physics::Impl::CallOnOverlapBegin(ref@, ref@)", [&entityA, &entityB](asIScriptContext* context) {
				CScriptHandle compHandleA;
				compHandleA.Set(entityA.obj.GetRaw(), entityA.obj.GetTypeInfo());
				context->SetArgObject(0, &compHandleA);
				CScriptHandle compHandleB;
				compHandleB.Set(entityB.obj.GetRaw(), entityB.obj.GetTypeInfo());
				context->SetArgObject(1, &compHandleB);
			});
		} else if (event.type == PhyEvenType::OverlapEnd) {
			gScriptSys->CallFunction("Engine", "void Physics::Impl::CallOnOverlapEnd(ref@, ref@)", [&entityA, &entityB](asIScriptContext* context) {
				CScriptHandle compHandleA;
				compHandleA.Set(entityA.obj.GetRaw(), entityA.obj.GetTypeInfo());
				context->SetArgObject(0, &compHandleA);
				CScriptHandle compHandleB;
				compHandleB.Set(entityB.obj.GetRaw(), entityB.obj.GetTypeInfo());
				context->SetArgObject(1, &compHandleB);
			});
		}
		
	}
	pendingEvents.clear();
}

void PhySystem::Term()
{
	BodyInterface& body_interface = physics_system.GetBodyInterface();

	for (auto& entity : phy_storage)
	{
		body_interface.RemoveBody(entity.body->GetID());
		body_interface.DestroyBody(entity.body->GetID());
	}
	bodyIdToHandle.clear();

	UnregisterTypes();
	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;
}

auto PhySystem::GetHandleByBodyID(const JPH::BodyID& id) -> PhysicsHandle
{
	const auto iter = bodyIdToHandle.find(id);
	return iter != bodyIdToHandle.end() ? iter->second : PhysicsHandle{};
}

auto PhySystem::Add(PhysicsEntity&& entity) -> PhysicsHandle
{
	PhysicsHandle handle = phy_storage.Add(std::move(entity));
	const PhysicsEntity& physBody = phy_storage[handle];
	bodyIdToHandle[physBody.body->GetID()] = handle;

	physics_system.OptimizeBroadPhase();  // TODO: Call when create new body. Not every frame!

	return handle;
}

auto PhySystem::AddBody(SceneTree::Handle entityHandle, const JPH::BodyCreationSettings& bodySettings, PhySystem::PhysicsEntity** outEntity) -> PhysicsHandle
{
	BodyInterface& body_interface = physics_system.GetBodyInterface();
	JPH::Body* body = body_interface.CreateBody(bodySettings);
	if (!body) {
		// log error
		return PhysicsHandle{};
	}
	body_interface.AddBody(body->GetID(), EActivation::Activate);

	const PhysicsHandle handle = phy_storage.Add(PhySystem::PhysicsEntity{});
	PhySystem::PhysicsEntity& entity = phy_storage[handle];
	entity.entity = entityHandle;
	entity.body = body;
	if (outEntity) {
		*outEntity = &entity;
	}

	bodyIdToHandle[body->GetID()] = handle;

	physics_system.OptimizeBroadPhase();  // TODO: Call when create new body. Not every frame!

	return handle;
}

void PhySystem::RemoveBody(PhysicsHandle handle)
{
	const PhysicsEntity& physBody = phy_storage[handle];
	const JPH::BodyID& bodyId = physBody.body->GetID();
	bodyIdToHandle.erase(bodyId);
	physics_system.GetBodyInterface().RemoveBody(bodyId);
	phy_storage.Remove(handle);
	// TODO: call overlap end events?
}

void PhySystem::CallEventsFromQueue()
{
	while (!pendingEvents.empty())
	{
		const PhyEvent event = pendingEvents.back();
		pendingEvents.pop_back();
		
		/*phy_storage.Get();
		uint32_t body1_ind = event.body1->body->GetID().GetIndex();
		uint32_t body2_ind = event.body2->body->GetID().GetIndex();
		std::string event_type = (event.type == PhyEvenType::OverlapStart) ? "OverlapStart" : "OverlapEnd";
		LOG_INFO("Body1:{} and Body2:{} has PhyEvent:{}", body1_ind, body2_ind, event_type);*/
	}
}

void PhySystem::UpdateBodyTransforms()
{
	BodyInterface& body_interface = physics_system.GetBodyInterface();
	for (const PhysicsEntity& physBody : phy_storage) {
		if (physBody.body->IsStatic() || physBody.entity.id_ < 0 || !gSceneTree->IsValidHandle(physBody.entity)) {
			continue;
		}
		const SceneTree::Entity& entity = gSceneTree->Get(physBody.entity);
		const Math::Transform& trs = SharedStorage::Instance().transforms.AccessRead(entity.transform);
		const Math::Vector3 pos = trs.GetPosition();
		const Math::Quaternion rot = trs.GetRotation();
		// TODO: update transform only when changed??
		body_interface.SetPositionAndRotation(physBody.body->GetID(), JPH::Vec3(pos.x, pos.y, pos.z), JPH::Quat(rot.x, rot.y, rot.z, rot.w), JPH::EActivation::Activate);
	}
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