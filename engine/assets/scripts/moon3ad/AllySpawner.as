class AllySpawner : CompositeComponent {

    [Editable]
    private ResourceHandle allyType;

    // Should be derived from prefab
    [Editable]
    private Moon3ad::BUILDING_TYPE typeEnum;

    private string type;

    AllySpawner(ICompositer@ parent = null) {
        super(parent);
    }

    void Spawn() {
        if (Moon3ad::gameState.HasEnoughCredits(Moon3ad::buildCost[typeEnum])) {
            log_critical("Spawning ally " + type);
            Moon3ad::gameState.AddCredits(-Moon3ad::buildCost[typeEnum]);
            Math::Transform transform;
            transform.SetPosition(GetParent().GetTransform().GetPosition());
            Game::SpawnPrefab(allyType, transform);
        }  
    }

    void Init() {
        if (type == "guardian") {
            typeEnum = Moon3ad::BUILDING_TYPE::GUARDIAN;
        }
        if (type == "miner") {
            typeEnum = Moon3ad::BUILDING_TYPE::MINER;
        }
        if (type == "builder") {
            typeEnum = Moon3ad::BUILDING_TYPE::BUILDER;
        }
        Moon3ad::gameState.SetAllyFactory(@this, typeEnum);
    }
}