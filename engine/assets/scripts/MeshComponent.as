class MeshComponent : CompositeComponent {
    [Editable]
    protected Renderable renderObj;
    [Editable]
    private bool isStatic = false;

    MeshComponent(ICompositer@ parent = null) {
        super(parent);
        renderObj = Renderable(GetId(), GetTransform());
    }

    Mesh& GetMesh() { return renderObj.mesh; }
    void SetMesh(Mesh &in mesh) { renderObj.mesh = mesh; }

    Material& GetMaterial() { return renderObj.material; }
    void SetMaterial(Material &in material) { renderObj.material = material; }

    void Init() {
        CompositeComponent::Init();

         // TODO: move Init() call after the simulation start
        renderObj.SetIsStatic(isStatic);
    }

    void OnDestroy() {
        renderObj = Renderable();
    }
}
