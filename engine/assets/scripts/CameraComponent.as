class CameraComponent : CompositeComponent {
    protected Camera camera;

    CameraComponent(ICompositer@ parent) {
        super(parent);
        camera = Camera(GetTransform());
    }

    const Camera& GetCamera() const { return camera; }
    Camera& GetCamera() { return camera; }
}
