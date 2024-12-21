interface ICompositer {
    Math::Transform@ GetTransform() const; // TODO: return by weak ref
    void AddChild(Component@ child);
    // TODO: RemoveChild
    // TODO: GetChild<>
};