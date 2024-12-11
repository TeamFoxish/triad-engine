class TestComponent : BaseComponent {

    int someInt = 1;
    float someFloat = 2.1;
    string someString = "Same";
    string material = "";
    TestClass testClass;
    TestComponent@ testComponent;

    void Update(float deltaTime) {
        println("Update time: " + deltaTime);
        println("someString: " + someString);
        println("someInt: " + someInt);
        println("someFloat: " + someFloat);
        println("material: " + material);
        if (testClass !is null) {
            testClass.Print();
        }
        if (testComponent !is null) {
            println("Component ref someInt: " + testComponent.someInt);
        }
    }

    void FixedUpdate(float deltaTime) {
        println("Fixed Update time: " + deltaTime);
    }
};