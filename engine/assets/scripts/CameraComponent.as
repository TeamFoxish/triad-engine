class CameraComponent : CompositeComponent {
    protected Camera camera;
    [Editable]
    protected bool activateAtInit = false;
    [Editable]
    protected bool isPerspective = true;

    CameraComponent(ICompositer@ parent = null) {
        super(parent);
        camera = Camera(GetTransform());
    }

    void Init() {
        if (activateAtInit) {
            camera.SetActive();
        }
        if (isPerspective) {
            camera.MakePerspective();
        } else {
            camera.MakeOrthographic();
        }
    }

    const Camera& GetCamera() const { return camera; }
    Camera& GetCamera() { return camera; }
}
