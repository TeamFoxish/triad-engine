class Scene : CompositeComponent {

    Scene(ICompositer@ parent){
        super(parent);
    }

    void Update(float deltaTime) {
        println("updading childs");
        println(GetName());
        const auto children = GetChildren(); // i can't hold reference to array ???
        if (children !is null) {
            for( uint n = 0; n < children.length(); n++ ) {
                if (children[n] !is null) {
                    children[n].Update(deltaTime);
                }
            }
        }
    }
};