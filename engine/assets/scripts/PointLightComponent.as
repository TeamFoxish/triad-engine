class PointLightComponent : CompositeComponent {
    protected PointLight light;

    PointLightComponent(ICompositer@ parent = null) {
        super(parent);
        light = PointLight(GetTransform());
    }

    const PointLight& GetLight() const { return light; }
    PointLight& GetLight() { return light; }
}
