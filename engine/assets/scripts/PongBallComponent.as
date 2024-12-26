class PongBallComponent : Component {
    float boundY = 10.0f;
    float boundX = 10.0f;
    float moveSpeed = 1.0f;
    float startMoveSpeed = 0.0f;
    Math::Vector3 moveDir = Math::Vector3(3.0, 2.0, 0.0);

    MeshComponent@ ballMesh;
    CompositeComponent@ paddle;

    Material red;
    Material green;
    Material blue;
    int curMatIdx = -1;

    PongBallComponent(ICompositer@ parent = null) {
        super(@parent);
    }

    void Init() {
        if (paddle !is null) {
            log_debug("paddle was set");
        } else {
            log_error("paddle wasn't set");
        }
        moveDir.Normalize();
        startMoveSpeed = moveSpeed;
    }

    void Update(float deltaTime) {
        UpdatePos(deltaTime);
    }

    protected void UpdatePos(float deltaTime) {
        // move ball
        Math::Transform@ parentTrs = GetParent().GetTransform();
        Math::Vector3 pos = parentTrs.GetPosition();
        const Math::Vector3 ballHalfScale = parentTrs.GetScale() * 0.5f;

        // resolve collision
        if (paddle is null) {
            return;
        }
        const Math::Vector3 paddlePos = paddle.GetParent().GetTransform().GetPosition();
        const Math::Vector3 paddleScale = paddle.GetTransform().GetScale(); // not dividing by 2, cause it seems cube mesh is 2m by each axis ?
        if (pos.x < paddlePos.x - paddleScale.x) {
            println("GAME OVER... restart");
            parentTrs.SetPosition(Math::Vector3Zero);
            moveSpeed = startMoveSpeed;
            return;
        }
        if (pos.x > paddlePos.x) {
            if (moveDir.x < 0.0f && pos.x - ballHalfScale.x < paddlePos.x + paddleScale.x) {
                if (pos.y - ballHalfScale.y > paddlePos.y + paddleScale.y &&
                    pos.y + ballHalfScale.y > paddlePos.y - paddleScale.y) {
                    // miss
                }
                else if (pos.y - ballHalfScale.y < paddlePos.y + paddleScale.y &&
                    pos.y + ballHalfScale.y < paddlePos.y - paddleScale.y) {
                    // miss
                }
                else {
                    HandlePaddleCollision();
                }
            } else if (pos.x > boundX) {
                moveDir.x *= -1.0f;
            }
        }
        if (pos.y + ballHalfScale.y > boundY||
            pos.y - ballHalfScale.y < -boundY) {
            moveDir.y *= -1.0f;
        }
        pos += moveDir * moveSpeed * deltaTime;
        parentTrs.SetPosition(pos);
    }

    protected void HandlePaddleCollision()
    {
        moveDir.x *= -1.0f;
        moveSpeed += startMoveSpeed * 0.1f;
        curMatIdx = (curMatIdx + 1) % 3;
        Material mat;
        switch (curMatIdx) {
        case 0:
            mat = red;
            break;
        case 1:
            mat = green;
            break;
        case 2:
            mat = blue;
            break;
        default:
            mat = red;
            break;
        }
        ballMesh.SetMaterial(mat);
    }
};
