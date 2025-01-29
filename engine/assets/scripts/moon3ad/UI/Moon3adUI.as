class Moon3adUI : Component {

    protected UIElement money;

    Moon3adUI(ICompositer@ parent) {
        super(parent);
    }

    void Update(float deltaTime) override {
        money.SetText("Money: " + Moon3ad::gameState.GetMoney());
    }

    void Init() {
        money.SetPosition(Math::Vector3(0.1, 0.1, 0));
        money.SetScale(Math::Vector3(0.5, 0.5, 0));
    }
}