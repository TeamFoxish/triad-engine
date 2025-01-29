class UIComponent : Component {
    protected UIElement element;

    UIComponent(ICompositer@ parent) {
        super(parent);
    }
}