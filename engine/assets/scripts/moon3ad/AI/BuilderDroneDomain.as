namespace AI {
    namespace BuilderDrone {

        // CONDITIONS

        bool IsNearConstructionSite(const WorldState &in state) {
            return state.GetBool("IsNearConstructionSite");
        }

        bool IsNearRepairmentSite(const WorldState &in state) {
            return state.GetBool("IsNearRepairmentSite");
        }

        bool IsAlarm(const WorldState &in state) {
            return state.GetBool("Alarm");
        }

        bool IsNearStorage(const WorldState &in state) {
            return state.GetBool("IsNearStorage");
        }

        // TASKS

        AI::ExecutionResult GoToConstructionSite(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            BuildingFoundationComponent@ factory;
            if (!state.IsPropertyExists("constructionSite")) {
                @factory = @Moon3ad::gameState.findNearestConstructuinSite(controller.parentTransform.GetPosition());
                if (factory is null || factory.GetParent() is null) {
                    return AI::ExecutionResult::FAILED;
                }
                log_debug("Builder " + controller.GetParentName() + " found constructuin site: " + factory.GetParentName());
                factory.SetIsUnderConstruction(true);
                factory.SetMarkedForConstruction(false);
                state.SetRef("constructionSite", @any(@factory));
            } else {
                state.GetRef("constructionSite").retrieve(@factory);
            }
            if (factory is null || factory.GetParent() is null) {
                return AI::ExecutionResult::FAILED;
            } else {
                if (state.GetBool("Alarm")) {
                    factory.SetIsUnderConstruction(false);
                    factory.SetMarkedForConstruction(true);
                    return AI::ExecutionResult::FAILED;
                }
                return AI::Drone::MoveTo(controller, state, factory.GetParent().GetTransform().GetPosition());
            }
        }

        AI::ExecutionResult GoToRepairmentSite(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            HealthComponent@ factory;
            if (!state.IsPropertyExists("repairementSite")) {
                @factory = @Moon3ad::gameState.GetConstructionForRepairment(controller.parentTransform.GetPosition());
                if (factory is null || factory.GetParent() is null) {
                    return AI::ExecutionResult::FAILED;
                }
                log_debug("Builder " + controller.GetParentName() + " found repairment site: " + factory.GetParentName());
                state.SetRef("repairementSite", @any(@factory));
            } else {
                state.GetRef("repairementSite").retrieve(@factory);
            }
            if (factory is null || factory.GetParent() is null) {
                return AI::ExecutionResult::FAILED;
            } else {
                if (state.GetBool("Alarm")) {
                    return AI::ExecutionResult::FAILED;
                }
                return AI::Drone::MoveTo(controller, state, factory.GetParent().GetTransform().GetPosition());
            }
        }

        AI::ExecutionResult GoToStorage(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            return AI::Drone::MoveTo(controller, state, Moon3ad::gameState.storageLocation);
        }

        AI::ExecutionResult WaitForAlarmEnd(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            AI::ExecutionResult result = AI::Drone::Wait(state.GetFloat("AlarmWait"), controller, state, deltaTime, executionState);
            return result;
        }

        AI::ExecutionResult Build(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            BuildingFoundationComponent@ factory;
            state.GetRef("constructionSite").retrieve(@factory);
            if (factory is null || factory.GetParent() is null) {
                return AI::ExecutionResult::FINISHED;
            }
            if (state.GetBool("Alarm")) {
                factory.SetIsUnderConstruction(false);
                factory.SetMarkedForConstruction(true);
                return AI::ExecutionResult::FAILED;
            }
            AI::ExecutionResult result = AI::Drone::Wait(state.GetFloat("WorkTime"), controller, state, deltaTime, executionState);
            if (result == AI::ExecutionResult::FINISHED) {
                Math::Transform trs;
                trs.SetLocalPosition(factory.GetParent().GetTransform().GetLocalPosition());
                CompositeComponent@ spawnedMine = Game::SpawnPrefab(ResourceHandle("res://moon3ad/prefabs/mine.prefab"), @trs);
                Game::Query::ComponentGetter<HealthComponent> queryHealth;
                HealthComponent@ healthComp = cast<HealthComponent@>(queryHealth.GetChildComponent(spawnedMine.GetId()));
                if (healthComp !is null) {
                    factory.MakeBusy(healthComp);
                    healthComp.onDied.Subscribe(HandleOnDied);
                }
                factory.SetIsUnderConstruction(false);
                log_info("BUILDED");
            }
            return result;
        }

        AI::ExecutionResult Repair(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            HealthComponent@ factory;
            state.GetRef("repairementSite").retrieve(@factory);
            if (factory is null || factory.GetParent() is null) {
                return AI::ExecutionResult::FINISHED;
            }
            if (state.GetBool("Alarm")) {
                return AI::ExecutionResult::FAILED;
            }
            AI::ExecutionResult result = AI::Drone::Wait(state.GetFloat("WorkTime"), controller, state, deltaTime, executionState);
            if (result == AI::ExecutionResult::FINISHED) {
                factory.AddHealth(100);
                Moon3ad::gameState.RemoveDamagedConstruction(factory);
            }
            return result;
        }

        // EFFECTS

        void NowNearConstructionSite(WorldState &inout state) {
            state.SetBool("IsNearConstructionSite", true);
            state.SetBool("IsNearRepairmentSite", false);
            state.SetBool("IsNearStorage", false);
        }

        void ConstructionDone(WorldState &inout state) {
            state.Remove("constructionSite");
        }

        void RepairmentDone(WorldState &inout state) {
            state.Remove("repairementSite");
        }

        void NowNearRepairmentSite(WorldState &inout state) {
            state.SetBool("IsNearRepairmentSite", true);
            state.SetBool("IsNearConstructionSite", false);
            state.SetBool("IsNearStorage", false);
        }

        void NowNearStorage(WorldState &inout state) {
            state.SetBool("IsNearStorage", true);
            state.SetBool("IsNearConstructionSite", false);
            state.SetBool("IsNearRepairmentSite", false);
        }
    }
}

