
class CompoundTask {
    private array<Method> methods;

    const array<Method>& GetMethods() {
        return methods;
    }
}

abstract class Method {

    string name;
    array<Task@> subTasks;

    bool checkPrecondition(WorldState state);
}