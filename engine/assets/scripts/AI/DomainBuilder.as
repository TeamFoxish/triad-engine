
class DomainBuilder {

    private CompoundTask@ hierarchyRoot;
    private dictionary@ tasksRegistry = @dictionary();
    private Domain@ domain;

    DomainBuilder() {}

    DomainBuilder(Domain@ domain) {
        log_info("NOT EMPTY BUILDER");
        @this.domain = @domain;
        @this.tasksRegistry = @dictionary();
    }


    HierarchyBuilder hierarchy(const string &in domainName) {
        log_info("Hierarchy " + domainName);
        return HierarchyBuilder(domainName, this);
    }

    CompoundTaskBuilder compoundTask(const string &in name) {
        log_info("Compound " + name);
        return CompoundTaskBuilder(name, this);
    }

    PrimitiveTaskBuilder primitiveTask(const string &in name) {
        log_info("Primitive " + name);
        return PrimitiveTaskBuilder(name, this);
    }

    void end() {
        log_info("FINAL ROOT TASK: " + hierarchyRoot.GetName() + ". REGISTRY SIZE: " + tasksRegistry.getSize());
        domain.SetRoot(hierarchyRoot);
        domain.SetTaskRegistry(tasksRegistry);
    }

    void AddTask(Task@ task) {
        log_info("Adding task " + task.GetName() + " to domain");
        @tasksRegistry[task.GetName()] = @task;
    }

    Task@ GetTaskFromRegistry(const string &in name) {
        return cast<Task@>(tasksRegistry[name]);
    }

    void SetHierarchy(CompoundTask@ compoundTask) {
        log_info("Setting root task." + compoundTask.GetName());
        @this.hierarchyRoot = @compoundTask;
    }
};

interface MethodStorage {
    MethodBuilder method();
    void AddMethod(Method@ method);
    DomainBuilder& end();
};

class CompoundTaskBuilder : MethodStorage {
    protected string name;
    protected DomainBuilder domain;
    protected array<Method@> methods;

    CompoundTaskBuilder() {}

    CompoundTaskBuilder(const string &in _name, DomainBuilder &inout _domain) {
        this.name = _name;
        this.domain = _domain;
    }

    MethodBuilder method() {
        log_info("Creating method");
        return MethodBuilder(domain, this);
    }

    DomainBuilder& end() {
        log_info("Creating method done");
        if (domain !is null) {
            cast<DomainBuilder@>(domain).AddTask(CompoundTask(name, methods));
        }
        return domain;
    }

    void AddMethod(Method@ method) {
        log_info("Adding method");
        methods.insertLast(method);
    }
};

class HierarchyBuilder : MethodStorage {
    protected string name;
    protected DomainBuilder domain;
    protected array<Method@> methods;

    HierarchyBuilder() {}

    HierarchyBuilder(const string &in domainName, DomainBuilder &inout _domain) {
        this.name = domainName;
        this.domain = _domain;
    }

    MethodBuilder method() {
        log_info("Creating method");
        return MethodBuilder(domain, this);
    }

    DomainBuilder& end() {
        log_info("Done creating hierarchy");
        if (domain !is null) {
            CompoundTask@ rootTask = CompoundTask(name, methods);
            domain.AddTask(rootTask);
            domain.SetHierarchy(rootTask);
        }
        return domain;
    }

    void AddMethod(Method@ method) {
        log_info("Adding method");
        methods.insertLast(method);
    }
};

class MethodBuilder {

    private DomainBuilder domain;
    private MethodStorage@ parent;
    private HierarchyBuilder hierarchyParent;
    private array<CHECK_PRECONDITION@> preconditions;
    private array<Task@> subtasks;

    MethodBuilder(const DomainBuilder &in domainBuilder, MethodStorage &inout parentBuilder) {
        @this.parent = parentBuilder;
        this.domain = domain;
    }

    MethodBuilder& precondition(CHECK_PRECONDITION@ preconditionFunc) {
        log_info("Adding precondition");
        preconditions.insertLast(preconditionFunc);
        return this;
    }

    MethodBuilder& subtask(const string &in name) {
        log_info("Adding subtask " + name);
        subtasks.insertLast(domain.GetTaskFromRegistry(name));
        return this;
    }

    MethodStorage@ end() {
        log_info("Done creating method");
        if (parent !is null) {
            parent.AddMethod(Method(preconditions, subtasks));
        }
        return parent;
    }
};

class PrimitiveTaskBuilder {
    private DomainBuilder domain;
    private string name;
    private array<CHECK_PRECONDITION@> preconditions;
    private EXECUTE_SUBTASK@ operatorImpl;
    private array<APPLY_EFFECT@> effects;

    PrimitiveTaskBuilder(const string &in _name, DomainBuilder &inout _domain) {
        this.name = _name;
        this.domain = _domain;
    }

    PrimitiveTaskBuilder& precondition(CHECK_PRECONDITION@ precondition) {
        log_info("Adding precondition");
        preconditions.insertLast(precondition);
        return this;
    }

    PrimitiveTaskBuilder& operator(EXECUTE_SUBTASK@ operator) {
        log_info("Adding operator");
        @operatorImpl = @operator;
        return this;
    }

    PrimitiveTaskBuilder& effect(APPLY_EFFECT@ effect) {
        log_info("Adding effect");
        effects.insertLast(effect);
        return this;
    }

    DomainBuilder& end() {
        log_info("Done creating primitive");
        domain.AddTask(PrimitiveTask(name, preconditions, operatorImpl, effects));
        return domain;
    }
};