class BuilderDroneDomain : Domain {

    void Init(WorldState &inout state, ICompositer@ parent, ColliderComponent@ visionPerception) override {

        // INITIAL STATE

        state.SetBool("IsNearConstructionSite", false);
        state.SetFloat("WorkTime", 1.0f);
        state.SetFloat("AlarmWait", 1.0f);
        state.SetBool("IsNearRepairmentSite", false);
        state.SetBool("Alarm", Moon3ad::gameState.alarmState);
        state.SetBool("IsNearStorage", false);

        // DOMAIN

        DomainBuilder@ builder = DomainBuilder(this);

        builder

        // PRIMITIVE TASK

            .primitiveTask("GoBuild")
                .operator(AI::BuilderDrone::GoToConstructionSite)
                .effect(AI::BuilderDrone::NowNearConstructionSite)
                .end()
            .primitiveTask("Build")
                .precondition(AI::BuilderDrone::IsNearConstructionSite)
                .operator(AI::BuilderDrone::Build)
                .effect(AI::BuilderDrone::ConstructionDone)
                .end()
            .primitiveTask("GoRepair")
                .operator(AI::BuilderDrone::GoToRepairmentSite)
                .effect(AI::BuilderDrone::NowNearRepairmentSite)
                .end()
            .primitiveTask("Repair")
                .precondition(AI::BuilderDrone::IsNearRepairmentSite)
                .operator(AI::BuilderDrone::Repair)
                .effect(AI::BuilderDrone::RepairmentDone)
                .end()
            .primitiveTask("GoStorage")
                .precondition(AI::BuilderDrone::IsAlarm)
                .operator(AI::BuilderDrone::GoToStorage)
                .effect(AI::BuilderDrone::NowNearStorage)
                .end()
            .primitiveTask("WaitAlarm")
                .precondition(AI::BuilderDrone::IsNearStorage)
                .precondition(AI::BuilderDrone::IsAlarm)
                .operator(AI::BuilderDrone::WaitForAlarmEnd)
                .end()


        // COMPOUND TASKS

            .compoundTask("BuildSomethig")
                .method()
                    .subtask("GoBuild")
                    .subtask("Build")
                    .end()
                .end()
            .compoundTask("RepairSomethig")
                .method()
                    .subtask("GoRepair")
                    .subtask("Repair")
                    .end()
                .end()
            .compoundTask("Alarm")
                .method()
                    .precondition(AI::BuilderDrone::IsNearStorage)
                    .subtask("WaitAlarm")
                    .end()
                .method()
                    .subtask("GoStorage")
                    .end()
                .end()

        // HIERARCHY
            
            .hierarchy("BuilderDrone")
                .method()
                    .precondition(AI::BuilderDrone::IsAlarm)
                    .subtask("Alarm")
                    .end()
                .method()
                    .subtask("BuildSomethig")
                    .subtask("RepairSomethig")
                    .end()
                .end()
            .end();

        Moon3ad::gameState.AddToAlarmSensetive(state);
    }
};