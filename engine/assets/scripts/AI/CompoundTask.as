
class CompoundTask : Task {
    private string name;
    private array<Method@> methods;

    CompoundTask(const string &in _name, array<Method@> &in _methods) {
        this.name = _name;
        this.methods = _methods;
    }

    const array<Method@>& GetMethods() const {
        return methods;
    }

    const Method@ FindSatisfyingMethod(const WorldState &in state) const {
        for (uint i = 0; i < methods.length(); i++) {
            if (methods[i].checkPrecondition(state)) {
                return methods[i];
            }
        }
        return null;
    }

    const string& GetName() const override { return name; }
    
    bool isPrimitive() const override { return false; }
}

class Method {

    private array<CHECK_PRECONDITION@> preconditions;
    private array<Task@> subtasks;

    Method() {}

    Method(array<CHECK_PRECONDITION@> _preconditions, array<Task@> _subtasks) {
        this.preconditions = _preconditions;
        this.subtasks = _subtasks;
    }

    bool checkPrecondition(const WorldState &in state) const {
        for (uint i = 0; i < preconditions.length(); i++) {
            if (!preconditions[i](state)) {
                return false;
            }
        }
        return true;
    }

    const array<Task@> GetSubtasks() const {
        return subtasks;
    }
};