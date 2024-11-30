class CompositeComponent : BaseComponent {
    Transform@ transform = Transform();
    array<Component@> child;

    void Update(float deltaTime) {
        for( uint n = 0; n < child.length(); n++ )
            child[n].Update(deltaTime);
    }

    void FixedUpdate(float deltaTime) {
        for( uint n = 0; n < child.length(); n++ )
            child[n].FixedUpdate(deltaTime);
    }

    void AddChild(Component@ children) {
        child.insertLast(children);
    }
}
