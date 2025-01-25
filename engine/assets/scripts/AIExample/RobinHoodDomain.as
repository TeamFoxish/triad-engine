
// Operator implementations

namespace RobinHood {

    AI::ExecutionResult rest(WorldState& state, float deltaTime, dictionary@ executionState) {
        log_info("Resting.");
        log_info("Zzz...");
        return AI::ExecutionResult::FINISHED;
    }

}


class RobinHoodDomain : Domain {

    void Init() override {
        DomainBuilder@ builder = DomainBuilder(this);
        
        builder

            // Primitive tasks

            .primitiveTask("Train")
                .precondition(function(state) { return state.GetBool('isRested'); })
                .operator(function(state, deltaTime, executionState) { 
                    log_info("Training");
                    return AI::ExecutionResult::FINISHED;
                })
                .effect(function(state) { state.SetBool('isTrained', true); })
                .effect(function(state) { state.SetBool('isRested', false); })
                .end()

            .primitiveTask("Rest")
                .precondition(function(state) { return !state.GetBool('isRested'); })
                .operator(RobinHood::rest)
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
    }
}