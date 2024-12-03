CompositeComponent@ sceneRoot;
MeshComponent@ meshTest;
float initialZ = 0.0f;
float time = 0.0f;
float speed = 1.0f;
float amplitude = 3.0f;

void UpdateImpl(float deltaTime) {
    time += deltaTime;
    Vector3 pos = meshTest.transform.GetPosition();
    pos.z = initialZ + sin(speed * time) * amplitude;
    println("" + sin(speed * time));
    meshTest.transform.SetPosition(pos);
    sceneRoot.Update(deltaTime);
}

void FixedUpdateImpl(float deltaTime) {
    sceneRoot.FixedUpdate(deltaTime);
}

void init()
{
    println("Initializing scripts...");
    @sceneRoot = @CompositeComponent();
    sceneRoot.AddChild(@DeltaTimePrinter());
    @meshTest = @MeshComponent();
    meshTest.SetMesh(Mesh("res://flop.mesh"));
    meshTest.SetMaterial(Material("res://materials/flopa.material"));
    meshTest.transform.SetScale(Vector3(0.005f, 0.005f, 0.005f));
    SetUpdate(UpdateImpl);
    SetFixedUpdate(FixedUpdateImpl);
    println("Script initialization done !");
}
