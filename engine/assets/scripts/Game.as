namespace Game {
    /*
    SoftRef<ResourceHandle> prefabRef("resd://prefabs/cheese.prefab");
    SoftRef<ResourceHandle> compRef("resd://components/SingleSoundComponent.component");
    SoftRef<ResourceHandle> meshCompRef("resd://components/MeshComponent.component");
    SoftRef<ResourceHandle> sceneRef("resd://scenes/first_scene.scene");
    */

    SoftRef<ResourceHandle> mainAgent("resd://navmeshagent/Base.agent");

    void Init() {
        for (uint i = 0; i < Private::scenes.length(); ++i) {
            Private::scenes[i].Init();
        }

        /*
        Math::Transform@ trs = Math::Transform();
        trs.SetLocalPosition(Math::Vector3(3.0f, 3.0f, 3.0f));
        Game::SpawnPrefab(prefabRef.Load(), trs);
        {
            SingleSoundComponent@ soundComp = cast<SingleSoundComponent@>(Game::SpawnComponent(compRef.Load()));
            soundComp.SetEvent("event:/Bubbles/Bubble3D");
            soundComp.Play();
        }
        
        Game::SpawnComposite(meshCompRef.Load());
        */

        //println("LOAD");
        //TransitToScene(sceneRef.Load());
        //println("FINISH");
    }

    void Update(float deltaTime) {
        const array<Math::Vector3>@ path = Navigation::FindPath(mainAgent.Load(), Math::Vector3(-15.0f, 1.1f, -15.0f), Math::Vector3(15.0f, 1.1f, 15.0f));
        // for (int i = 0; i < path.length(); ++i) {
        //     log_info("path vec3: " + path[i].x + ", " + path[i].y + ", " + path[i].z);
        // }

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
