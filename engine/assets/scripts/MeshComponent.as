class MeshComponent : CompositeComponent {
    protected Renderable renderObj;

    MeshComponent(ICompositer@ parent = null) {
        super(parent);
        renderObj = Renderable(GetId(), GetTransform());
    }

    Mesh& GetMesh() { return renderObj.mesh; }
    void SetMesh(Mesh &in mesh) { renderObj.mesh = mesh; }

    Material& GetMaterial() { return renderObj.material; }
    void SetMaterial(Material &in material) { renderObj.material = material; }

    void OnDestroy() {
        renderObj = Renderable();
    }
}
