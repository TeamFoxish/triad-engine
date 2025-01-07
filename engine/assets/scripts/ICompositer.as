interface ICompositer {
    Scene::EntityId GetId() const;
    Math::Transform@ GetTransform() const; // TODO: return by weak ref
    void AddChild(Component@ child);
    void RemoveChild(Component@ child);
    // TODO: GetChild<>
};