Scene@ sceneRoot;

float initialZ = 0.0f;
float time = 0.0f;
float speed = 1.0f;
float amplitude = 3.0f;

void UpdateImpl(float deltaTime) {
    log_critical("some");
    if (Input::IsKeyDown(Input::Key::Space)) {
        Sound::System::PlayEvent("event:/Bubbles/Bubble2D");
    }
    log_debug("updating root");
    sceneRoot.Update(deltaTime);
}

void FixedUpdateImpl(float deltaTime) {
    sceneRoot.FixedUpdate(deltaTime);
}

void ShutdownImpl() {
    @sceneRoot = null;
}

void SetScene(Scene@ scene) {
    log_info("Loaded scene: " + scene.GetName());
    @sceneRoot = @scene;
}

Scene@ GetScene() {
    return sceneRoot;
}

void init()
{
    log_info("Initializing scripts...");
    SetUpdate(UpdateImpl);
    SetFixedUpdate(FixedUpdateImpl);
    SetShutdown(ShutdownImpl);
    log_info("Script initialization done !");
    
    // TODO: extract to a separate init function
    sceneRoot.Init(); // TEMP
}
