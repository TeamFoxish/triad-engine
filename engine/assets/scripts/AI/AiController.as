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
        log_info("INIT in AI CONTROLLER");
        IDomain@ domainHolder = Domain();
        log_info("CREATING DOMAIN");
        CreateObject("Engine", domainName).retrieve(@domainHolder);
        log_info("INIT DOMEN");
        domainHolder.Init();
        log_info("INIT in AI CONTROLLER DONE");
        domain = cast<Domain>(domainHolder);
    }
}