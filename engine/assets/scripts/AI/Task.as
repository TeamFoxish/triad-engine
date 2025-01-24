
abstract class Task {
    private string name;

    abstract bool checkPrecondition(WorldState state);
    abstract ExecutionResult execute(float deltaTime);
    abstract void applyEffect(WorldState &in state);
}

enum ExecutionResult {
    FINISHED,
    CONTINUES,
    FAILED
}