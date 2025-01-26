class AiController {
    protected WorldState state;
    protected Domain domain;
    string domainName;
    protected PlanRunner runner;
    protected HTNPlanner planner;
    protected ICompositer@ parent;
    AIMovementComponent@ movement;
    BoxColliderComponent@ visionCollider;

    void Update(float deltaTime) {
        if (state.IsDirty()) {
            runner.SetPlan(planner.GeneratePlan(state, domain));
        }
        AI::ExecutionResult result = runner.run(this, state, deltaTime);
        if (result == AI::ExecutionResult::FINISHED || result == AI::ExecutionResult::FAILED) {
            runner.SetPlan(planner.GeneratePlan(state, domain));
        }
    }

    void Init(ICompositer@ _parent) {
        IDomain@ domainHolder = Domain();
        CreateObject("Engine", domainName).retrieve(@domainHolder);
        domainHolder.Init(state, parent, visionCollider);
        domain = cast<Domain>(domainHolder);

        @this.parent = @_parent;
        @this.movement = @gMovementComp;
        counter++;
        log_critical("CC " + counter);
    }
}