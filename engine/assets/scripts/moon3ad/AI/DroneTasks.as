namespace AI {
    namespace Drone {

        AI::ExecutionResult MoveTo(AIComponent@ controller, WorldState &inout state, Math::Vector3 location, bool force = false) {
            bool finish = controller.IsFinished();
            if (!force && finish) {
                return AI::ExecutionResult::FINISHED;
            }
            if (!force && controller.IsMoving()) {
                return AI::ExecutionResult::CONTINUES;
            } else {
                controller.MoveTo(location);
                if (controller.IsMoving()) {
                    return AI::ExecutionResult::CONTINUES;
                } else {
                    return AI::ExecutionResult::FAILED;
                }
            }
        }

        AI::ExecutionResult Wait(float time, AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            if (!executionState.exists("remainTime")) {
                executionState["remainTime"] = time;
                return AI::ExecutionResult::CONTINUES;
            } else {
                float remainTime = float(executionState["remainTime"]) - deltaTime;
                if (remainTime <= 0) {
                    return AI::ExecutionResult::FINISHED;
                } else {
                    executionState["remainTime"] = remainTime;
                    return AI::ExecutionResult::CONTINUES;
                }
            }
        }
    }
}