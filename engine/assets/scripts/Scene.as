class Scene : CompositeComponent {
    private string sceneName;

    void Update(float deltaTime) {
        println(sceneName);
        if (child !is null) {
            for( uint n = 0; n < child.length(); n++ ) {
                if (child[n] !is null) {
                    child[n].Update(deltaTime);
                }
            }
        }
    }

    Scene(string name) {
        sceneName = name;
    }

    string GetName() { return sceneName; }
    void SetName(string name) { sceneName = name; }
};