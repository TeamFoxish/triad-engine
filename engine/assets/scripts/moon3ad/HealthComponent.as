class HealthComponent : Component {
    Misc::Event onDied;

    [Editable]
    private float health = 100.0f;
    private bool isAlly;

    private float startHealth = health;

    HealthComponent(ICompositer@ parent = null) {
        super(@parent);
    }

    void Init() {
        startHealth = health;
        if (isAlly) {
            Moon3ad::gameState.RegisterAlly(@this);
        } else {
            Moon3ad::gameState.RegisterEnemy(@this);
        }
    }

    /*
    void Update(float dt) {
        AddHealth(-20.0 * dt);
    }
    */

    float GetHealth() const { return health; }

    void AddHealth(float incr) {
        log_critical("ATTACK");
        health += incr;
        health = Math::Clamp(health, 0.0f, 100.0f);
        if (health == 0.0f) {
            if (isAlly) {
                Moon3ad::gameState.UnregisterAlly(@this);
            } else {
                Moon3ad::gameState.UnregisterEnemy(@this);
            }
            onDied(@this);
            log_debug("Has fallen: " + GetParentName());
            if (GetParent() !is null) {
                cast<CompositeComponent@>(GetParent()).Destroy();
            }
        }
    }
};
