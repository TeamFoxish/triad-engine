#pragma once

#include "misc/Handles.h"
#include "shared/TransformStorage.h" // TEMP

#include <string>
#include <queue>


#include "Jolt/Jolt.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Physics/Body/Body.h"
#include "Jolt/Core/JobSystemThreadPool.h"


class PhySystem {
public:
    struct PhysicsEntity {
        JPH::Body* body = nullptr;
        TransformStorage::Handle transform;

        void (*beginOverlap)(PhysicsEntity&) = nullptr;
        void (*endOverlap)(PhysicsEntity&) = nullptr;
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

        PhysicsEntity* body1;
        PhysicsEntity* body2;
    };

public:
    bool Init();
    void Update(float deltaTime);
    void Term();

    PhysicsHandle GetHandleFromId(PhysicsStorage::Index id) const { return phy_storage.GetHandleFromId(id); }
    
    PhysicsEntity& Get(PhysicsHandle handle) { return phy_storage[handle]; }
    const PhysicsEntity& Get(PhysicsHandle handle) const { return phy_storage[handle]; }
    
    PhysicsEntity* GetEntityByBodyID(const JPH::BodyID& id)
    {
        for (auto& entity : phy_storage)
        {
            if (entity.body->GetID() == id)
            {
                return &entity;
            }
        }
        return nullptr;
    }
   
    PhysicsHandle Add(PhysicsEntity&& entity);
    JPH::PhysicsSystem* GetPhySystem() { return &physics_system; }

    void AddEventToQueue(PhyEvent event) { event_queue.push(event); };
    void CallEventsFromQueue();

private:
    PhysicsStorage phy_storage;

private:
    JPH::PhysicsSystem physics_system;
    JPH::TempAllocatorImpl* temp_allocator;
    JPH::JobSystemThreadPool* job_system;

    

    // add: PhyEvent_array;
    std::queue<PhyEvent> event_queue;
};

extern std::unique_ptr<class PhySystem> gPhySys;

bool InitPhysicsSystem();
void TermPhysicsSystem();