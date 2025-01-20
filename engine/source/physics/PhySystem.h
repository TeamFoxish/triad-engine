#pragma once

#include "PhyEvent.h"
#include "misc/Handles.h"
#include "shared/TransformStorage.h" // TEMP

#include <string>

#include "Jolt/Jolt.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Physics/Body/Body.h"
#include "jolt/Core/JobSystemThreadPool.h"

using namespace JPH;

class PhySystem {
public:
    struct PhysicsEntity {
        Body* body = nullptr;
        TransformStorage::Handle transform;

        void (*beginOverlap)(PhysicsEntity& other) = nullptr;
        void (*endOverlap)(PhysicsEntity& other) = nullptr;
    };
    using PhysicsStorage = HandleStorage<PhysicsEntity>;
    using PhysicsHandle = PhysicsStorage::Handle;

    bool Init();
    void Update(float deltaTime);
    void Term();

    PhysicsHandle GetHandleFromId(PhysicsStorage::Index id) const { return phy_storage.GetHandleFromId(id); }
    
    PhysicsEntity& Get(PhysicsHandle handle) { return phy_storage[handle]; }
    const PhysicsEntity& Get(PhysicsHandle handle) const { return phy_storage[handle]; }
    
    PhysicsEntity& GetEntityByBodyID(const BodyID& id)
    {
        PhysicsEntity e;

        for (auto& entity : phy_storage)
        {
            if (entity.body->GetID() == id)
            {
                e = entity;
            }
        }

        return e;
    }
   
    PhysicsHandle Add(PhysicsEntity&& entity);
    JPH::PhysicsSystem* GetPhySystem() { return &physics_system; }

private:
    PhysicsStorage phy_storage;

private:
    JPH::PhysicsSystem physics_system;
    TempAllocatorImpl* temp_allocator;
    JobSystemThreadPool* job_system;
};

extern std::unique_ptr<class PhySystem> gPhySys;

bool InitPhysicsSystem();
void TermPhysicsSystem();