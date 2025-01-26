interface Task {
    const string& GetName() const;
    bool isPrimitive() const;
};

namespace AI {
    enum ExecutionResult {
        FINISHED = 0,
        CONTINUES = 1,
        FAILED = 2
    };
}

funcdef bool CHECK_PRECONDITION(const WorldState &in state);
funcdef AI::ExecutionResult EXECUTE_SUBTASK(AiController@ controller, WorldState& state, float deltaTime, dictionary@ executionState);
funcdef void APPLY_EFFECT(WorldState& state);