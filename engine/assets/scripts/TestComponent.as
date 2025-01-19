class TestComponent : Component {

    TestComponent(ICompositer@ parent = null) {
        super(@parent);
    }

    [Editable]
    int someInt = 1;
    [Editable]
    float someFloat = 30.0;
    string someString = "Same";
    Material material;
    TestClass testClass;
    TestComponent@ testComponent;

    void Update(float deltaTime) {
        //println("Update time: " + deltaTime);
        //println("someString: " + someString);
        //println("someInt: " + someInt);
        //println("someFloat: " + someFloat);
        if (testClass !is null) {
            //testClass.Print();
        }
        if (testComponent !is null) {
            //println("Component ref someInt: " + testComponent.someInt);
        }
    }

    void FixedUpdate(float deltaTime) {
        //println("Fixed Update time: " + deltaTime);
    }
};