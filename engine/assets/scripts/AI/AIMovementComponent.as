class AIMovementComponent : Component {
    [Editable]
    private ResourceHandle agentProfile;
    [Editable]
    private float moveSpeed = 2.5f;

    private array<Math::Vector3> currPath;
    private bool isMoving = false;
    private bool isFinished = false;
    private Math::Vector3 curMoveDir;

    AIMovementComponent(ICompositer@ parent = null) {
        super(@parent);
        @gMovementComp = @this;
    }

    void Update(float deltaTime) {
        if (isMoving) {
            UpdatePos(deltaTime);
        }
    }

    protected void UpdatePos(float deltaTime) {
        if (currPath.isEmpty()) {
            isMoving = false;
            isFinished = true;
            return;
        }
        const Math::Vector3 target = currPath[currPath.length() - 1];
        Math::Transform@ parentTrs = GetParent().GetTransform();
        Math::Vector3 pos = parentTrs.GetPosition();
        Math::Vector3 dir = target - pos;
        if (dir.LengthSq() < 0.001 || dir.Dot(curMoveDir) < 0.0) {
            // next point
            currPath.removeLast();
            return;
        }
        dir.Normalize();
        curMoveDir = dir;
        parentTrs.SetPosition(pos + curMoveDir * moveSpeed * deltaTime);
        const Math::Quaternion rot = Math::QuaternionFromLookRotation(curMoveDir, Math::Vector3Up);
        parentTrs.SetRotation(rot);
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
        curMoveDir = Math::Vector3Zero;
        const Math::Transform@ parentTrs = GetParent().GetTransform();
        currPath = Navigation::FindPath(agentProfile, target, parentTrs.GetPosition());
        if (currPath.isEmpty()) {
            return;
        }
        currPath.insertAt(0, target); // TODO: check if not needed
        isMoving = true;
    }

    void StopMovement() {
        isMoving = false;
    }

    void ResumeMovement() {
        if (!currPath.isEmpty()) {
            return;
        }
        isMoving = true;
    }
};

AIMovementComponent@ gMovementComp = null;
int counter;