class PaddleComponent : Component {
    float boundY = 10.0f;
    float moveSpeed = 1.0f;

    PaddleComponent(ICompositer@ parent = null) {
        super(@parent);
    }

    void Update(float deltaTime) {
        UpdatePos(deltaTime);
    }

    protected void UpdatePos(float deltaTime) {
        float dir = 0.0f;
        if (Input::IsKeyHold(Input::Key::W)) {
            dir += 1.0f;
        }
        if (Input::IsKeyHold(Input::Key::S)) {
            dir -= 1.0f;
        }
        if (Math::closeTo(dir, 0.0f)) {
            return;
        }
        Math::Transform@ parentTrs = GetParent().GetTransform();
        Math::Vector3 pos = parentTrs.GetPosition();
        pos.y += dir * moveSpeed * deltaTime;
        pos.y = Math::Clamp(pos.y, -boundY, boundY);
        parentTrs.SetPosition(pos);
    }
};
