abstract class Component {
    private ICompositer@ parent; // TODO: hold parent as weak ref
    private string name;
    private Scene::EntityId id;
    private bool isDead = false;

    Component(ICompositer@ _parent = null) {
        println("Base component factory .");
        @parent = @_parent;
        if (parent !is null) {
            parent.AddChild(this);
        }
        id = Scene::Tree::AddEntity(CreateEntity());
    }

    ~Component() {
        Destroy();
    }

    void Destroy() {
        if (isDead) {
            return;
        }
        isDead = true;
        Scene::Tree::RemoveEntity(id);
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

    Scene::EntityId GetId() const { return id; }
    
    protected Scene::Entity CreateEntity() {
        Scene::Entity entity;
        @entity.entity = @this;
        if (parent !is null) {
            entity.parent = parent.GetId();
        }
        entity.name = name;
        entity.isComposite = false;
        return entity;
    }
};