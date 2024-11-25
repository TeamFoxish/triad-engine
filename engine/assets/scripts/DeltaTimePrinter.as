class DeltaTimePrinter : BaseComponent {

    void Update(float deltaTime) {
        println("Update time: " + deltaTime);
    }

    void FixedUpdate(float deltaTime) {
        println("Fixed Update time: " + deltaTime);
    }
}