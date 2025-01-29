class AIComponent : Component {

    AIComponent(ICompositer@ parent = null) {
        super(parent);
        @parentTransform = @parent.GetTransform();
    }

    void Init() {
        IDomain@ domainHolder = Domain();
        CreateObject("Engine", domainName).retrieve(@domainHolder);
        domainHolder.Init(state, GetParent(), visionCollider);
        domain = cast<Domain>(domainHolder);
        
    }

    protected WorldState state;
    protected Domain domain;
    [Editable]
    string domainName;
    protected PlanRunner runner;
    protected HTNPlanner planner;
    BoxColliderComponent@ visionCollider;

    void Update(float deltaTime) {
        if (isMoving) {
            UpdatePos(deltaTime);
        }
        if (state.IsDirty()) {
            runner.SetPlan(planner.GeneratePlan(state, domain));
        }
        AI::ExecutionResult result = runner.run(this, state, deltaTime);
        if (result == AI::ExecutionResult::FINISHED || result == AI::ExecutionResult::FAILED) {
            log_critical("STORED: " + state.GetFloat("MineralsAmount"));
            runner.SetPlan(planner.GeneratePlan(state, domain));
            log_critical("STORED: " + state.GetFloat("MineralsAmount"));
        }
    }







    // MOVEMENT


    [Editable]
    private ResourceHandle agentProfile;
    [Editable]
    private float moveSpeed = 2.5f;

    private array<Math::Vector3> currentPath;
    private bool isMoving = false;
    private bool isFinished = false;
    private Math::Vector3 currentMoveDir;
    private Math::Transform@ parentTransform;

    protected void UpdatePos(float deltaTime) {
        if (currentPath.isEmpty()) {
            isMoving = false;
            isFinished = true;
            return;
        }
        const Math::Vector3 target = currentPath[currentPath.length() - 1];
        Math::Vector3 pos = parentTransform.GetPosition();
        Math::Vector3 dir = target - pos;
        if (dir.LengthSq() < 0.001 || dir.Dot(currentMoveDir) < 0.0) {
            // next point
            currentPath.removeLast();
            return;
        }
        dir.Normalize();
        currentMoveDir = dir;
        parentTransform.SetPosition(pos + currentMoveDir * moveSpeed * deltaTime);
        const Math::Quaternion rot = Math::QuaternionFromLookRotation(currentMoveDir, Math::Vector3Up);
        parentTransform.SetRotation(rot);
    }

    bool IsMoving() { return isMoving; }

    bool IsFinished() { 
        bool res = isFinished;
        this.isFinished = false;
        return res; 
    }

    void MoveTo(Math::Vector3 target) {
        isMoving = false;
        isFinished = false;
        currentMoveDir = Math::Vector3Zero;
        currentPath = Navigation::FindPath(agentProfile, target, parentTransform.GetPosition());
        if (currentPath.isEmpty()) {
            return;
        }
        currentPath.insertAt(0, target); // TODO: check if not needed
        isMoving = true;
    }

    void StopMovement() {
        isMoving = false;
    }

    void ResumeMovement() {
        if (!currentPath.isEmpty()) {
            return;
        }
        isMoving = true;
    }
}