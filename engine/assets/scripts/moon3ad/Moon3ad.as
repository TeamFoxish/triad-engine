class Moon3ad {
    array<BuildingFoundationComponent@> foundations;
    array<FactoryBuilding@> factories;
}

namespace Moon3ad{
    Moon3ad@ gameState;
}

class Moon3adComponent : Component {
    [Editable]
    private Moon3ad state;
    [Editable]
    private ResourceHandle minePrefab;

    Moon3adComponent(ICompositer@ parent = null) {
        super(@parent);
        @Moon3ad::gameState = @state;
    }

    void Init() {
    }

    void OnDestroy() {
        @Moon3ad::gameState = null;
    }

    void Update(float deltaTime) {
        ProceedInput();
    }

    void ProceedInput() {
        if (!Input::IsKeyDown(Input::Key::LeftButton)) {
            return;
        }
        Component@ comp = Scene::GetComponentUnderCursor();
        if (comp is null) {
            return;
        }
        log_debug("Component under cursor: " + comp.GetName() + " id: " + comp.GetId());
        CompositeComponent@ parent = cast<CompositeComponent@>(comp.GetParent());
        if (parent is null) {
            return;
        }
        log_debug("Component parent " + parent.GetName() + " id: " + parent.GetId());

        {
            Game::Query::ComponentGetter<BuildingFoundationComponent> query;
            ref@ obj = query.GetChildComponent(parent.GetId());
            BuildingFoundationComponent@ foundation = cast<BuildingFoundationComponent@>(obj);
            if (foundation !is null) {
                if (foundation.isBusy) {
                    return;
                }
                foundation.isBusy = true;
                Math::Transform trs;
                trs.SetLocalPosition(parent.GetTransform().GetLocalPosition());
                CompositeComponent@ spawnedMine = Game::SpawnPrefab(minePrefab, @trs);
                Game::Query::ComponentGetter<HealthComponent> queryHealth;
                HealthComponent@ healthComp = cast<HealthComponent@>(queryHealth.GetChildComponent(spawnedMine.GetId()));
                if (healthComp !is null) {
                    healthComp.onDied.Subscribe(HandleOnDied);
                }
            }
        }
    }
};

void HandleOnDied(ref@ caller) {
    HealthComponent@ healthComp = cast<HealthComponent@>(caller);
    if (healthComp is null) {
        return;
    }
    cast<CompositeComponent@>(healthComp.GetParent()).Destroy();
}
