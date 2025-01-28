class HealthComponent : Component {
    Misc::Event onDied;

    [Editable]
    private float health = 100.0f;

    private float startHealth = health;

    HealthComponent(ICompositer@ parent = null) {
        super(@parent);
    }

    void Init() {
        startHealth = health;
    }

    float GetHealth() const { return health; }

    void AddHealth(float incr) {
        health += incr;
        health = Math::Clamp(health, 0.0f, 100.0f);
        if (health == 0.0f) {
            onDied(@this);
        }
    }
};
