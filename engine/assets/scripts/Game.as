
namespace Game {

    SceneInstance@ CreateScene(const ResourceHandle &in sceneRef) 
    {
        ref@ scene = Impl::CreateScene(sceneRef);
        return scene !is null ? cast<SceneInstance@>(scene) : null;
    }

}
