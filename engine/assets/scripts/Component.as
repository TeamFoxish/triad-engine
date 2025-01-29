abstract class Component {
    private Scene::EntityId parentId = Scene::EntityInvalidId;
    private string name;
    private string entityKey;
    private Scene::EntityId id;
    private bool isDead = false;

    Component(ICompositer@ _parent = null) explicit {
        if (_parent !is null) {
            parentId = _parent.GetId();
            _parent.AddChild(this);
        }
        id = Scene::Tree::AddEntity(CreateEntity());
        entityKey = formatUInt(id);
    }

    Component(Component@) delete;

    Component@ opAssign(const Component@) delete;
    Component@ opAssign(Component@) delete;

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
        if (Scene::IsValidEntity(parentId)) {
            GetParent().RemoveChild(this);
            parentId = Scene::EntityInvalidId;
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

    ICompositer@ GetParent() const {
        if (!HasValidParent()) {
            return null;
        }
        return cast<ICompositer@>(Scene::Tree::GetComponentById(parentId));
    }

    const string GetParentName() const {
        if (!HasValidParent()) {
            return "Null";
        }
        return Scene::Tree::GetComponentById(parentId).GetName();
    }

    bool HasValidParent() const {
        return parentId != Scene::EntityInvalidId && Scene::IsValidEntity(parentId);
    }

    Scene::EntityId GetId() const { return id; }
    
    protected Scene::Entity CreateEntity() {
        Scene::Entity entity;
        @entity.entity = @this;
        if (Scene::IsValidEntity(parentId)) {
            entity.parent = parentId;
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