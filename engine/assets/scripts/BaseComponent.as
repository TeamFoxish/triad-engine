class BaseComponent : Component {
    Component@ parent;
    uint64 id;
    string name;

    BaseComponent() {
        println("Base component factory .");
    }

    void Update(float deltaTime) {
        // do nothing, default behavior
    }

    void FixedUpdate(float deltaTime) {
        // do nothing, default behavior
    }

    void SetParent(Component@ _parent) {
        @parent = @_parent;
    }

    Component@ GetParent() {
        return parent;
    }
};