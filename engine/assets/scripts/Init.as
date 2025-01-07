SceneInstance@ sceneRoot;
SoftRef<ResourceHandle> prefabRef("resd://prefabs/cheese.prefab");
SoftRef<ResourceHandle> compRef("resd://components/SingleSoundComponent.component");
SoftRef<ResourceHandle> meshCompRef("resd://components/MeshComponent.component");

void UpdateImpl(float deltaTime) {
    if (Input::IsKeyDown(Input::Key::Space)) {
        Sound::System::PlayEvent("event:/Bubbles/Bubble2D");
    }
    sceneRoot.Update(deltaTime);
}

void FixedUpdateImpl(float deltaTime) {
    sceneRoot.FixedUpdate(deltaTime);
}

void ShutdownImpl() {
    @sceneRoot = null;
}

void SetScene(SceneInstance@ scene) {
    log_info("Loaded scene: " + scene.GetName());
    @sceneRoot = @scene;
}

SceneInstance@ GetScene() {
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

    Math::Transform@ trs = Math::Transform();
    trs.SetLocalPosition(Math::Vector3(3.0f, 3.0f, 3.0f));
    Game::SpawnPrefab(prefabRef.Load(), trs, sceneRoot);
    SingleSoundComponent@ soundComp = cast<SingleSoundComponent@>(Game::SpawnComponent(compRef.Load(), sceneRoot));
    soundComp.SetEvent("event:/Bubbles/Bubble3D");
    soundComp.Play();
    Game::SpawnComposite(meshCompRef.Load(), null, sceneRoot);
}
