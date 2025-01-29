class Moon3ad {
    private int32 money = 0;

    void IncreaseMoney(int32 income) {
        log_critical("Income: " + income);
        money += income;
    }

    void DecreaseMoney(int32 expense) {
        log_debug("Expense: " + expense);
        money -= expense;
    }

    int32 GetMoney() {
        return money;
    }
}

namespace Moon3ad{
    Moon3ad gameState;
}

class Moon3adComponent : Component {
    Moon3adComponent(ICompositer@ parent = null) {
        super(@parent);
    }

    void Init() {
    }

    void Update(float deltaTime) {
    }
};
