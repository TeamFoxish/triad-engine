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
