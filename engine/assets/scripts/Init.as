Scene@ sceneRoot;

void UpdateImpl(float deltaTime) {
    sceneRoot.Update(deltaTime);
}

void FixedUpdateImpl(float deltaTime) {
    sceneRoot.FixedUpdate(deltaTime);
}

void SetScene(Scene@ scene) {
    @sceneRoot = @scene;
}

void init()
{
    println("Initializing scripts...");
    SetUpdate(UpdateImpl);
    SetFixedUpdate(FixedUpdateImpl);
    println("Script initialization done !");
}
