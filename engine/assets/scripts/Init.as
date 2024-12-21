Scene@ sceneRoot;

float initialZ = 0.0f;
float time = 0.0f;
float speed = 1.0f;
float amplitude = 3.0f;

void UpdateImpl(float deltaTime) {
    sceneRoot.Update(deltaTime);
}

void FixedUpdateImpl(float deltaTime) {
    sceneRoot.FixedUpdate(deltaTime);
}

void ShutdownImpl() {
    @sceneRoot = null;
}

void SetScene(Scene@ scene) {
    @sceneRoot = @scene;
}

Scene@ GetScene() {
    return sceneRoot;
}

void init()
{
    println("Initializing scripts...");
    SetUpdate(UpdateImpl);
    SetFixedUpdate(FixedUpdateImpl);
    SetShutdown(ShutdownImpl);
    println("Script initialization done !");
}
