class AIComponent : Component {

    AiController controller;

    AIComponent(ICompositer@ parent = null) {
        super(parent);
    }

    void Update(float deltaTime) {
        controller.Update(deltaTime);
    }

    void Init() {
        log_info("INIT in AI COMPONENT");
        controller.Init();
        log_info("INIT in AI COMPONENT DONE");
    }
}