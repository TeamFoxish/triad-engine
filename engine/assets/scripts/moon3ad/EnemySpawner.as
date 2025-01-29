class EnemySpawner : Component {

    [Editable]
    private ResourceHandle enemyPrefab;

    [Editable]
    private float spawnInterval = 0.5f;
    private float currentSpawnDelay = spawnInterval;
    [Editable]
    private float radeInterval = 100.0f;
    private float currentRadeDelay = radeInterval;
    private int needToSpawnCount = 0;
    private int radeSize = 1;

    [Editable]
    private string eventName;


    EnemySpawner(ICompositer@ parent = null) {
        super(parent);
    }

    void Init() {
        currentSpawnDelay = spawnInterval;
        currentRadeDelay = radeInterval;
    }

    void Update(float deltaTime) override {
        currentSpawnDelay -= deltaTime;
        currentRadeDelay -= deltaTime;

        if (currentRadeDelay < 0) {
            log_info("Rade of size " + radeSize + " has started.");
            currentRadeDelay = radeInterval + currentRadeDelay;
            needToSpawnCount += radeSize;
            radeSize++;
            Sound::System::PlayEvent(eventName);
        }

        if (needToSpawnCount > 0 && currentSpawnDelay < 0) {
            currentSpawnDelay = spawnInterval + currentSpawnDelay;

            Math::Transform transform;
            transform.SetPosition(GetParent().GetTransform().GetPosition());
            Game::SpawnPrefab(enemyPrefab, transform, cast<CompositeComponent>(GetParent()).GetParent());
            needToSpawnCount--;
        }
    }
}