
class PlanRunner {

    array<const PrimitiveTask@> currentPlan;
    dictionary executionState;

    AI::ExecutionResult run(AiController@ controller, WorldState &in state, float deltaTime) {
        if (!currentPlan.isEmpty()) {
            const PrimitiveTask@ currentTask = currentPlan[0];
            log_critical("Plan size: " + currentPlan.length());
            AI::ExecutionResult result = currentTask.execute(controller, state, deltaTime, executionState);
            switch (result) {
                case AI::ExecutionResult::FINISHED:
                    executionState.deleteAll();
                    currentTask.applyEffect(state);
                    currentPlan.removeAt(0);
                    if (currentPlan.isEmpty()) {
                        return AI::ExecutionResult::FINISHED;
                    } else {
                        return AI::ExecutionResult::CONTINUES;
                    }
                case AI::ExecutionResult::FAILED:
                    executionState.deleteAll();
                    return AI::ExecutionResult::FAILED;
                default:
                    return result;
            }
        } else {
            return AI::ExecutionResult::FINISHED;
        }
    }

    void SetPlan(array<const PrimitiveTask@> plan) {
        currentPlan = plan;
    }
};