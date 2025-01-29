class Moon3ad {
    array<BuildingFoundationComponent@> foundations;
    // vandals
    array<HealthComponent@> enemies;
    // buildings and drones
    array<HealthComponent@> allies;
    // damaged something
    array<HealthComponent@> damagedConstructions;
    // entites that will hide in storageLocation
    array<WorldState@> hidingInStorage;

    Math::Vector3 storageLocation = Math::Vector3(10.0f, 0.0f, 10.0f);
    bool alarmState;

    private int credits = 100;

    bool HasEnoughCredits(int amount) const { return credits >= amount; }
    int GetCredits() const { return credits; }

    int GetCredits() {
        return credits;
    }

    void AddCredits(int value) {
        credits += value;
        credits = Math::Max(credits, 0);
    }

    void RegisterAlly(HealthComponent@ ally) {
        allies.insertLast(ally);
    }

    void RegisterEnemy(HealthComponent@ enemy) {
        enemies.insertLast(enemy);
    }

    void UnregisterAlly(HealthComponent@ ally) {
        const int idx = Moon3ad::gameState.allies.findByRef(ally);
        if (idx >= 0) {
            Moon3ad::gameState.allies.removeAt(idx);
        }
    }

    void UnregisterEnemy(HealthComponent@ enemy) {
        const int idx = Moon3ad::gameState.enemies.findByRef(enemy);
        if (idx >= 0) {
            Moon3ad::gameState.enemies.removeAt(idx);
        }
    }

    void AddDamagedConstruction(HealthComponent@ construction) {
        damagedConstructions.insertLast(construction);
    }

    void RemoveDamagedConstruction(HealthComponent@ construction) {
        const int idx = Moon3ad::gameState.damagedConstructions.findByRef(construction);
        if (idx >= 0) {
            Moon3ad::gameState.damagedConstructions.removeAt(idx);
        }
    }

    void AddToAlarmSensetive(WorldState@ state) {
        hidingInStorage.insertLast(@state);
    }

    BuildingFoundationComponent@ findNearestFreeFactory(Math::Vector3 position) {
        BuildingFoundationComponent@ bestFactory = null;
        float currentDistance = 1000;
        for (uint i = 0; i < foundations.length(); i++) {
            BuildingFoundationComponent@ foundation = @foundations[i];
            float distance = Math::Vector3Distance(foundation.GetParent().GetTransform().GetPosition(), position);
            if (foundation.isBusy 
                && !foundation.isWorking 
                && !foundation.IsMarkedForConstruction() 
                && !foundation.IsUnderConstruction() 
                && distance < currentDistance) {
                    @bestFactory = @foundation;
                    currentDistance = distance;
                }
        }
        return bestFactory;
    }

    BuildingFoundationComponent@ findNearestConstructuinSite(Math::Vector3 position) {
        BuildingFoundationComponent@ bestFactory = null;
        float currentDistance = 1000;
        for (uint i = 0; i < foundations.length(); i++) {
            BuildingFoundationComponent@ foundation = @foundations[i];
            float distance = Math::Vector3Distance(foundation.GetParent().GetTransform().GetPosition(), position);
            if (foundation.IsMarkedForConstruction() && !foundation.IsUnderConstruction() && distance < currentDistance) {
                    @bestFactory = @foundation;
                    currentDistance = distance;
                }
        }
        return bestFactory;
    }

    HealthComponent@ GetConstructionForRepairment(Math::Vector3 position) {
        HealthComponent@ bestConstruction = null;
        float currentDistance = 1000;
        for (uint i = 0; i < damagedConstructions.length(); i++) {
            HealthComponent@ damagedConstruction = @damagedConstructions[i];
            float distance = Math::Vector3Distance(damagedConstruction.GetParent().GetTransform().GetPosition(), position);
            if (damagedConstruction.IsAlive() && distance < currentDistance) {
                    @bestConstruction = @damagedConstruction;
                    currentDistance = distance;
                }
        }
        return bestConstruction;
    }

    HealthComponent@ findNearestAlly(Math::Vector3 position) {
        HealthComponent@ bestAlly = null;
        float currentDistance = 1000;
        for (uint i = 0; i < allies.length(); i++) {
            HealthComponent@ ally = @allies[i];
            float distance = Math::Vector3Distance(ally.GetParent().GetTransform().GetPosition(), position);
            if (ally.IsAlive() && distance < currentDistance) {
                    @bestAlly = @ally;
                    currentDistance = distance;
                }
        }
        return bestAlly;
    }

    HealthComponent@ findNearestEnemy(Math::Vector3 position) {
        HealthComponent@ bestEnemy = null;
        float currentDistance = 1000;
        for (uint i = 0; i < enemies.length(); i++) {
            HealthComponent@ enemy = @enemies[i];
            float distance = Math::Vector3Distance(enemy.GetParent().GetTransform().GetPosition(), position);
            if (enemy.IsAlive() && distance < currentDistance) {
                    @bestEnemy = @enemy;
                    currentDistance = distance;
                }
        }
        return bestEnemy;
    }

    void SwitchAlarm() {
        alarmState = !alarmState;
        for (uint i = 0; i < hidingInStorage.length(); i++) {
            hidingInStorage[i].SetBool("Alarm", alarmState);
        }
    }
}

namespace Moon3ad {
    Moon3ad@ gameState;

    enum BUILDING_TYPE {
        MINE
    }

    const array<int> buildCost = {
        50 // MINE
    };
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
        if (Input::IsKeyDown(Input::Key::F)) {
            Moon3ad::gameState.SwitchAlarm();
        }
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

        if (state.HasEnoughCredits(Moon3ad::buildCost[Moon3ad::BUILDING_TYPE::MINE])) {
            Game::Query::ComponentGetter<BuildingFoundationComponent> query;
            ref@ obj = query.GetChildComponent(parent.GetId());
            BuildingFoundationComponent@ foundation = cast<BuildingFoundationComponent@>(obj);
            if (foundation !is null) {
                if (foundation.isBusy) {
                    return;
                }
                state.AddCredits(-Moon3ad::buildCost[Moon3ad::BUILDING_TYPE::MINE]);
                foundation.SetMarkedForConstruction(true);
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
