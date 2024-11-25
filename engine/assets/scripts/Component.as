interface Component {
    void Update(float deltaTime);
    void FixedUpdate(float deltaTime);
    void SetParent(Component@ _parent);
    Component@ GetParent();
}