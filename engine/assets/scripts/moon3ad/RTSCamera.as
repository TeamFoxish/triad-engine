class RTSCameraComponent : Component {
    [Editable]
    private float mouseSensitivity = 1.0f;
    [Editable]
    private float maxPitch = 1.0f;
    [Editable]
    private float moveSpeed = 1.0f;

    private float rotationX = 0.0f;
    private float rotationY = 0.0f;

    RTSCameraComponent(ICompositer@ parent = null) {
        super(@parent);
    }

    void Init() {
    }

    void Update(float deltaTime) {
        if (!Input::IsKeyHold(Input::Key::RightButton)) {
            return;
        }
        Math::Transform@ camTrs = GetParent().GetTransform();
        Math::Quaternion camRot = camTrs.GetLocalRotation();
        float speedMult = 1.0f;
        if (Input::IsKeyHold(Input::Key::LeftShift)) {
            speedMult = 3.0f;
        }
        Math::Vector3 moveDir = Math::Vector3Zero;
        if (Input::IsKeyHold(Input::Key::W)) {
            moveDir += Math::Vector3Forward;
        }
        if (Input::IsKeyHold(Input::Key::S)) {
            moveDir -= Math::Vector3Forward;
        }
        if (Input::IsKeyHold(Input::Key::D)) {
            moveDir += Math::Vector3Right;
        }
        if (Input::IsKeyHold(Input::Key::A)) {
            moveDir -= Math::Vector3Right;
        }
        if (moveDir.LengthSq() > 0.01f) {
            moveDir.Transform(camRot);
            moveDir.y = 0.0f;
            moveDir = Math::Vector3(moveDir.x, moveDir.y, moveDir.z).NormalizedCopy();
            Math::Vector3 pos = camTrs.GetLocalPosition();
            pos += moveDir * moveSpeed * speedMult * deltaTime;
            camTrs.SetLocalPosition(pos);
        }
        float verticalLift = 0.0f;
        if (Input::IsKeyHold(Input::Key::E)) {
            verticalLift += 1.0f;
        }
        if (Input::IsKeyHold(Input::Key::Q)) {
            verticalLift -= 1.0f;
        }
        if (Math::Abs(verticalLift) > 0.01f) {
            Math::Vector3 pos = camTrs.GetLocalPosition();
            pos += Math::Vector3Up * verticalLift * moveSpeed * speedMult * deltaTime;
            camTrs.SetLocalPosition(pos);
        }
        {
            float x, y;
            Input::GetMouseOffset(x, y);
            if (x == 0.0f && y == 0.0f) {
                return;
            }
            const float angleX = -x * deltaTime;
            const float angleY = -y * deltaTime;
            rotationX += angleX * mouseSensitivity;
            rotationY += angleY * mouseSensitivity;
            rotationY = Math::Clamp(rotationY, -maxPitch, maxPitch);
        
            camRot = Math::QuaternionFromYawPitchRoll(rotationX, rotationY, 0.0f);
            camTrs.SetLocalRotation(camRot);
        }
    }
};
