void UpdateImpl(float deltaTime) {
    if (Input::IsKeyDown(Input::Key::Space)) {
        Sound::System::PlayEvent("event:/Bubbles/Bubble2D");
    }
    Game::Update(deltaTime);
}

void FixedUpdateImpl(float deltaTime) {
    Game::FixedUpdate(deltaTime);
}

void ShutdownImpl() {
    Game::Shutdown();
}

void SetScene(CompositeComponent@ scene) {
    if (scene is null) {
        return;
    }
    log_info("Loaded scene: " + scene.GetName());
    Game::Private::scenes.insertLast(scene);
    scene.Init();
}

void init()
{
    log_info("Initializing scripts...");
    SetUpdate(UpdateImpl);
    SetFixedUpdate(FixedUpdateImpl);
    SetShutdown(ShutdownImpl);
    SetDestroyComponent(Game::Private::DestroyComponent);
    log_info("Script initialization done !");

    Game::Init();
}
