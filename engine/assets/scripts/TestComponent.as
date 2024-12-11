class TestComponent : BaseComponent {

    int someInt = 1;
    float someFloat = 2.1;
    string someString = "Same";
    string material = "";

    void Update(float deltaTime) {
        println("Update time: " + deltaTime);
        println("someString: " + someString);
        println("someInt: " + someInt);
        println("someFloat: " + someFloat);
        println("material: " + material);
    }

    void FixedUpdate(float deltaTime) {
        println("Fixed Update time: " + deltaTime);
    }
};