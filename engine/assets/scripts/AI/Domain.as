interface IDomain {
    void Init();
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

    void Init() override {
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
        log_info("REG SIZE: " + _taskRegistry.getSize());
        @this.taskRegistry = @_taskRegistry;
        log_info("NEW REG SIZE: " + this.taskRegistry.getSize());
    }
};