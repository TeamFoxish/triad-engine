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

        // TASKS

        AI::ExecutionResult GoToFactory(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            return AI::Drone::MoveTo(controller, state, state.GetVector("FactoryLocation"));
        }

        AI::ExecutionResult GoToStorage(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            return AI::Drone::MoveTo(controller, state, state.GetVector("StorageLocation"));
        }

        AI::ExecutionResult Mine(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            return AI::Drone::Wait(state.GetFloat("WorkTime"), controller, state, deltaTime, executionState);
        }

        AI::ExecutionResult StoreMinerals(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            AI::ExecutionResult result = AI::Drone::Wait(state.GetFloat("WorkTime"), controller, state, deltaTime, executionState);
            if (result == AI::ExecutionResult::FINISHED) {
                Moon3ad::gameState.IncreaseMoney(state.GetFloat("MineralsAmount") * 5);
            }
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

        state.SetVector("FactoryLocation", Math::Vector3(-15.0f, 0.0f, 15.0f));
        state.SetVector("StorageLocation", Math::Vector3(10.0f, 0.0f, 10.0f));
        state.SetBool("IsNearFactory", false);
        state.SetFloat("WorkTime", 1.0f);
        state.SetFloat("MineralsAmount", 0.0f);

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
                .precondition(AI::MinerDrone::IsCarryingMinerals)
                .operator(AI::MinerDrone::GoToStorage)
                .effect(AI::MinerDrone::NowNearStorage)
                .end()
            .primitiveTask("StoreMinerals")
                .precondition(AI::MinerDrone::IsCarryingMinerals)
                .precondition(AI::MinerDrone::IsNearStorage)
                .operator(AI::MinerDrone::StoreMinerals)
                .effect(AI::MinerDrone::MineralsStored)
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

        // HIERARCHY
            
            .hierarchy("MinerDrone")
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
    }
};