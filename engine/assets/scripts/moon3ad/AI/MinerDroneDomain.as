namespace AI {
    namespace MinerDrone {

        // CONDITIONS

        bool IsNotCarryingMinerals(const WorldState &in state) {
            return state.GetFloat("MineralsAmount") == 0;
        }

        bool IsCarryingMinerals(const WorldState &in state) {
            return state.GetFloat("MineralsAmount") != 0;
        }

        bool IsNearFactory(const WorldState &in state) {
            return state.GetBool("IsNearFactory");
        }

        bool IsNearStorage(const WorldState &in state) {
            return state.GetBool("IsNearStorage");
        }

        bool IsAlarm(const WorldState &in state) {
            return state.GetBool("Alarm");
        }

        // TASKS

        AI::ExecutionResult GoToFactory(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            BuildingFoundationComponent@ factory;
            if (!state.IsPropertyExists("factory")) {
                @factory = @Moon3ad::gameState.findNearestFreeFactory(controller.parentTransform.GetPosition());
                if (factory is null) {
                    return AI::ExecutionResult::FAILED;
                }
                log_debug("Miner " + controller.GetParentName() + " found factory: " + factory.GetParentName());
                factory.SetWorking(true);
                state.SetRef("factory", @any(@factory));
            } else {
                state.GetRef("factory").retrieve(@factory);
            }
            if (factory is null) {
                return AI::ExecutionResult::FAILED;
            } else {
                if (state.GetBool("Alarm")) {
                    factory.SetWorking(false);
                    return AI::ExecutionResult::FAILED;
                }
                return AI::Drone::MoveTo(controller, state, factory.GetParent().GetTransform().GetPosition());
            }
        }

        AI::ExecutionResult GoToStorage(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            return AI::Drone::MoveTo(controller, state, Moon3ad::gameState.storageLocation);
        }

        AI::ExecutionResult Mine(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            BuildingFoundationComponent@ factory;
            state.GetRef("factory").retrieve(@factory);
            if (state.GetBool("Alarm")) {
                factory.SetWorking(false);
                return AI::ExecutionResult::FAILED;
            }
            AI::ExecutionResult result = AI::Drone::Wait(state.GetFloat("WorkTime"), controller, state, deltaTime, executionState);
            if (result == AI::ExecutionResult::FINISHED) {
                factory.SetWorking(false);
            }
            return result;
        }

        AI::ExecutionResult StoreMinerals(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            if (state.GetBool("Alarm")) {
                return AI::ExecutionResult::FAILED;
            }
            AI::ExecutionResult result = AI::Drone::Wait(state.GetFloat("WorkTime"), controller, state, deltaTime, executionState);
            if (result == AI::ExecutionResult::FINISHED) {
                Moon3ad::gameState.AddCredits(int(state.GetFloat("MineralsAmount")) * 5);
            }
            return result;
        }

        AI::ExecutionResult WaitForAlarmEnd(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            if (!state.GetBool("Alarm")) {
                return AI::ExecutionResult::FINISHED;
            }
            AI::ExecutionResult result = AI::Drone::Wait(state.GetFloat("AlarmWait"), controller, state, deltaTime, executionState);
            return result;
        }

        // EFFECTS

        void NowNearFactory(WorldState &inout state) {
            state.SetBool("IsNearFactory", true);
            state.SetBool("IsNearStorage", false);
        }

        void NowNearStorage(WorldState &inout state) {
            state.SetBool("IsNearStorage", true);
            state.SetBool("IsNearFactory", false);
        }

        void MiningComplete(WorldState &inout state) {
            state.SetFloat("MineralsAmount", 5.0f);
        }

        void MineralsStored(WorldState &inout state) {
            state.SetFloat("MineralsAmount", 0.0f);
        }

    }
}

class MinerDroneDomain : Domain {

    void Init(WorldState &inout state, ICompositer@ parent, ColliderComponent@ visionPerception) override {

        // INITIAL STATE

        state.SetBool("IsNearFactory", false);
        state.SetBool("IsNearStorage", false);
        state.SetFloat("WorkTime", 1.0f);
        state.SetFloat("MineralsAmount", 0.0f);
        state.SetBool("Alarm", Moon3ad::gameState.alarmState);
        state.SetFloat("AlarmWait", 1.0f);

        // DOMAIN

        DomainBuilder@ builder = DomainBuilder(this);

        builder

        // PRIMITIVE TASK

            .primitiveTask("GoToFactory")
                .precondition(AI::MinerDrone::IsNotCarryingMinerals)
                .operator(AI::MinerDrone::GoToFactory)
                .effect(AI::MinerDrone::NowNearFactory)
                .end()
            .primitiveTask("Mine")
                .precondition(AI::MinerDrone::IsNotCarryingMinerals)
                .precondition(AI::MinerDrone::IsNearFactory)
                .operator(AI::MinerDrone::Mine)
                .effect(AI::MinerDrone::MiningComplete)
                .end()
            .primitiveTask("GoToStorage")
                .operator(AI::MinerDrone::GoToStorage)
                .effect(AI::MinerDrone::NowNearStorage)
                .end()
            .primitiveTask("StoreMinerals")
                .precondition(AI::MinerDrone::IsCarryingMinerals)
                .precondition(AI::MinerDrone::IsNearStorage)
                .operator(AI::MinerDrone::StoreMinerals)
                .effect(AI::MinerDrone::MineralsStored)
                .end()
            .primitiveTask("WaitAlarm")
                .precondition(AI::BuilderDrone::IsNearStorage)
                .precondition(AI::BuilderDrone::IsAlarm)
                .operator(AI::BuilderDrone::WaitForAlarmEnd)
                .end()

        // COMPOUND TASKS

            .compoundTask("Mining")
                .method()
                    .subtask("GoToFactory")
                    .subtask("Mine")
                    .end()
                .end()
            .compoundTask("Dumping")
                .method()
                    .subtask("GoToStorage")
                    .subtask("StoreMinerals")
                    .end()
                .end()
            .compoundTask("Alarm")
                .method()
                    .precondition(AI::MinerDrone::IsNearStorage)
                    .subtask("WaitAlarm")
                    .end()
                .method()
                    .subtask("GoToStorage")
                    .end()
                .end()

        // HIERARCHY
            
            .hierarchy("MinerDrone")
                .method()
                    .precondition(AI::MinerDrone::IsAlarm)
                    .subtask("Alarm")
                    .end()
                .method()
                    .precondition(AI::MinerDrone::IsNotCarryingMinerals)
                    .subtask("Mining")
                    .end()
                .method()
                    .precondition(AI::MinerDrone::IsCarryingMinerals)
                    .subtask("Dumping")
                    .end()
                .end()
            .end();

        Moon3ad::gameState.AddToAlarmSensetive(state);
    }
};