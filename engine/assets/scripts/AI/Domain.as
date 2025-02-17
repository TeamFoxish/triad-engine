interface IDomain {
    void Init(WorldState& statem, ICompositer@ parent, ColliderComponent@ visionPerception);
    const Task@ GetTask(const string &in name) const;
    const CompoundTask@ GetRoot() const;
    void SetRoot(CompoundTask@ root);
    void SetTaskRegistry(dictionary@ taskRegistry);
    
}

class Domain : IDomain {

    protected CompoundTask@ root;
    protected dictionary@ taskRegistry;

    Domain() {}

    Domain(CompoundTask@ _root, dictionary@ _taskRegistry) {
        this.root = _root;
        this.taskRegistry = _taskRegistry;
    }

    void Init(WorldState& state, ICompositer@ parent, ColliderComponent@ visionPerception) override {
        log_warn("Default empty domain was initialized");
    }

    const Task@ GetTask(const string &in name) const override {
        return cast<Task@>(taskRegistry[name]);
    }

    const CompoundTask@ GetRoot() const override {
        return root;
    }

    void SetRoot(CompoundTask@ root) override {
        @this.root = @root;
    }

    void SetTaskRegistry(dictionary@ _taskRegistry) override {
        @this.taskRegistry = @_taskRegistry;
    }
};