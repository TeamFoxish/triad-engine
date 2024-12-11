class CompositeComponent : BaseComponent {
    Transform@ transform = Transform();
    array<Component@> child;

    void Update(float deltaTime) {
        if (child !is null) {
            for( uint n = 0; n < child.length(); n++ ) {
                if (child[n] !is null) {
                    child[n].Update(deltaTime);
                }
            }
        }
    }

    void FixedUpdate(float deltaTime) {
        if (child !is null) {
            for( uint n = 0; n < child.length(); n++ ) {
                if (child[n] !is null) {
                    child[n].FixedUpdate(deltaTime);
                }
            }
        }
    }

    void AddChild(Component@ children) {
        child.insertLast(children);
        children.SetParent(this);
    }
};