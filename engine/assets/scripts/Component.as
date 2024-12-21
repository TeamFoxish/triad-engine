abstract class Component {
    private ICompositer@ parent; // TODO: hold parent as weak ref
    private string name;

    Component(ICompositer@ _parent = null) {
        println("Base component factory .");
        @parent = @_parent;
        if (parent !is null) {
            parent.AddChild(this);
        }
    }

    void Update(float deltaTime) {
        // do nothing, default behavior
    }

    void FixedUpdate(float deltaTime) {
        // do nothing, default behavior
    }

    ICompositer@ GetParent() {
        return parent;
    }
};