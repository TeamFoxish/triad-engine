class DirectionalLightComponent : CompositeComponent {
    [Editable]
    protected DirectionalLight light;

    DirectionalLightComponent(ICompositer@ parent = null) {
        super(parent);
        light = DirectionalLight(GetTransform());
    }

    const DirectionalLight& GetLight() const { return light; }
    DirectionalLight& GetLight() { return light; }
}
