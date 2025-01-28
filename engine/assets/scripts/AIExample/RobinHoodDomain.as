
// Operator implementations

namespace RobinHood {

    AI::ExecutionResult moveTo(AiController@ controller, WorldState& state, Math::Vector3 location) {
        bool finish = controller.movement.IsFinished();
        if (finish) {
            return AI::ExecutionResult::FINISHED;
        }
        if (controller.movement.IsMoving()) {
            return AI::ExecutionResult::CONTINUES;
        } else {
            controller.movement.MoveTo(location);
            if (controller.movement.IsMoving()) {
                return AI::ExecutionResult::CONTINUES;;
            } else {
                return AI::ExecutionResult::FAILED;
            }
        }
    }

    AI::ExecutionResult wait(AiController@ controller, WorldState& state, float deltaTime, dictionary@ executionState) {
        return AI::ExecutionResult::CONTINUES;
    }

    // AI::ExecutionResult rest(AiController@ controller, WorldState& state, float deltaTime, dictionary@ executionState) {
    //     AI::ExecutionResult movRes = RobinHood::moveTo(controller, state, state.GetVector("Bed"));
    //     if (movRes == AI::ExecutionResult::FINISHED) {
    //         log_info("Resting.");
    //         log_info("Zzz...");
    //     }
    //     return movRes;
    // }

}


class RobinHoodDomain : Domain {

    void Init(WorldState& state, ICompositer@ parent, ColliderComponent@ visionPerception) override {

        state.SetBool("isTrained", false);
        state.SetBool("isRested", true);
        state.SetVector("TrainCamp", Math::Vector3(15.0f, 1.1f, 15.0f));
        state.SetVector("Bed", Math::Vector3(-12.0f, 1.1f, -15.0f));

        DomainBuilder@ builder = DomainBuilder(this);
        
        builder

            // Primitive tasks

            .primitiveTask("Train")
                .precondition(function(state) { return state.GetBool('isRested'); })
                .operator(function(controller, state, deltaTime, executionState) {
                    AI::ExecutionResult movRes = RobinHood::moveTo(controller, state, state.GetVector("TrainCamp"));
                    if (movRes == AI::ExecutionResult::FINISHED) {
                        log_info("Training");
                    }
                    if (movRes == AI::ExecutionResult::FAILED) {
                        log_warn("Training failed");
                    }
                    return movRes;
                })
                .effect(function(state) { state.SetBool('isTrained', true); })
                .effect(function(state) { state.SetBool('isRested', false); })
                .end()

            .primitiveTask("Rest")
                .precondition(function(state) { return !state.GetBool('isRested'); })
                .operator(function(controller, state, deltaTime, executionState) {
                    AI::ExecutionResult movRes = RobinHood::moveTo(controller, state, state.GetVector("Bed"));
                    if (movRes == AI::ExecutionResult::FINISHED) {
                        log_info("Resting");
                    }
                    if (movRes == AI::ExecutionResult::FAILED) {
                        log_warn("Resting failed");
                    }
                    return movRes;
                })
                .effect(function(state) { state.SetBool('isRested', true); })
                .end()

            // Compound Tasks

            .compoundTask("LiveInCamp")
                .method()
                    .precondition(function(state) { return state.GetBool('isRested'); })
                    .subtask("Train")
                    .end()
                .method()
                    .subtask("Rest")
                    .end()
                .end()
            
            // Hierarchy

            .hierarchy("RobinHood")
                .method()
                    .subtask("LiveInCamp")
                    .end()
                .end()
            .end();

        log_info("BUILDED DOMAIN REGISTRY " + taskRegistry.getSize());






    //     visionPerception.onOverlapBegin = function(ColliderComponent@ first, ColliderComponent@ other) {
    //         if (other.GetName() == "Player") {
    //             state.ReplanningBoolWrite("SeesPlayer", true);
    //         }
    //     };

    //     visionPerception.onOverlapEnd = function(ColliderComponent@ first, ColliderComponent@ other) {
    //         if (other.GetName() == "Player") {
    //             state.SetBool("SeesPlayer", false);
    //         }
    //     };
    }
}