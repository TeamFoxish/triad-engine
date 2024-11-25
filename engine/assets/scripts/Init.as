CompositeComponent@ sceneRoot;

void UpdateImpl(float deltaTime) {
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
    SetUpdate(UpdateImpl);
    SetFixedUpdate(FixedUpdateImpl);
    println("Script initialization done !");
}
