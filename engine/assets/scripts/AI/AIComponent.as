class AIComponent : CompositeComponent {

    AiController controller;

    AIComponent(ICompositer@ parent = null) {
        super(parent);
    }

    void Update(float deltaTime) {
        controller.Update(deltaTime);
    }

    void Init() {
        controller.Init(GetParent());
    }
}