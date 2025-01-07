
namespace Game {

    SceneInstance@ CreateScene(const ResourceHandle &in sceneRef) {
        ref@ scene = Impl::CreateScene(sceneRef);
        return scene !is null ? cast<SceneInstance@>(scene) : null;
    }

    CompositeComponent@ SpawnPrefab(const ResourceHandle &in prefabRef, const Math::Transform@ transform = null, ICompositer@ parent = null) {
        // TODO: add prefab to existing scene root if parent is null
        ref@ prefab = Impl::SpawnPrefab(prefabRef, parent);
        if (prefab is null) {
            return null;
        }
        CompositeComponent@ comp = cast<CompositeComponent@>(prefab);
        if (transform !is null) {
            comp.SetTransform(transform);
        }
        return comp;
    }

    Component@ SpawnComponent(const ResourceHandle &in componentRef, ICompositer@ parent = null) {
        // TODO: add component to existing scene root if parent is null
        ref@ compInst = Impl::SpawnComponent(componentRef, parent);
        return compInst !is null ? cast<Component@>(compInst) : null;
    }

    CompositeComponent@ SpawnComposite(const ResourceHandle &in componentRef, const Math::Transform@ transform = null, ICompositer@ parent = null) {
        // TODO: set name for spawned component
        Component@ comp = SpawnComponent(componentRef, parent);
        CompositeComponent@ composite = cast<CompositeComponent@>(comp);
        if (composite is null) {
            log_error("failed to spawn CompositeComponent. invalid component type");
            comp.Destroy();
            return null;
        }
        if (transform !is null) {
            composite.SetTransform(transform);
        }
        return composite;
    }

    namespace Private {
        dictionary gPendingDeadComponents = {};
        // TODO: add gPendingAddedComponents;

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
