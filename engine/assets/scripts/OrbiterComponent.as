class OrbiterComponent : Component {
    // TODO: add overrides for vectors separatly from transform
    Math::Vector3 center = Math::Vector3(0, 0, 0);
    float radius = 5.0f;
    float angle = 0.0f;
    float moveSpeed = 1.0f;
    float rotSpeed = 1.0f;

    OrbiterComponent(ICompositer@ parent = null) {
        super(@parent);
    }

    void Update(float deltaTime) {
        UpdatePos(deltaTime);
    }

    protected void UpdatePos(float deltaTime) {
        angle += moveSpeed * deltaTime;
        Math::Vector3 pos;
        pos.x = center.x + Math::cos(angle) * radius;
        pos.y = center.y;
        pos.z = center.z + Math::sin(angle) * radius;
        Math::Transform@ parentTrs = GetParent().GetTransform();
        parentTrs.SetLocalPosition(pos);
        const Math::Quaternion rot = parentTrs.GetLocalRotation();
        const Math::Quaternion inc = Math::QuaternionFromAxisAngle(Math::Vector3Up, rotSpeed * deltaTime);
        parentTrs.SetLocalRotation(Math::QuaternionConcatenate(rot, inc));
    }
};
