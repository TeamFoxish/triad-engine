
class PrimitiveTask : Task {
    private string name;
    private array<CHECK_PRECONDITION@> preconditions;
    private EXECUTE_SUBTASK@ operator;
    private array<APPLY_EFFECT@> effects;

    PrimitiveTask(
            const string &in _name,
            array<CHECK_PRECONDITION@> _preconditions,
            EXECUTE_SUBTASK@ _operator,
            array<APPLY_EFFECT@> _effects
        ) {
        this.name = _name;
        this.preconditions = _preconditions;
        @this.operator = @_operator;
        this.effects = _effects;
    }

    bool checkPrecondition(const WorldState &in state) const {
        for (uint i = 0; i < preconditions.length(); i++) {
            if (!preconditions[i](state)) {
                return false;
            }
        }
        return true;
    }

    AI::ExecutionResult execute(AiController@ controller, WorldState state, float deltaTime, dictionary@ executionState) const {
        return operator(controller, state, deltaTime, executionState);
    }

    void applyEffect(WorldState &in state) const {
        for (uint i = 0; i < effects.length(); i++) {
            effects[i](state);
        }
    }

    const string& GetName() const override { return name; }

    bool isPrimitive() const override { return true; }
};