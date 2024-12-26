class CompositeComponent : Component, ICompositer {
    private array<Component@> children;
    private Math::Transform@ transform;

    CompositeComponent(ICompositer@ _parent = null) {
        super(_parent);
        @transform = Math::Transform(GetParent() !is null ? GetParent().GetTransform() : null);
        Scene::Tree::AddEntityTransform(GetId(), transform);
    }

    Math::Transform@ GetTransform() const { return transform; }

    const array<Component@>& GetChildren() const { return children; }

    void Init() {
        if (children !is null) {
            for( uint n = 0; n < children.length(); n++ ) {
                if (children[n] !is null) {
                    children[n].Init();
                }
            }
        }
    }

    void Update(float deltaTime) {
        if (children !is null) {
            for( uint n = 0; n < children.length(); n++ ) {
                if (children[n] !is null) {
                    children[n].Update(deltaTime);
                }
            }
        }
    }

    void FixedUpdate(float deltaTime) {
        if (children !is null) {
            for( uint n = 0; n < children.length(); n++ ) {
                if (children[n] !is null) {
                    children[n].FixedUpdate(deltaTime);
                }
            }
        }
    }

    void AddChild(Component@ child) {
        // TODO: adjust child transform if compositer
        // TODO: remove child from its parent component
        children.insertLast(child);
    }

    protected Scene::Entity CreateEntity() {
        Scene::Entity entity = Component::CreateEntity();
        entity.isComposite = true;
        return entity;
    }
};