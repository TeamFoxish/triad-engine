class MeshComponent : CompositeComponent {
    MeshComponent() {
        renderObj = Renderable(0, transform);
    }

    protected Renderable renderObj;

    Mesh& GetMesh() { return renderObj.mesh; }
    void SetMesh(Mesh &in mesh) { renderObj.mesh = mesh; }

    Material& GetMaterial() { return renderObj.material; }
    void SetMaterial(Material &in material) { renderObj.material = material; }
}
