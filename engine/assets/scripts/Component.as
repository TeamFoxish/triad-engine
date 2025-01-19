abstract class Component {
    private ICompositer@ parent; // TODO: hold parent as id
    private string name;
    private string entityKey;
    private Scene::EntityId id;
    private bool isDead = false;

    Component(ICompositer@ _parent = null) {
        @parent = @_parent;
        if (parent !is null) {
            parent.AddChild(this);
        }
        id = Scene::Tree::AddEntity(CreateEntity());
        entityKey = formatUInt(id);
    }

    Component(Component@) delete;

    Component@ opAssign(const Component@) delete;

    ~Component() {
        Destroy();
    }

    void Init() {
        // do nothing, default behavior
    }

    void Destroy() {
        if (isDead) {
            return;
        }
        OnDestroy();
        isDead = true;
        Scene::Tree::RemoveEntity(id);
        if (parent !is null) {
            parent.RemoveChild(this);
            @parent = null;
        }
    }

    void OnDestroy() {}

    void Update(float deltaTime) {
        // do nothing, default behavior
    }

    void FixedUpdate(float deltaTime) {
        // no implementation yet
    }

    bool IsAlive() const { return !isDead; }

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

    string GetName() {
        return name;
    }

    protected const string& GetEntityKey() const { return entityKey; }
};