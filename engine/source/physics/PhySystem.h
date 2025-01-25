#pragma once

#include "misc/Handles.h"
#include "scene/SceneTree.h"

#include <string>
#include <queue>

#include "Jolt/Jolt.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Physics/Body/Body.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"

class PhySystem {
public:
    struct PhysicsEntity {
        JPH::Body* body = nullptr; // better hold by id?
        SceneTree::Handle entity;
    };
    using PhysicsStorage = HandleStorage<PhysicsEntity>;
    using PhysicsHandle = PhysicsStorage::Handle;

public:
    enum class PhyEvenType
    {
        OverlapStart,
        OverlapEnd
    };

    struct PhyEvent
    {
        PhyEvenType type;
        PhysicsHandle body1;
        PhysicsHandle body2;
    };

public:
    bool Init();
    void Update(float deltaTime);
    void ProceedPendingEvents();
    void Term();

    PhysicsHandle GetHandleFromId(PhysicsStorage::Index id) const { return phy_storage.GetHandleFromId(id); }
    
    bool IsValidHandle(PhysicsHandle handle) const { return phy_storage.Get(handle) != nullptr; }
    PhysicsEntity& Get(PhysicsHandle handle) { return phy_storage[handle]; }
    const PhysicsEntity& Get(PhysicsHandle handle) const { return phy_storage[handle]; }
    
    PhysicsHandle GetHandleByBodyID(const JPH::BodyID& id);
   
    // deprecated
    PhysicsHandle Add(PhysicsEntity&& entity);

    PhysicsHandle AddBody(SceneTree::Handle entityHandle, const JPH::BodyCreationSettings& bodySettings, PhySystem::PhysicsEntity** outEntity = nullptr);
    void RemoveBody(PhysicsHandle handle);

    JPH::PhysicsSystem* GetPhySystem() { return &physics_system; }

    void AddEventToQueue(const PhyEvent& event) { pendingEvents.push_back(event); };
    void CallEventsFromQueue();

private:
    void UpdateBodyTransforms();

private:
    PhysicsStorage phy_storage;

private:
    JPH::PhysicsSystem physics_system;
    JPH::TempAllocatorImpl* temp_allocator;
    JPH::JobSystemThreadPool* job_system;

    std::vector<PhyEvent> pendingEvents;

    std::unordered_map<JPH::BodyID, PhysicsHandle> bodyIdToHandle;
};

extern std::unique_ptr<class PhySystem> gPhySys;

bool InitPhysicsSystem();
void TermPhysicsSystem();