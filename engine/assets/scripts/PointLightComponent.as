class PointLightComponent : CompositeComponent {
    protected PointLight light;

    PointLightComponent(ICompositer@ parent) {
        super(parent);
        light = PointLight(GetTransform());
        println("wtf");
    }

    const PointLight& GetLight() const { return light; }
    PointLight& GetLight() { return light; }
}
