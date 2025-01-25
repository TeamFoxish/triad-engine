class AiController {
    protected WorldState state;
    protected Domain domain;
    string domainName;
    protected PlanRunner runner;
    protected HTNPlanner planner;

    void Update(float deltaTime) {
        if (state.IsDirty()) {
            runner.SetPlan(planner.GeneratePlan(state, domain));
        }
        AI::ExecutionResult result = runner.run(state, deltaTime);
        if (result == AI::ExecutionResult::FINISHED || result == AI::ExecutionResult::FAILED) {
            runner.SetPlan(planner.GeneratePlan(state, domain));
        }
    }

    void Init() {
        IDomain@ domainHolder = Domain();
        CreateObject("Engine", domainName).retrieve(@domainHolder);
        domainHolder.Init();
        domain = cast<Domain>(domainHolder);

        state.SetBool("IsRested", false);
        state.SetBool("IsTrained", false);
    }
}