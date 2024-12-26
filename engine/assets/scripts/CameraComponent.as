class CameraComponent : CompositeComponent {
    protected Camera camera;
    protected bool activateAtInit = false;
    protected bool isPerspective = true;

    CameraComponent(ICompositer@ parent) {
        super(parent);
        camera = Camera(GetTransform());
    }

    void Init() {
        if (activateAtInit) {
            camera.SetActive();
        }
        if (isPerspective) {
            println("perspective");
            camera.MakePerspective();
        } else {
            println("!perspective");
            camera.MakeOrthographic();
        }
    }

    const Camera& GetCamera() const { return camera; }
    Camera& GetCamera() { return camera; }
}
