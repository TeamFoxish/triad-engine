
class PlanRunner {
    
    array<Task@> currentPlan;

    ExecutionResult run(WorldState &in state, float deltaTime) {
        if (!currentPlan.isEmpty()) {
            Task@ currentTask = currentPlan[0];
            ExecutionResult result = currentTask.execute(deltaTime);
            switch (result) {
                case FINISHED:
                    currentTask.applyEffect(state);
                    currentPlan.removeAt(0);
                    if (currentPlan.isEmpty()) {
                        return result;
                    } else {
                        return CONTINUES;
                    }
                default:
                    return result;
            }
        }
    }
}