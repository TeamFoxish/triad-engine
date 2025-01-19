class CompositeComponent : Component, ICompositer {
    private array<Component@> children;
    private Math::Transform@ transform;
    private bool isUpdating = false;

    CompositeComponent(ICompositer@ _parent = null) {
        super(_parent);
        @transform = Math::Transform(GetParent() !is null ? GetParent().GetTransform() : null);
        Scene::Tree::AddEntityTransform(GetId(), transform);
    }

    CompositeComponent(CompositeComponent@) delete;

    CompositeComponent@ opAssign(const CompositeComponent@) delete;

    Math::Transform@ GetTransform() const { return transform; }

    void SetTransform(const Math::Transform@ trs) { transform = trs; } // copy transform

    // TODO: extract user Init logic to OnInit
    void Init() {
        if (children !is null) {
            for( uint n = 0; n < children.length(); n++ ) {
                if (children[n] !is null) {
                    children[n].Init();
                }
            }
        }
    }

    // TODO: extract user Update logic to OnUpdate
    void Update(float deltaTime) {
        isUpdating = true;
        if (children !is null) {
            for( uint n = 0; n < children.length(); n++ ) {
                if (children[n] !is null) {
                    children[n].Update(deltaTime);
                }
            }
        }
        isUpdating = false;
        dictionaryValue pendingDeadVal;
        array<Component@>@ pendingDead;
        if (Game::Private::gPendingDeadComponents.get(GetEntityKey(), @pendingDead) && pendingDead !is null) {
            for (uint i = 0; i < pendingDead.length(); ++i) {
                RemoveChild(pendingDead[i]);
            }
            Game::Private::gPendingDeadComponents.delete(GetEntityKey());
        }
    }

    void FixedUpdate(float deltaTime) {
        // no implementation yet
    }

    void Destroy() {
        if (!IsAlive()) {
            return;
        }
        if (children !is null) {
            array<Component@> childrenCopy = children;
            for( int n = childrenCopy.length() - 1; n >= 0; --n ) {
                if (childrenCopy[n] !is null) {
                    childrenCopy[n].Destroy();
                }
            }
            children.resize(0);
        }
        Component::Destroy();
    }

    void AddChild(Component@ child) {
        // TODO: adjust child transform if compositer
        // TODO: remove child from its parent component
        children.insertLast(child);
    }

    void RemoveChild(Component@ child) {
        if (isUpdating) {
            if (!Game::Private::gPendingDeadComponents.exists(GetEntityKey())) {
                array<Component@>@ arr = array<Component@>();
                @Game::Private::gPendingDeadComponents[GetEntityKey()] = @arr;
            }
            {
                dictionaryValue val = Game::Private::gPendingDeadComponents[GetEntityKey()];
                array<Component@>@ arr = cast<array<Component@>>(val);
                arr.insertLast(child);
            }
            return;
        }
        const int idx = children.findByRef(@child);
        if (idx < 0) {
            log_error("failed to remove child " + child.GetName() + " with id " + child.GetId() + ". it wasn't found in parent " + GetName() + " with id " + GetId());
            return;
        }
        children.removeAt(uint(idx));
    }

    protected Scene::Entity CreateEntity() {
        Scene::Entity entity = Component::CreateEntity();
        entity.isComposite = true;
        return entity;
    }
};
