class UIComponent : Component {
    UIElement element;
    int counter;

    UIComponent(ICompositer@ parent) {
        super(parent);
    }

    void Update(float deltaTime) override {
        counter++;
        element.SetText("" + counter);
    }
}