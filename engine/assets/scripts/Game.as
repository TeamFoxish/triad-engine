namespace Game {

    void Init() {
        for (uint i = 0; i < Private::scenes.length(); ++i) {
            Private::scenes[i].Init();
        }
    }

    void Update(float deltaTime) {
        array<uint> pendingDead;
        for (uint i = 0; i < Private::scenes.length(); ++i) {
            if (!Private::scenes[i].IsAlive()) {
                pendingDead.insertLast(i);
                continue;
            }
            Private::scenes[i].Update(deltaTime);
        }
        for (int i = pendingDead.length() - 1; i >= 0; --i) {
            Private::scenes.removeAt(uint(i));
        }
    }

    void FixedUpdate(float deltaTime) {
        // no implementation yet
    }

    void Shutdown() {
        for (int i = Private::scenes.length() - 1; i >= 0; --i) {
            if (!Private::scenes[i].IsAlive()) {
                continue;
            }
            log_info("begin scene " + Private::scenes[i].GetName() + " destruction");
            Private::scenes[i].Destroy();
        }
        Private::scenes.resize(0);
    }

    void TransitToScene(const ResourceHandle &in sceneRef) {
        Shutdown(); // clear all scenes
        CreateScene(sceneRef);
    }

    CompositeComponent@ CreateScene(const ResourceHandle &in sceneRef) {
        log_info("begin scene creation"); // TODO: place scene tag in log msg
        ref@ scene = Impl::CreateScene(sceneRef);
        if (scene is null) {
            return null;
        }
        CompositeComponent@ sceneInst = cast<CompositeComponent@>(scene);
        Private::scenes.insertLast(sceneInst);
        sceneInst.Init();
        return sceneInst;
    }

    CompositeComponent@ SpawnPrefab(const ResourceHandle &in prefabRef, const Math::Transform@ transform = null, ICompositer@ parent = null) {
        if (parent is null) {
            if (Private::scenes.isEmpty()) {
                log_error("failed to spawn prefab. no active scene found");
                return null;
            }
            @parent = @Private::scenes[0];
        }
        ref@ prefab = Impl::SpawnPrefab(prefabRef, @parent);
        if (prefab is null) {
            return null;
        }
        CompositeComponent@ comp = cast<CompositeComponent@>(prefab);
        if (transform !is null) {
            comp.SetTransform(transform);
        }
        comp.Init();
        return comp;
    }

    Component@ SpawnComponent(const ResourceHandle &in componentRef, ICompositer@ parent = null) {
        Component@ comp = Private::SpawnComponent(componentRef, @parent);
        if (comp is null) {
            return null;
        }
        comp.Init();
        return comp;
    }

    CompositeComponent@ SpawnComposite(const ResourceHandle &in componentRef, const Math::Transform@ transform = null, ICompositer@ parent = null) {
        // TODO: set name for spawned component
        Component@ compInst = Private::SpawnComponent(componentRef, @parent);
        CompositeComponent@ composite = cast<CompositeComponent@>(compInst);
        if (composite is null) {
            log_error("failed to spawn CompositeComponent. invalid component type");
            if (compInst !is null) {
                compInst.Destroy();
            }
            return null;
        }
        if (transform !is null) {
            composite.SetTransform(transform);
        }
        composite.Init();
        return composite;
    }

    namespace Private {
        array<CompositeComponent@> scenes;

        dictionary gPendingDeadComponents = {};

        Component@ SpawnComponent(const ResourceHandle &in componentRef, ICompositer@ parent = null) {
            if (@parent is null) {
                if (scenes.isEmpty()) {
                    log_error("failed to spawn component. no active scene found");
                    return null;
                }
                @parent = @scenes[0];
            }
            ref@ compInst = Impl::SpawnComponent(componentRef, @parent);
            return compInst !is null ? cast<Component@>(compInst) : null;
        }

        void DestroyComponent(ref@ compHandle) {
            Component@ comp = cast<Component@>(compHandle);
            if (comp is null) {
                log_error("failed to destroy component with native Game::DestroyComponent call. invlalid component type or null");
                return;
            }
            comp.Destroy();
        }
    }

}
