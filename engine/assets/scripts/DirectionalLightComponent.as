class DirectionalLightComponent : CompositeComponent {
    protected DirectionalLight light;

    DirectionalLightComponent(ICompositer@ parent) {
        super(parent);
        light = DirectionalLight(GetTransform());
    }

    const DirectionalLight& GetLight() const { return light; }
    DirectionalLight& GetLight() { return light; }
}
